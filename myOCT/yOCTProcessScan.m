function varargout = yOCTProcessScan(varargin)
%This function will utilize parallel computing to load OCT dataset and
%process using processFunc
%USAGE:
%    [out1, out2, ..., dimensions] = yOCTLoadScan(inputDataFolder, processFunc [,parameter1,...]);
%INPUTS:
%   - inputDataFolder - OCT file / folder in local computer or at s3://
%   - processFunc - what function to run on each slide, options are:
%       * 'meanAbs' - to return mean BScan / AScan avg 
%       * 'speckleVariance' - to return STD of BScan / AScan Avg
%       * function handle implementing interface: @(scan,scanAbs,dim)(func(scan,scanAbs,dim)) Where
%           + scan is a vloume 
%           + scanAbs - is abs(scan) - provided for faster calculation
%           + dim is the dimensions structure
%           function should return a matrix the same size as scan.
%       * combination of the above by specifing a cell array. Example:
%           {'meanAbs','speckleVariance',@myfun}
%   - parameter1,... - parameters to be passed to yOCTLoadInterfFromFile
%       and yOCTInterfToScanCpx or any of the parameters below
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                 Default     Information & Values
% 'nYPerIteration'          1           Number of Y scans to load per
%                                       iteration. Try to set such that
%                                       nYPerIteration*scanAvg = 100 scans.
%                                       For parallel usage. Leave default
% 'showStats'               False       Prints execution stats
%
%OUTPUTS:
%   - out1, out2... same as processFunc dimensions: (z,x,y)
%   - dimensions - dimensions structure 

%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

inputDataFolder = varargin{1}; 
processFunc = varargin{2}; 
nYPerIteration = 1;
showStats = false;
parameters = {};
for i=3:2:length(varargin)
    switch(lower(varargin{i}))
        case 'nyperiteration'
            nYPerIteration = varargin{i+1};
        case 'showstats'
            showStats = varargin{i+1};
        otherwise
            parameters(end+1) = varargin(i);
            parameters(end+1) = varargin(i+1);
    end
end

if ~iscell(processFunc)
    processFunc = {processFunc};
end

%% Gather Data
p = gcp;
dimensions = yOCTLoadInterfFromFile([{inputDataFolder}, parameters {'PeakOnly'},{true}]);
[~, ~, sizeY, AScanAvgN, BScanAvgN] = yOCTLoadInterfFromFile_DataSizing(dimensions);

%% Create Grid
if (sizeY == 1)
    %2D
    ys = 1; %Only the first image
    nYPerIteration = 1; %One scan per iteration, as we don't have additional ones
    is3DMode = false;
else
    %3D
    ys = dimensions.y.index;
    is3DMode = true;
end
nScanAvg = AScanAvgN*BScanAvgN;

nIterations = (sizeY/nYPerIteration); %Make sure this number is integer
if (nIterations ~= round(nIterations))
    error('Please set nYPerIteration to be a multiplicative of sizeY');
end

%% Generate Output Structure & Execution Functions
func = cell(size(processFunc));
datOut = ...
    zeros(length(dimensions.lambda.values)/2,length(dimensions.x.values),nYPerIteration,length(func),nIterations,'single'); %z,x,y,iteration, function

for i=1:length(processFunc)  
    if(ischar(processFunc{i}))
        switch(processFunc{i})
            case 'meanAbs'
                func{i} = @meanAbs;
            case 'speckleVariance'
                func{i} = @speckleVariance;
            otherwise
                error('Function Unknown');
        end
    else
        func{i} = processFunc{i};
    end
end

%% Initialize profiling data
profData_dataLoadFrameTime = zeros(nIterations,1);
profData_dataLoadHeaderTime = zeros(nIterations,1);
profData_processingTime   = zeros(nIterations,1);
tt = tic;
ticBytes(p);

%% Loop over all files
iis = zeros(nIterations,nYPerIteration);
for i=1:nIterations
    iis(i,:) = ys((i-1)*nYPerIteration + (1:nYPerIteration));
end
tmpSize = [size(datOut,1) size(datOut,2) size(datOut,3) length(func)];
starI = round(linspace(1,nIterations,10));
fprintf('Processing, Wait for %d Stars ... [ ',length(starI));
myT = tic;
parfor i = 1:nIterations
%for i = 1:nIterations
    tw = tic;
    
    ii = iis(i,:);
    
    %Load interf from file
    [interf,dim,~,prof] = yOCTLoadInterfFromFile([{inputDataFolder} parameters {'YFramesToProcess'} {ii} {'dimensions'} {dimensions}]);
    
    %Convert to scan
    scanCpx = yOCTInterfToScanCpx([{interf},{dim},parameters]);
    scanAbs = abs(scanCpx);
    
    %Process data
    tmp = zeros(tmpSize);
    for j=1:length(func)
        tmp(:,:,:,j) = single(func{j}(scanCpx,scanAbs,dim));
    end
    datOut(:,:,:,:,i) = tmp;
    
    %Profiling
    ld = prof.totalFrameLoadTimeSec + prof.headerLoadTimeSec;
    profData_dataLoadFrameTime(i) = prof.totalFrameLoadTimeSec;
    profData_dataLoadHeaderTime(i) = prof.headerLoadTimeSec;
    profData_processingTime(i)   = toc(tw)-ld;
    
    if (any(starI==i))
        fprintf('* ');
        pause(0.01);
    end
end
fprintf('] Done! (Took %.1fmin)\n',toc(myT)/60);

