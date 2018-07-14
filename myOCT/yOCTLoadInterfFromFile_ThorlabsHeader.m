function dimensions = yOCTLoadInterfFromFile_ThorlabsHeader (inputDataFolder,OCTSystem)
%This function loads dimensions structure from xml header
% INPUTS:
%   - inputDataFolder - OCT folder with header.xml file
%   - OCTSystem - OCT System name

%% Figure Out basic Parameters
if (strcmpi(inputDataFolder(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    yOCTSetAWScredentials;
else
    isAWS = false;
end

%Get OCT Chirp file and lambda min & max
%Original chirp files are installed with Thorlabs software at: C:\Program Files\Thorlabs\SpectralRadar\Config\Chirp.dat
%They can also be extracted from the OCT Folder at [inputDataFolder '\data\Chirp.data']. 
%To load file from OCT folder use chirp = fread(fid, 2048, 'float32');
%To get lambda min & max we contacted thorlabs
switch(OCTSystem)
    case 'Ganymede'
        chirpFileName = 'ChirpGanymede.dat';
        lambdaMin = 824.16;%[nm]
        lambdaMax = 1038.03;%[nm]
    case 'Telesto'
        chirpFileName = 'ChirpTelesto.dat';
        lambdaMin = 1200.56;%[nm]
        lambdaMax = 1367.75;%[nm]
    
    otherwise
        error('ERROR: Wrong OCTSystem name! (yOCTLoadInterfFromFile)')
end

%% Figure out lambda

%Load chirp 
currentFileFolder = fileparts(mfilename('fullpath'));
if exist([currentFileFolder chirpFileName]','file')
    ds=fileDatastore([currentFileFolder chirpFileName],'ReadFcn',@readChirpTxt);
else
    %Cannot find file locally, read it from data folder
    ds=fileDatastore([inputDataFolder '/data/chirp.data'],'ReadFcn',@readChirpBin);
end
chirp = ds.read;

%We belive that chirp describes the corrections for k. meaning
%k(i) = chirp(i)*A+B. Since lambda = 2*pi/k = 2*pi/(chirp*A+B) then
lambda = 1./( ...
            chirp .* ( 1/(length(chirp)-1)*(1/lambdaMax-1/lambdaMin) )+... chirp*A/2pi
            1/lambdaMin ... B/2pi
        );

%% Load Xml

%LoadXML
ds=fileDatastore([inputDataFolder '/Header.xml'],'ReadFcn',@xml2struct);
xDoc = ds.read;
xDoc = xDoc.Ocity;

order = 1;

%Lambda Size
dimensions.lambda.order  = order;
dimensions.lambda.values = lambda;
dimensions.lambda.values = dimensions.lambda.values(:)';
dimensions.lambda.units = 'nm';
order = order + 1;

%Along B Scan Axis (x)
sizeX = str2double(xDoc.Image.SizePixel.SizeX.Text);
sizeXReal = str2double(xDoc.Image.SizeReal.SizeX.Text)*1000;
dimensions.x.order = order;
dimensions.x.values = linspace(0,1,sizeX).*sizeXReal;
dimensions.x.values = dimensions.x.values(:)';
dimensions.x.units = 'microns';
dimensions.x.index = (1:sizeX);
dimensions.x.index = dimensions.x.index(:)';
order = order + 1;

%Across B Scan Axis (y)
if ~isfield(xDoc.Image.PixelSpacing,'SpacingY') %Only 1 B Scan
    sizeY = 1;
    dimensions.y.order = NaN;
    dimensions.y.values = 0;
    dimensions.y.units = 'microns';
    dimensions.y.index = 1;
    dimensions.y.indexMax = 1;
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
        dimensions.y.index = dimensions.y.index(:)';
        dimensions.y.indexMax = sizeY;
        order = order + 1;

    else
        %Y Dimension is actually BScanAvg
        BScanAvgN = sizeY;
        sizeY = 1;
        dimensions.y.order = NaN;
        dimensions.y.values = 0;
        dimensions.y.units = 'microns';
        dimensions.y.index = 1;
        dimensions.y.indexMax = sizeY;
    end
end

%A Scan Averaging
AScanAvgN = str2double(xDoc.Acquisition.IntensityAveraging.AScans.Text);
if (AScanAvgN > 1)
    dimensions.AScanAvg.order = order;
    dimensions.AScanAvg.index = 1:AScanAvgN;
    dimensions.AScanAvg.index = dimensions.AScanAvg.values(:)';
    dimensions.AScanAvg.indexMax = AScanAvgN;

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
end
if (BScanAvgN > 1)
    dimensions.BScanAvg.order = order;
    dimensions.BScanAvg.index = 1:BScanAvgN;
    dimensions.BScanAvg.index = dimensions.BScanAvg.index(:)';
    dimensions.BScanAvg.indexMax = BScanAvgN;

    order = order + 1;
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
dimensions.aux.interfSize = str2double(xDoc.DataFiles.DataFile{spectralInd}.Attributes.SizeX);
dimensions.aux.apodSize = str2double(xDoc.DataFiles.DataFile{spectralInd}.Attributes.ApoRegionEnd0);

%A Scan Binning if relevant
dimensions.aux.AScanBinning = str2double(xDoc.Acquisition.IntensityAveraging.Spectra.Text);

end

function chirp = readChirpTxt(fp)
fid = fopen(fp);
chirp = textscan(fid,'%f');
chirp = chirp{1};
fclose(fid);
end
function chirp = readChirpBin(fp)
fid = fopen(fp);
chirp  = fread(fid, 'float32');
fclose(fid);
end