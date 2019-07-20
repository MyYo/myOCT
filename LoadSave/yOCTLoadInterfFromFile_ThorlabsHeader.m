function dimensions = yOCTLoadInterfFromFile_ThorlabsHeader (inputDataFolder)
%This function loads dimensions structure header
% INPUTS:
%   - inputDataFolder - OCT folder with header.xml file or srr files
%   - OCTSystem - OCT System name

if (strcmpi(inputDataFolder(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials;
    inputDataFolder = awsModifyPathForCompetability(inputDataFolder);
else
    isAWS = false;
end

%% Figure out which of the Thorlabs systems are we using

%LoadXML
ds=fileDatastore([inputDataFolder '/Header.xml'],'ReadFcn',@xml2struct);
xDoc = ds.read;
xDoc = xDoc.Ocity;

if ~isempty(regexp(xDoc.Instrument.Model.Text,'Ganymed','once'))
    OCTSystem = 'Ganymede';
elseif ~isempty(regexp(xDoc.Instrument.Model.Text,'Telesto','once'))
    OCTSystem = 'Telesto';
else
    OCTSystem = 'Unknown';
end


%% Process Xml

order = 1;

%Lambda Size
dimensions = yOCTLoadInterfFromFile_ThorlabsHeaderLambda(inputDataFolder,OCTSystem);
order = order + 1;

try
    %See if Data was aquired in 1D mode
    ds=fileDatastore([inputDataFolder '/data/SpectralFloat.data'],'ReadFcn',@fread);
    oneDMode = true;
catch
    oneDMode = false;
end
if (oneDMode)
    %x,y, B Scan Averaging are all 1
    dimensions.x.order  = NaN;
    dimensions.x.values = 0;
    dimensions.x.units  = 'NaN';
    dimensions.x.index  = 1;
    dimensions.y.order  = NaN;
    dimensions.y.values = 0;
    dimensions.y.units  = 'NaN';
    dimensions.y.index  = 1;
    dimensions.y.indexMax = 1;
    dimensions.AScanAvg.order = 2;
    dataFile = xDoc.DataFiles.DataFile{4}; %This is the xml file specifing 'SpectralFloat.data' file
    AScanAvgN = str2double(dataFile.Attributes.RangeX);
    dimensions.AScanAvg.index = 1:AScanAvgN;
    dimensions.AScanAvg.index = dimensions.AScanAvg.index(:)';
    dimensions.AScanAvg.indexMax = AScanAvgN;
    return;
end

%2D or 3D Modes

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
    dimensions.AScanAvg.index = dimensions.AScanAvg.index(:)';
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