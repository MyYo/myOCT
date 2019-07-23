%This is a demo runs a scan in a tile function

%% Inputs
dispA = 122;

gridRange = 1; %[mm], x,y size of a single tile
gridRangeMaxX = [-2 2]; %[mm] range min max
gridRangeMaxY = [-2 2]; %[mm] range min max

gridXc = (gridRangeMaxX(1)+gridRange/2):gridRange:(gridRangeMaxX(2)-gridRange/2);
gridYc = (gridRangeMaxY(1)+gridRange/2):gridRange:(gridRangeMaxY(2)-gridRange/2);
[gridXcc,gridYcc] = meshgrid(gridXc,gridYc);
gridXcc = gridXcc(:);
gridYcc = gridYcc(:);

%% Initialize
ThorlabsImagerNETLoadLib();
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit('C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini');
x0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('x');
y0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('y');

%% Scan
disp('Large Volume')
for q = 1:length(gridXcc)

	%Move
	disp(['yOCTStageSetXPosition at ' num2str(x0 + gridXcc(q)) ]);
    disp(['yOCTStageSetYPosition at ' num2str(y0 + gridYcc(q)) ]);
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',...
         x0 + gridXcc(q)... Movement [mm]
		);
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',...
         y0 + gridYcc(q)... Movement [mm]
		);
    
	%Scan
	disp('Scan');
	folder = [sprintf('%02d',q)];
    ThorlabsImagerNET.ThorlabsImager.yOCTScan3DVolume(...
        0,0,gridRange,gridRange, ...centerX,centerY,rangeX,rangeY [mm]
        0,       ... rotationAngle [deg]
        100,100,   ... SizeX,sizeY [# of pixels]
        1,       ... B Scan Average
        folder,   ... Output directory, make sure it exists before running this function
        dispA      ... Dispersion Parameter
        );
end



%% Finalize

%Move back
ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0);
ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0);

%Close Stage
ThorlabsImagerNET.ThorlabsImager.yOCTStageClose('x');
ThorlabsImagerNET.ThorlabsImager.yOCTStageClose('y');

%Close OCT
ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose();

