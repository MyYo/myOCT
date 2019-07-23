%This is a demo run of the functionality in this library

%% Initialize
ThorlabsImagerNETLoadLib();

%% Scan & Photobleach

disp('yOCTScannerInit')
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit('C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini')


disp('yOCTScan3DVolume')
ThorlabsImagerNET.ThorlabsImager.yOCTScan3DVolume(...
    0,0,1,1, ... centerX,centerY,rangeX,rangeY [mm]
	0,       ... rotationAngle [deg]
    100,3,   ... SizeX,sizeY [# of pixels]
    2,       ... B Scan Average
    'scan'   ... Output directory
    );

disp('yOCTPhotobleachLine')
ThorlabsImagerNET.ThorlabsImager.yOCTPhotobleachLine( ...
    -1,0,1,0, ...startX,startY,endX,endY [mm]
	2,        ...duration[sec]
    10         ...repetitions, how many passes to photobleach (choose 1 or 2)
    );

disp('yOCTScannerClose')
ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose();

%% Move Stage

%Initialize Stage (All Z positions will be with respect to that initial one)
disp('yOCTStageInit')
z0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('z');

%Move
disp('yOCTStageSetZPosition');
dz = 500; %[um]
ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',...
    z0+dz/1000  ... Movement [mm]
    );

%Move back (reset)
ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0);

%Close
ThorlabsImagerNET.ThorlabsImager.yOCTStageClose('z');
disp('Testing Done');