%% Reshape and output
varargout = cell(length(func)+1,1);
for j=1:length(func)
    varargout{j} = reshape(datOut(:,:,:,j,:),[size(datOut,1) size(datOut,2) sizeY]); %Reshape matrix to a form which is independent of parallelization
end
varargout{end}=dimensions;

%Profiling
profData_totalRunTime = toc(tt);
profData_totalBytesTransfer = sum(tocBytes(p)); %Total bytes sent, total recived

%% Compute statistics
NumWorkers = min(p.NumWorkers,nIterations); %To acount for a situation where number of workers larger then number of iterations
meanIterationLoadDataFrameTime = mean(profData_dataLoadFrameTime);
meanIterationLoadDataHeaderTime = mean(profData_dataLoadHeaderTime);
meanIterationProcessingTime = mean(profData_processingTime);
meanIterationTime = meanIterationLoadDataFrameTime + meanIterationProcessingTime + meanIterationLoadDataHeaderTime;
totalRunTime = profData_totalRunTime;
totalWorkTime = nIterations/NumWorkers*meanIterationTime;%Time spent in sequential work (assuming work load of all workers is equal)
totalOverheadTime = totalRunTime - totalWorkTime;

if showStats
    %General Data
    fprintf('Data Size: X:%d, Y:%d, ScanAvg:%d, workers: %d\n',length(dimensions.x.values),...
        sizeY,nScanAvg,NumWorkers);
    
    %Table
    fprintf('%d Frames per iteration = Y*ScanAvg\n',nScanAvg*nYPerIteration);
    fprintf('Item\t\t\t\t\tData Transfer Time[sec]\tProcessing Time[sec]\tTotal\n');
    
    %Mean Iteration
    fprintf('Mean Iteration:\n');
    fprintf('\tRead OCT Header\t\t%4.1f\t\t\t\t\t\t\t\t\t\t\t%.1f\n',meanIterationLoadDataHeaderTime,meanIterationLoadDataHeaderTime);
    fprintf('\tLoad OCT Frames\t\t%4.1f\t\t\t\t\t%4.1f\t\t\t\t\t%.1f\n',meanIterationLoadDataFrameTime,meanIterationProcessingTime,meanIterationProcessingTime+meanIterationLoadDataFrameTime);
    fprintf('\tTotal\t\t\t\t%4.1f\t\t\t\t\t%4.1f\t\t\t\t\t%.1f\n',meanIterationLoadDataHeaderTime+meanIterationLoadDataFrameTime,meanIterationProcessingTime,totalWorkTime/(nIterations/NumWorkers));
    fprintf('Number of Parallel Iteration Sets: %.1f\n',nIterations/NumWorkers);
    
    %All Iterations
    fprintf('Total Execution:\n');
    fprintf('\tIterations\t\t\t NA\t\t\t\t\t\t NA\t\t\t\t\t\t%.1f\n',totalWorkTime);
    fprintf('\tOverhead\t\t\t%4.1f\t\t\t\t\t\t\t\t\t\t\t%.1f\n',totalOverheadTime,totalOverheadTime);
    fprintf('\t\tOverhead equivalent baud rate: %.2f[MBytes/sec]\n',...
       (sum(profData_totalBytesTransfer)/1024/1024)/totalOverheadTime);
    fprintf('\tTotal\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t%.1f\n',totalRunTime);
end

end
%% Some Default Functions
function out = meanAbs(scan, scanAbs, dim)
    %Which dimensions should the mean be computed for
    dim2Avg = [];
    if(isfield(dim,'BScanAvg'))
        dim2Avg(end+1) = dim.BScanAvg.order; 
    end
    if(isfield(dim,'AScanAvg'))
        dim2Avg(end+1) = dim.AScanAvg.order; 
    end
    if isempty(dim2Avg)
        dim2Avg = 100;
    end

    %Compute mean
    m = scanAbs;
    for j=1:length(dim2Avg)
        m = mean(m,dim2Avg(j));
    end
    out = m;
end
    
function out = speckleVariance(scan, scanAbs, dim)
    %Which dimensions should the mean be computed for
    dim2Avg = [];
    if(isfield(dim,'BScanAvg'))
        dim2Avg(end+1) = dim.BScanAvg.order; 
    end
    if(isfield(dim,'AScanAvg'))
        dim2Avg(end+1) = dim.AScanAvg.order; 
    end
    if isempty(dim2Avg)
        dim2Avg = 100;
    end

    %Compute Speckle Variance.
    %We can compute speckle variance in 2 ways:
    %   - Take variance of the entire dataset, will yield the most accurate
    %     estimation of variance, however if a slight movement exists in
    %     the data (animal breathing for example), then the entire tissue
    %     will have high speckle variance. 
    %   - Take variance of a few samples at a time in a sliding window and
    %     average. This will make sure that we are sensitive only to fast
    %     moving objects (like blood), but not to overall tissue movements.
    %     Estimation of variance however, will be worst. SSE of variance is
    %     2*sigma^4/(N-1). And if we average K samples overall estimator
    %     noise is 2*sigma^4/((N-1)*K). Since N*K=M const then
    %     2*sigma^4/((N-1)/N*M). Meaning the higher N, the more acurate the
    %     estimation is. A good compramise however is N=3.
    
    %Compute Speckle Variance in a moving window
    wSize = 3; %Window Size
    m = movvar(scanAbs,wSize,[],dim2Avg(1));
    out = sqrt(mean(m,dim2Avg(1)));    
end