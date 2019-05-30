function [interferogram, apodization,prof] = yOCTLoadInterfFromFile_ThorlabsData(varargin)
%Interface implementation of yOCTLoadInterfFromFile. See help yOCTLoadInterfFromFile
% OUTPUTS:
%   - interferogram - interferogram data, apodization corrected. 
%       Dimensions order (lambda,x,y,AScanAvg,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - apodization - OCT baseline intensity, without the tissue scatterers.
%       Dimensions order (lambda,podization #,y,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - prof - profiling data - for debug purposes 

%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

inputDataFolder = varargin{1};
if (strcmpi(inputDataFolder(1:3),'s3:'))
    %Load Data from AWS
    yOCTSetAWScredentials;
    inputDataFolder = myOCTModifyPathForAWSCompetability(inputDataFolder);
end

%Optional Parameters
for i=2:2:length(varargin)
    switch(lower(varargin{i}))
        case 'dimensions'
            dimensions = varargin{i+1};
        otherwise
            %error('Unknown parameter');
    end
end

%% Determine dimensions
[sizeLambda, sizeX, sizeY, AScanAvgN, BScanAvgN] = yOCTLoadInterfFromFile_DataSizing(dimensions);   

if (sizeX == 1)
    %% 1D Mode, Different loading scheme
    
    prof.numberOfFramesLoaded = 1;
    tic;
    ds=fileDatastore([inputDataFolder '/data/SpectralFloat.data'],'ReadFcn',@(a)(DSRead(a,'float32')));
    temp = double(ds.read);
    prof.totalFrameLoadTimeSec = toc;
    temp = reshape(temp,[sizeLambda,AScanAvgN]);
    interferogram = zeros(sizeLambda,sizeX,sizeY, AScanAvgN, BScanAvgN);
    interferogram(:,1,1,:,1) = temp;
    apodization = NaN; %No Apodization in file
    return;
end

interfSize = dimensions.aux.interfSize;
apodSize = dimensions.aux.apodSize;
AScanBinning = dimensions.aux.AScanBinning;

%% Generate File Grid
%What frames to load
[yI,BScanAvgI] = meshgrid(1:sizeY,1:BScanAvgN);
yI = yI(:)';
BScanAvgI = BScanAvgI(:)';

%fileIndex is organized such as beam scans B scan avg, then moves to the
%next y position
if (isfield(dimensions,'BScanAvg'))
    fileIndex = (dimensions.y.index(yI)-1)*dimensions.BScanAvg.indexMax + dimensions.BScanAvg.index(BScanAvgI)-1;
else
    fileIndex = (dimensions.y.index(yI)-1);
end

%% Loop over all frames and extract data
%Define output structure
interferogram = zeros(sizeLambda,sizeX,sizeY, AScanAvgN, BScanAvgN);
apodization   = zeros(sizeLambda,apodSize,sizeY,1,BScanAvgN);
N = sizeLambda;
prof.numberOfFramesLoaded = length(fileIndex);
prof.totalFrameLoadTimeSec = 0;
for fi=1:length(fileIndex)
    spectralFilePath = [inputDataFolder '/data/Spectral' num2str(fileIndex(fi)) '.data'];
    td=tic;
 
    %Load Data
    ds=fileDatastore(spectralFilePath,'ReadFcn',@(a)(DSRead(a,'short')));
    temp=double(ds.read);

    if (isempty(temp))
        error(['Missing file / file size wrong' spectralFilePath]);
    end
    prof.totalFrameLoadTimeSec = prof.totalFrameLoadTimeSec + toc(td);
    temp = reshape(temp,[N,interfSize]);

    %Read apodization
    apod = temp(:,1:apodSize);
    
    %Read interferogram
    interf = temp(:,apodSize+1:end); 
    
    %Average over AScanBinning
    if (AScanBinning > 1)
        avgFilt = ones(1,AScanBinning)/(AScanBinning);
        interfAvg = filter2(avgFilt,interf);
        interfAvg = interfAvg(:,max(1,floor((AScanBinning)/2)):AScanBinning:end);    
    else
        interfAvg = interf;
    end
    
    %Save
    apodization(:,:,fi) = apod;
    if (AScanAvgN > 1)
        %Reshape to extract A scan averaging
        tmpR = reshape(interfAvg,...
                [sizeLambda,AScanAvgN,sizeX]);
        interferogram(:,:,yI(fi),:,BScanAvgI(fi)) = permute(tmpR,[1 3 2]);
    else
        interferogram(:,:,yI(fi),:,BScanAvgI(fi)) = interfAvg;
    end
end

function temp = DSRead(fileName, dataType) %dataType can be 'short','float32'
fid = fopen(fileName);
temp = fread(fid,inf,dataType);
fclose(fid);
