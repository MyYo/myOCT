function [interferogram, dimensions, apodization] = yOCTLoadInterfFromFile(varargin)
%USAGE:
% [interferogram, dimensions, apodization] = 
%       yOCTLoadInterfFromFile(inputDataFolder,'param1',value1,'param2',value2,...)
%This function loads an interferogram data from file and preforms initial
%interferogram preprocessing
%
% INPUTS:
%   - inputDataFolder - OCT data folder / AWS data folder
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                 Default     Information & Values
% 'OCTSystem'               'Ganymede'  OCT System Name, can be 'Ganymede' or 'Telesto'
% 'BScanAvgFramesToProcess' all         What B-Scan Average frame indexies to process. Usefull for memory management applications     
% 'YFramesToProcess'        all         What Y frames to process indexies
%                                       (applicable only for 3D scans). Usefull for memory management applications
% 'PeakOnly'                false       when set to true, this function will return 
%                                       dimensions without computing the interferogram
%                                       Usage: dimensions = yOCTLoadInterfFromFile(...)
% OUTPUTS:
%   - interferogram - interferogram data. 
%       Dimensions order (lambda,x,y,AScanAvg,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - dimensions  - interferogram dimensions structure, containing dimension order and values. 
%   - apodization - OCT baseline intensity, without the tissue scatterers.
%       Dimensions order (lambda,podization #,y,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%Author: Yonatan W (25 Dec, 2017)

%% Input Checks
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
OCTSystem = 'Ganymede';
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
            error('Unknown parameter');
    end
end

%% System Specific Information
switch(OCTSystem)
    case 'Ganymede'
        chirpFileName = 'Chirp_Ganymede.mat';
        lambdaMin = 800;  %nm
        lambdaMax = 1000; %nm
        filt = @(chirp_vect)(ones(size(length(chirp_vect))));
    case 'Telesto'
        chirpFileName ='Chirp_Telesto.mat';
        filt = @(chirp_vect)(hann(length(chirp_vect)));
    otherwise
        error('ERROR: Wrong OCTSystem name! (yOCTLoadInterfFromFile)')
end

%Load Chirp
if ~isAWS
    currentFileFolder = fileparts(mfilename());
    load([currentFileFolder chirpFileName],'chirp_vect');
else
    ds=fileDatastore(['s3://delazerdalab1/CodePackage/' chirpFileName],'ReadFcn',@load);
    tmp = ds.read;
    chirp_vect = tmp.chirp_vect;
end
filt = filt(chirp_vect);

%LoadXML
if ~isAWS
    xDoc = xml2struct([inputDataFolder '/Header.xml']); 
    xDoc = xDoc.Ocity;
else
    ds=fileDatastore([inputDataFolder '/Header.xml'],'ReadFcn',@xml2struct);
    xDoc = ds.read;
    xDoc = xDoc.Ocity;
end

%% Determine Dimensions
order = 1;

%Lambda Size
sizeLambda = length(chirp_vect);
dimensions.lambda.order  = order;
dimensions.lambda.values = ...
    interp1([0,max(chirp_vect)],[lambdaMin,lambdaMax],flipud(chirp_vect));
dimensions.lambda.values = dimensions.lambda.values(:)';
dimensions.lambda.units = 'nm';
dimensions.lambda.k_n = flipud(chirp_vect);
order = order + 1;

%Along B Scan Axis (x)
sizeX = str2double(xDoc.Image.SizePixel.SizeX.Text);
sizeXReal = str2double(xDoc.Image.SizeReal.SizeX.Text)*1000;
dimensions.x.order = order;
dimensions.x.values = linspace(0,1,sizeX).*sizeXReal;
dimensions.x.values = dimensions.x.values(:)';
dimensions.x.units = 'microns';
dimensions.x.index = (1:sizeX)';
order = order + 1;

%Across B Scan Axis (y)
if ~isfield(xDoc.Image.PixelSpacing,'SpacingY') %Only 1 B Scan
    sizeY = 1;
else
    sizeY = str2double(xDoc.Image.SizePixel.SizeY.Text);
    sizeYReal=str2double(xDoc.Image.SizeReal.SizeY.Text)*1000;
    
    if (sizeYReal > 0)
        %Y Dimension exists
        dimensions.y.order = order;
        dimensions.y.values = linspace(0,1,sizeY).*sizeYReal;
        dimensions.y.values = dimensions.y.values(:)';
        dimensions.y.units = 'microns';
        dimensions.y.index = 1:sizeY;
        order = order + 1;
        
        if exist('YFramesToProcess','var')
            %Process only subset of Y frames
            dimensions.y.values = dimensions.y.values(YFramesToProcess);
            dimensions.y.index = dimensions.y.index(YFramesToProcess);
            sizeY = length(YFramesToProcess);
        end
    else
        %Y Dimension is actually BScanAvg
        BScanAvgN = sizeY;
        BScanAvgNOrig = BScanAvgN;
        sizeY = 1;
    end
end

%A Scan Averaging
AScanAvgN = str2double(xDoc.Acquisition.IntensityAveraging.AScans.Text);
if (AScanAvgN > 1)
    dimensions.AScanAvg.order = order;
    dimensions.AScanAvg.values = 1:AScanAvgN;
    dimensions.AScanAvg.values = dimensions.AScanAvg.values(:)';
    dimensions.AScanAvg.units = '#';
    order = order + 1;
end

%B Scan Averaging
if (exist('BScanAvgN','var'))
    %'Y' is actually the B-Scan Avg axis    
else
    BScanAvgN = 1;
    if isfield(xDoc.Acquisition,'SpeckleAveraging')
        BScanAvgN = str2double(xDoc.Acquisition.SpeckleAveraging.SlowAxis.Text);
    end
    BScanAvgNOrig = BScanAvgN;
end
if (BScanAvgN > 1)
    dimensions.BScanAvg.order = order;
    dimensions.BScanAvg.values = 1:BScanAvgN;
    dimensions.BScanAvg.values = dimensions.BScanAvg.values(:)';
    dimensions.BScanAvg.units = '#';
    %order = order + 1;
    
    if exist('BScanAvgFramesToProcess','var')
        %Process only subset of Y frames
        dimensions.BScanAvg.values = dimensions.BScanAvg.values(BScanAvgFramesToProcess);
        BScanAvgN = length(BScanAvgFramesToProcess);
    end
end

if (peakOnly == true)
    %We are done!
    interferogram = dimensions;
    apodization = dimensions;
    return;
end

%% Determine dimensions for apodization

%Get Index of data file that has spectral information
spectralInd = 0;
for i = 1:length(xDoc.DataFiles.DataFile)
    if strcmp(xDoc.DataFiles.DataFile{i}.Text(1:13),'data\Spectral')
        spectralInd = i;
        break;
    end
end
if spectralInd == 0
    error('missing raw spectral data in folder')
end

%Extract apodization information from the relevant frame
interfSize = str2double(xDoc.DataFiles.DataFile{spectralInd}.Attributes.SizeX);
apodSize = str2double(xDoc.DataFiles.DataFile{spectralInd}.Attributes.ApoRegionEnd0);

%A Scan Binning if relevant
AScanBinning = str2double(xDoc.Acquisition.IntensityAveraging.Spectra.Text);

%% Generate File Grid
%What frames to load
[yI,BScanAvgI] = meshgrid(1:sizeY,1:BScanAvgN);
yI = yI(:)';
BScanAvgI = BScanAvgI(:)';

%fileIndex is organized such as beam scans B scan avg, then moves to the
%next y position
fileIndex = (dimensions.y.index(yI)-1)*BScanAvgNOrig + dimensions.BScanAvg.values(BScanAvgI)-1;

%% Loop over all frames and extract data
%Define output structure
interferogram = zeros(sizeLambda,sizeX,sizeY, AScanAvgN, BScanAvgN);
apodization   = zeros(sizeLambda,apodSize,sizeY,BScanAvgN);
N = sizeLambda;
for fi=1:length(fileIndex)
    if ~isAWS
        %Load Data from File
        fid = fopen([inputDataFolder '/data/Spectral' num2str(fileIndex(fi)) '.data']);
        temp = fread(fid,inf,'short');
        fclose(fid);
    else
        %Load Data from AWS
        ds=fileDatastore([inputDataFolder '/data/Spectral' num2str(fileIndex(fi)) '.data'],'ReadFcn',@AWSRead);
        temp=ds.read;
    end
    temp = reshape(temp,[N,interfSize]);

    %Read apodization
    apodization(:,:,fi) = flipud(temp(:,1:apodSize));
    apod = mean(apodization(:,:,fi),2);    
    
    %Read interferogram
    interf = flipud(temp(:,apodSize+1:end)); 
    
    %Correct for apodization
    interf = interf - repmat(apod,1,size(interf,2)); %Subtract baseline intensity
    
    %Average over AScanBinning
    if (AScanBinning > 1)
        avgFilt = ones(1,AScanBinning)/(AScanBinning);
        interfAvg = filter2(avgFilt,interf);
        interfAvg = interfAvg(:,max(1,floor((AScanBinning)/2)):AScanBinning:end);    
    else
        interfAvg = interf;
    end
    
    % Apply fliter specific to OCT System
    interfAvg = interfAvg.*filt;
    
    %Save
    if (AScanAvgN > 1)
        %Reshape to extract A scan averaging
        tmpR = reshape(interfAvg,...
                [sizeLambda,AScanAvgN,sizeX]);
        interferogram(:,:,yI(fi),:,BScanAvgI(fi)) = permute(tmpR,[1 3 2]);
    else
        interferogram(:,:,yI(fi),:,BScanAvgI(fi)) = interfAvg;
    end
end

interferogram = squeeze(interferogram);
apodization = squeeze(apodization);

function temp = AWSRead(fileName)
fid = fopen(fileName);
temp = fread(fid,inf,'short');
fclose(fid);