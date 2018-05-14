function [interferogram, dimensions, apodization,prof] = yOCTLoadInterfFromFile_Wasatch(varargin)
%Interface implementation of yOCTLoadInterfFromFile. See help yOCTLoadInterfFromFile_Thorlabs
% OUTPUTS:
%   - interferogram - interferogram data, apodization corrected. 
%       Dimensions order (lambda,x,y,AScanAvg,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - dimensions  - interferogram dimensions structure, containing dimension order and values. 
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
if (inputDataFolder(end) ~='/' && inputDataFolder(end) ~='\')
    inputDataFolder = [inputDataFolder '/'];
end
if (strcmpi(inputDataFolder(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    yOCTSetAWScredentials;
else
    isAWS = false;
end

%Optional Parameters
peakOnly = false;
OCTSystem = 'Wasatch';
for i=2:2:length(varargin)
    switch(lower(varargin{i}))
        case 'octsystem'
            OCTSystem = varargin{i+1};
        case 'peakonly'
            peakOnly = varargin{i+1};
        case 'yframestoprocess'
            YFramesToProcess = varargin{i+1};
            YFramesToProcess = YFramesToProcess(:);
        case 'bscanavgframestoprocess'
            BScanAvgFramesToProcess = varargin{i+1};
            BScanAvgFramesToProcess = BScanAvgFramesToProcess(:);
        otherwise
            %error('Unknown parameter');
    end
end

tt=tic;

%Define file naming convension
rawFilePath = @(i)(sprintf('%sraw_%05d.tif',inputDataFolder,i));

%% Header Information

ds=fileDatastore(rawFilePath(1),'ReadFcn',@(a)DSInfo(a));
temp=ds.read;

sizeX = temp.Height; %Number of A scans in a B Scan
sizeLambda = temp.Width; %Number of pixels in an A-scan

%Quarry how many files are there
ds=fileDatastore([inputDataFolder 'raw_0*.tif'],'ReadFcn',@(a)DSInfo(a));
sizeY = length(ds.Files);

%% Compute Chirp
%Information from Wasatch
%variables->lmbd(nm)=cl0+cl1*pixel+cl2*pixel^2+cl3*pixel^3
cl0=6.57328E+02;
cl1=8.28908E-02;
cl2=-1.10175E-06;
cl3=-7.04714E-11;

pixel = 1:(sizeLambda);
lambda = cl0+cl1*pixel+cl2*pixel.^2+cl3*pixel.^3;

%% Determine Dimensions
order = 1;

%Lambda Size
dimensions.lambda.order  = order;
dimensions.lambda.values = lambda;
dimensions.lambda.values = dimensions.lambda.values(:)';
dimensions.lambda.units = 'nm';
order = order + 1;

%Along B Scan Axis (x)
dimensions.x.order = order;
dimensions.x.values = linspace(0,1,sizeX);
dimensions.x.values = dimensions.x.values(:)';
dimensions.x.units = 'NA';
dimensions.x.index = (1:sizeX);
dimensions.x.index = dimensions.x.index(:)';
order = order + 1;

%Across B Scan Axis (y)
dimensions.y.order = order;
dimensions.y.values = linspace(0,1,sizeY);
dimensions.y.values = dimensions.y.values(:)';
dimensions.y.units = 'NA';
dimensions.y.index = (1:sizeY);
dimensions.y.index = dimensions.y.index(:)';
order = order + 1;

%Request to process some of the frames either in y or in BScan Avg
if exist('YFramesToProcess','var')
    %Process only subset of Y frames
    dimensions.y.values = dimensions.y.values(YFramesToProcess);
    dimensions.y.index = dimensions.y.index(YFramesToProcess);
    dimensions.y.index = dimensions.y.index(:)';
    sizeY = length(YFramesToProcess);
elseif exist('BScanAvgFramesToProcess','var')
    %Process only subset of Y frames
    dimensions.y.values = dimensions.y.values(BScanAvgFramesToProcess);
    dimensions.y.index = dimensions.y.index(BScanAvgFramesToProcess);
    dimensions.y.index = dimensions.y.index(:)';
    sizeY = length(BScanAvgFramesToProcess);
end

if (peakOnly == true)
    %We are done!
    interferogram = dimensions;
    apodization = dimensions;
    return;
end

%% Loop over all frames and extract data
%Which file indexes to load
fileIndex = dimensions.y.index;

%Define output structure
interferogram = zeros(sizeLambda,sizeX,sizeY);
prof.numberOfFramesLoaded = length(fileIndex);
prof.totalFrameLoadTimeSec = 0;
for fI=1:length(fileIndex)    
    td=tic;
    ds=fileDatastore(rawFilePath(fI),'ReadFcn',@(a)(DSRead(a)));
    temp=double(ds.read);   
    if (isempty(temp))
        error(['Missing file / file size wrong' spectralFilePath]);
    end
    prof.totalFrameLoadTimeSec = prof.totalFrameLoadTimeSec + toc(td);
    
    interferogram(:,:,fI) = temp';
end

%% Load apodization
try
    ds=fileDatastore([inputDataFolder 'raw_bg_00001.tif'],'ReadFcn',@(a)(DSRead(a)));
    apodization = double(ds.read)';
    apodization = mean(apodization,2);
catch
    %No Apodization file, then apodization is the mean of all datasets
    apodization = squeeze(mean(mean(interferogram,3),2));
end

%% Correct for apodization
interferogram = interferogram - repmat(apodization,[1 sizeX sizeY]);

%% Finish
interferogram = squeeze(interferogram);
apodization = squeeze(apodization);

function temp = DSRead(fileName)
temp = imread(fileName,'tif');

function temp = DSInfo(fileName)
temp = imfinfo(fileName,'tif');
