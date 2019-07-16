%This is a demo run

%Load library
if ~libisloaded('ThorlabsImager')
    currentFileFolder = fileparts(mfilename('fullpath'));
    addpath(genpath(currentFileFolder));
	
	% Copy Subfolders to here
	copyfile('LaserDiode\*.dll','.')
	copyfile('MotorController\*.dll','.')
	copyfile('ThorlabsOCT\*.dll','.')

    loadlibrary('ThorlabsImager',@ThorlabsImager);
end

%% OCT Scan (Volume)
disp('OCT Scan');

%Initialize Probe
calllib('ThorlabsImager','yOCTScannerInit','C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini');

%Scan a 3D Volume
calllib('ThorlabsImager','yOCTScan3DVolume', ...
    0,0,1,1, ...startX,startY,rangeX,rangeY [mm]
	0,       ... rotationAngle [deg]
    100,3,   ... SizeX,sizeY [# of pixels]
    2,       ... B Scan Average
    'scan'   ... Output directory
    );

%Cleanup
calllib('ThorlabsImager','yOCTScannerClose');

%% Photobleach A Line
disp('Photobleach');

calllib('ThorlabsImager','yOCTPhotobleachLine', ...
    -1,0,1,0, ...startX,startY,endX,endY [mm]
	2,        ...duration[sec]
    1         ...repetitions, how many passes to photobleach (choose 1 or 2)
    );

%% Move Z Stage
disp('Z Stage');

%Initialize Stage (All Z positions will be with respect to that initial one)
calllib('ThorlabsImager','yOCTStageInit');

%Move
dz = 15; %[um]
calllib('ThorlabsImager','yOCTStageSetZPosition',...
    dz/1000  ... Movement [mm]
    );

%Move back (reset)
calllib('ThorlabsImager','yOCTStageSetZPosition',0);