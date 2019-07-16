%This is a demo run

%% Load library
global ThorlabsImagerNETLoaded
if isempty(ThorlabsImagerNETLoaded) || ~ThorlabsImagerNETLoaded
    % Copy Subfolders to here
	copyfile('LaserDiode\*.*','.')
	copyfile('MotorController\*.*','.')
	copyfile('ThorlabsOCT\*.*','.')
    
    %Load Assembly
    asm = NET.addAssembly(['\\171.65.17.174\MATLAB_Share\Yonatan\Tools\yOCT\ThorlabsImager\Lib\' 'ThorlabsImagerNET.dll']);
    ThorlabsImagerNETLoaded = true;
end

%% Scan & Photobleach
disp('yOCTScannerInit')
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit('C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini')

disp('yOCTScan3DVolume')
ThorlabsImagerNET.ThorlabsImager.yOCTScan3DVolume(...
    0,0,1,1, ...startX,startY,rangeX,rangeY [mm]
	0,       ... rotationAngle [deg]
    100,3,   ... SizeX,sizeY [# of pixels]
    2,       ... B Scan Average
    'scan'   ... Output directory
    );

disp('yOCTPhotobleachLine')
ThorlabsImagerNET.ThorlabsImager.yOCTPhotobleachLine( ...
    -1,0,1,0, ...startX,startY,endX,endY [mm]
	2,        ...duration[sec]
    1         ...repetitions, how many passes to photobleach (choose 1 or 2)
    );

disp('yOCTScannerClose')
ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose();

%% Move Stage
disp('yOCTStageInit')
%Initialize Stage (All Z positions will be with respect to that initial one)
ThorlabsImagerNET.ThorlabsImager.yOCTStageInit();

%Move
disp('yOCTStageSetZPosition');
dz = 1000; %[um]
ThorlabsImagerNET.ThorlabsImager.yOCTStageSetZPosition(...
    dz/1000  ... Movement [mm]
    );

%Move back (reset)
ThorlabsImagerNET.ThorlabsImager.yOCTStageSetZPosition(0);

disp('Testing Done');
