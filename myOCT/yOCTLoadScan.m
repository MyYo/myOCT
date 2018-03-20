function varargout = yOCTLoadScan(varargin)
%This function will utilize parallel computing to load OCT file and preprocess
%USAGE:
%    [scanAbs, scanSpeckleVar] = yOCTLoadScan(inputDataFolder [,parameter1,...]);
%INPUTS:
%   - inputDataFolder - OCT file / folder in local computer or at s3://
%   - parameter1,... - parameters to be passed to yOCTLoadInterfFromFile
%       and yOCTInterfToScanCpx or any of the parameters below
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                 Default     Information & Values
% 'nYPerIteration'          10          Number of Y scans to load per
%                                       iteration. Try to set such that
%                                       nYPerIteration*scanAvg = 100 scans
% 'showStats'               False       Prints execution stats
%
%OUTPUTS:
%   - scanAbs - mean abs scan value (z,x,y) - linear
%   - scanSpeckleVar - speckle variance (z,x,y) 
%Author: Yonatan W (21 Jan, 2018)

%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

inputDataFolder = varargin{1}; 
nYPerIteration = 10;
showStats = false;
parameters = {};
for i=2:2:length(varargin)
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

switch(nargout)
    case 1
        isComputeScanVar = false;
    case 2
        isComputeScanVar = true;
end

%% Gather Data
p = gcp;
dimensions = yOCTLoadInterfFromFile([{inputDataFolder}, parameters {'PeakOnly'},{true}]);

%% Create Grid
if (isnan(dimensions.y.order))
    %2D
    ys = 1; %Only the first image
    nYPerIteration = 1; %One scan per iteration, as we don't have additional ones
else
    %3D
    ys = dimensions.y.index;
end
sizeY = length(ys);

nIterations = (sizeY/nYPerIteration); %Make sure this number is integer
if (nIterations ~= round(nIterations))
    error('Please set nYPerIteration to be a multiplicative of sizeY');
end

%% Initialize profiling data
profData_dataLoadFrameTime = zeros(nIterations,1);
profData_dataLoadHeaderTime = zeros(nIterations,1);
profData_processingTime   = zeros(nIterations,1);
tt = tic;
ticBytes(p);

%% Loop over all files
scanAbs = single(zeros(length(dimensions.lambda.values)/2,length(dimensions.x.values),nYPerIteration,nIterations));
if (isComputeScanVar)
    scanVar = scanAbs;
end
parfor i = 1:nIterations
%for i = 1:nIterations
    tw = tic;
    
    ii = ys((i-1)*nYPerIteration + (1:nYPerIteration));
    
    %Load interf from file
    [interf,dim,~,prof] = yOCTLoadInterfFromFile([{inputDataFolder} parameters {'YFramesToProcess'} {ii}]);
    
    %Convert to scan
    scanCpx = yOCTInterfToScanCpx([{interf},{dim.lambda.k_n},parameters]);
    
    %Which dimensions should the mean be computed for
    dim2Avg = [];
    if(isfield(dim,'BScanAvg'))
        dim2Avg(end+1) = dim.BScanAvg.order; 
    end
    if(isfield(dim,'AScanAvg'))
        dim2Avg(end+1) = dim.AScanAvg.order; 
    end
    
    %Compute mean
    scanCpx = abs(scanCpx);
    m = scanCpx;
    s = [];
    for j=1:length(dim2Avg)
        if (isComputeScanVar && j==(length(dim2Avg)-1))
            %Speckle Variance on the last averaging dimension
            s = std(m,[],dim2Avg(j));
        end
        
        m = mean(m,dim2Avg(j));
    end
        
    %Save data
    scanAbs(:,:,:,i) = single(m);
    if (isComputeScanVar)   
        scanVar(:,:,:,i) = single(s);
    end
    
    %Profiling
    ld = prof.totalFrameLoadTimeSec + prof.headerLoadTimeSec;
    profData_dataLoadFrameTime(i) = prof.totalFrameLoadTimeSec;
    profData_dataLoadHeaderTime(i) = prof.headerLoadTimeSec;
    profData_processingTime(i)   = toc(tw)-ld;
end

scanAbs = reshape(scanAbs,[size(scanAbs,1) size(scanAbs,2) sizeY]); %Reshape matrix to a form which is independent of parallelization
varargout{1} = scanAbs;
if (isComputeScanVar)   
   scanVar = reshape(scanVar,[size(scanVar,1) size(scanVar,2) sizeY]); %Reshape matrix to a form which is independent of parallelization
   varargout{2} = scanVar;
end

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

nScanAvg = 1; %How many frames were averaged per location (B Scan Avg or A Scan Avg)
if (isfield(dimensions,'AScanAvg'))
    nScanAvg = nScanAvg*length(dimensions.AScanAvg.values);
end
if (isfield(dimensions,'BScanAvg'))
    nScanAvg = nScanAvg*length(dimensions.BScanAvg.values);
end

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