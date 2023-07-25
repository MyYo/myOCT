% Run this demo to use Thorlabs system to scan a 3D OCT Volume and process
% it.
% Before running this script, make sure myOCT folder is in path for example
% by running: addpath(genpath('F:\Jenkins\Scan OCTHist Dev\workspace\'))

% The protocol for how to use this script can be found here:
% https://docs.google.com/presentation/d/1EUYneJwzGAgj2Qg-rG0k6EQb5t1KOCVi0VxEJl8mPmM/edit#slide=id.g25bcdbd2c45_0_0

%% Inputs

% Define the 3D Volume
pixel_size_um = 1; % x-y Pixel size in microns
xOverall_mm = [-1 1]; % Define the overall volume you would like to scan [start, finish].
yOverall_mm = [-1 1]; % Define the overall volume you would like to scan [start, finish].
% Set yOverall_mm = [NaN,NaN] if you would like to only do a BScan.

% Define probe 
octProbePath = getProbeIniPath('10x');
octProbeFOV_mm = 0.5; % How much of the field of view to use from the probe.
oct2stageXYAngleDeg = 0; % Angle between x axis of the motor and the Galvo's x axis

% Define z stack and z-stitching
scanZJump_um = 15; % Use 15 microns for 10x lens, 5 microns for 40x lens
zToScan_mm = ((-190:scanZJump_um:500)-15)*1e-3; %[mm]
focusSigma = 20; %When stitching along Z axis (multiple focus points), what is the size of each focus in z [pixel], use 20 for 10x, 1 for 40x

% Other scanning parameters
nBScanAvg = 1; % Number of B-Scan averages
tissueRefractiveIndex = 1.4; % Use either 1.33 or 1.4 depending on the results
%dispersionQuadraticTerm=6.539e07; % Thorlabs 10x
%dispersionQuadraticTerm=9.56e7;   % Thorlbas 40x
dispersionQuadraticTerm=-2.059e8;  % Jingjing's setup 10x

% Where to save scan files
output_folder = '\';

% Foe debug purpuse. Set to true if you would like to process existing scan
% rather than scan a new one.
skipScanning = false;

%% Compute scanning parameters
% Check that scan range is a whole number compared to octProbeFOV_mm
xOverallRange_mm = diff(xOverall_mm);
yOverallRange_mm = diff(yOverall_mm);
if ((xOverallRange_mm > octProbeFOV_mm) && ...
    (round(xOverallRange_mm/octProbeFOV_mm) ~= (xOverallRange_mm/octProbeFOV_mm)))
    error('The range expresed in xOverall_mm should be a multiplier of octProbeFOV_mm');
end
if ((yOverallRange_mm > octProbeFOV_mm) && ...
    (round(yOverallRange_mm/octProbeFOV_mm) ~= (yOverallRange_mm/octProbeFOV_mm)))
    error('The range expresed in yOverall_mm should be a multiplier of octProbeFOV_mm');
end

% Define centers of the scan
x_centers_mm = ...
    (xOverall_mm(1) + octProbeFOV_mm/2) : ...
    octProbeFOV_mm : ...
    (xOverall_mm(2) - octProbeFOV_mm/2);
if isempty(x_centers_mm)
    x_centers_mm = 0;
end
y_centers_mm = ...
    (yOverall_mm(1) + octProbeFOV_mm/2) : ...
    octProbeFOV_mm : ...
    (yOverall_mm(2) - octProbeFOV_mm/2);
if isempty(y_centers_mm)
    y_centers_mm = 0;
end

% Define the range in x and y. according to FOV
if xOverallRange_mm > octProbeFOV_mm
    % The xOverall scanning area is more than one FOV, use all of it
    xRange_mm = octProbeFOV_mm;
else
    xRange_mm = xOverallRange_mm;
end
if yOverallRange_mm > octProbeFOV_mm
    % The xOverall scanning area is more than one FOV, use all of it
    yRange_mm = octProbeFOV_mm;
else
    yRange_mm = yOverallRange_mm;
end

% Define number of pixels in each axis to preserve pixel size
nXPixels = xRange_mm * 1000 / pixel_size_um;
yXPixels = yRange_mm * 1000 / pixel_size_um;

% Deal with single B-Scan situation.
if isnan(y_centers_mm)
    % Only a single B-Scan is needed
    y_centers_mm = 0;
    yRange_mm = octProbeFOV_mm/10;
    yXPixels = 2; % Scan two pixels in y direction since this code is not ment for a single B scan
end

%% Perform the scan
volumeOutputFolder = [output_folder '/OCTVolume/'];
if ~skipScanning
    msgbox('Please adjust sample such that the sample-gel interface is at OCT focus')
    
    fprintf('%s Scanning Volume\n',datestr(datetime));
    scanParameters = yOCTScanTile (...
        volumeOutputFolder, ...
        'octProbePath', octProbePath, ...
        'tissueRefractiveIndex', tissueRefractiveIndex, ...
        'xOffset',   0, ...
        'yOffset',   0, ... 
        'xRange',    xRange_mm, ...
        'yRange',    yRange_mm, ...
        'nXPixels',  nXPixels, ...
        'nYPixels',  nYPixels, ...
        'nBScanAvg', nBScanAvg, ...
        'xCenters',  x_centers_mm, ...
        'zDepths',   zToScan_mm, ... [mm]
        'oct2stageXYAngleDeg', oct2stageXYAngleDeg, ...
        'v',true  ...
        );
end	

%% Find focus in the scan
fprintf('%s Find Focus\n',datestr(datetime));
focusPositionInImageZpix = yOCTFindFocusTilledScan(volumeOutputFolder,...
    'reconstructConfig',{'dispersionQuadraticTerm',dispersionQuadraticTerm},'verbose',true);

%% Process the scan
fprintf('%s Processing\n',datestr(datetime));
outputTiffFile = [output_folder '/Image.tiff'];
yOCTProcessTiledScan(...
        volumeOutputFolder, ... Input
        {outputTiffFile},... Save only Tiff file as folder will be generated after smoothing
        'focusPositionInImageZpix', focusPositionInImageZpix,... No Z scan filtering
		'focusSigma',focusSigma,...
        'dispersionQuadraticTerm',dispersionQuadraticTerm,... Use default
        'interpMethod','sinc5', ...
        'v',true);
