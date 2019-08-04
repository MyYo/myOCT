function [interferogram, apodization,prof] = yOCTLoadInterfFromFile_WasatchData(varargin)
%Interface implementation of yOCTLoadInterfFromFile. See help yOCTLoadInterfFromFile
% OUTPUTS:
%   - interferogram - interferogram data, apodization corrected. 
%       Dimensions order (lambda,x,y,AScanAvg,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - apodization - OCT baseline intensity, without the tissue scatterers.
%       Dimensions order (lambda,apodization #,y,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - prof - profiling data - for debug purposes 

%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

inputDataFolder = varargin{1};
if (awsIsAWSPath(inputDataFolder))
    %Load Data from AWS
    awsSetCredentials;
    inputDataFolder = awsModifyPathForCompetability(inputDataFolder);
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

is2D = dimensions.aux.is2D;
rawFilePath = dimensions.aux.rawFilePath;

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

if (is2D)
    DSRead = @(a)DSRead_Tif(a);
    fileIndex = fileIndex+1; %In 2D file index starts with 1, in 3D, starts with 0
else
    %3D
    DSRead = @(a)DSRead_Bin(a);
end


%% Loop over all frames and extract data
%Define output structure
interferogram = zeros(sizeLambda,sizeX,sizeY,1,BScanAvgN);
prof.numberOfFramesLoaded = length(fileIndex);
prof.totalFrameLoadTimeSec = 0;
for fI=1:length(fileIndex)    
    td=tic;
    ds=fileDatastore(rawFilePath(fileIndex(fI)),'ReadFcn',@(a)(DSRead(a)));
    temp=double(ds.read);   
    if (isempty(temp))
        error(['Missing file / file size wrong' spectralFilePath]);
    end
    prof.totalFrameLoadTimeSec = prof.totalFrameLoadTimeSec + toc(td);
    
    interferogram(:,:,yI(fI),1,BScanAvgI(fI)) = temp;
end

%% Load apodization
try
    ds=fileDatastore([inputDataFolder 'raw_bg_00001.tif'],'ReadFcn',@(a)(DSRead(a)));
    apodization = double(ds.read);
    apodization = mean(apodization,2);
catch
    %No Apodization file, then apodization is the mean of all datasets
    apodization = squeeze(mean(mean(interferogram,3),2));
    
    if (size(interferogram,2)*size(interferogram,3) < 100)
        warning('Apodization is computed from average A-Scan. But not many A Scans are found, so value might be off');
    end
end

function temp = DSRead_Tif(fileName)
temp = imread(fileName,'tif');
temp = temp';

function temp = DSRead_Bin(fileName)
out = DSInfo_Bin(fileName);
f = fopen(fileName);
in = fread(f,'*uint16');
fclose(f);
temp = reshape(in,out.sizeLambda,out.sizeX);

function out = DSInfo_Bin(fileName)
%Process file name
[~, fn, ~] = fileparts(fileName);
temp = sscanf(fn,'%05d_raw_us_%d_%d_%d');
out.sizeX = temp(3); %Number of A scans in a B Scan
out.sizeLambda = temp(2); %Number of pixels in an A-scan
out.scanNumber = temp(1);
out.bScanAvgFrameNumber = temp(4);
