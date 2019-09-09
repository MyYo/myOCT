function [json] = yOCTScanTile(varargin)
%This function preforms an OCT Scan of a volume, and them tile around to
%stitch together multiple scans. Tiling will be done by 3D translation
%stage.
%INPUTS:
%   octFolder - folder to save all output information
%
%NAME VALUE INPUTS:
%   Parameter  Default Value    Notes
%   octProbePath       'probe.ini' Where is the probe.ini is saved to be used
%   isVerifyMotionRange   true     Try the full range of motion before scanning, to make sure we won't get 'stuck' through the scan
%   tissueRefractiveIndex 1.4      Refractive index of tissue
%Parameter controling each tile:
%   xCenter         0           Center position of the 3D volume (using galvo). Units: mm
%   yCenter         0           Center position of the 3D volume (using galvo). Units: mm
%   xRange          1           Total range of scan, x size in mm. Galvo position to physical size conversion is done by probe model set by yOCTScannerInit
%   yRange          1           Total range of scan, y size in mm. Galvo position to physical size conversion is done by probe model set by yOCTScannerInit
%   nXPixels        1000        Number of pixels in x direction (equally spaced)
%   nYPixels        1000        Number of pixels in y direction (equally spaced)
%   nBScanAvg       1           How many B Scan Averaging to scan
%Scan tiling parameters, these will cerate a meshgrid (relative to current position of stage)
%   xToScan         0           scan center x to scan. Default is not to move stage at all. Units: mm
%   yToScan         0           scan center y to scan. Default is not to move stage at all. Units: mm
%   zToScan         0           what depths to scan (positive value is deeper). Default is not to move stage at all. Units: mm
%Debug parameters:
%   v               true        verbose mode      
%OUTPUT:
%   json - config file
%
%How Tiling works. The assumption is that the OCT is stationary, and the
%sample is mounted on 3D translation stage that moves around to tile

%% Input Parameters
p = inputParser;
addRequired(p,'octFolder',@isstr);

%General parameters
addParameter(p,'octProbePath','probe.ini',@isstr);
addParameter(p,'isVerifyMotionRange',true,@islogical);
addParameter(p,'tissueRefractiveIndex',1.4,@isnumeric);

%Single scan parmaeters
addParameter(p,'xCenter',0,@isnumeric);
addParameter(p,'yCenter',0,@isnumeric);
addParameter(p,'xRange',0,@isnumeric);
addParameter(p,'yRange',0,@isnumeric);
addParameter(p,'nXPixels',1000,@isnumeric);
addParameter(p,'nYPixels',1000,@isnumeric);
addParameter(p,'nBScanAvg',1,@isnumeric);

%Tile Parameters
addParameter(p,'xToScan',0,@isnumeric);
addParameter(p,'yToScan',0,@isnumeric);
addParameter(p,'zToScan',0,@isnumeric);

%Debugging
addParameter(p,'v',true,@islogical);

parse(p,varargin{:});

in = p.Results;
octFolder = in.octFolder;
v = in.v;
in = rmfield(in,'octFolder');
in = rmfield(in,'v');
in.units = 'mm'; %All units are mm
in.version = 1; %Version of this file

%% Scan center list

%Scan order, z changes fastest, x after, y latest
[in.gridXcc, in.gridZcc,in.gridYcc] = meshgrid(in.xToScan,in.zToScan,in.yToScan); 
in.gridXcc = in.gridXcc(:);
in.gridYcc = in.gridYcc(:);
in.gridZcc = in.gridZcc(:);
in.scanOrder = 1:length(in.gridZcc);
in.octFolders = arrayfun(@(x)(sprintf('Data%02d',x)),in.scanOrder,'UniformOutput','false');

scanOrder = in.scanOrder;

%% Initialize hardware
if (v)
    fprintf('%s Initialzing Hardware...\n',datestr(datetime));
end
 
ThorlabsImagerNETLoadLib(); %Init library
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit(in.octProbePath); %Init OCT
z0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('z'); %Init stage
x0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('x'); %Init stage
y0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('y'); %Init stage

%Move 
if (in.isVerifyMotionRange)
    if (v)
        fprintf('%s Motion Range Test\n',datestr(datetime));
    end
    if (length(in.gridZcc)>1)
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0+min(in.zToScan)); %Movement [mm]
        pause(0.5);
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0+max(in.zToScan)); %Movement [mm]
        pause(0.5);
    end
    
    if (length(in.gridYcc)>1)
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0+min(in.yToScan)); %Movement [mm]
        pause(0.5);
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0+max(in.yToScan)); %Movement [mm]
        pause(0.5);
    end
    
    if (length(in.gridXcc)>1)
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0+min(in.xToScan)); %Movement [mm]
        pause(0.5);
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0+max(in.xToScan)); %Movement [mm]
        pause(0.5);
    end
end

if (v)
    fprintf('%s Done\n',datestr(datetime));
end

%% Make sure folder is empty
if exist(octFolder,'dir')
    rmdir(octFolder,'s');
else
    mkdir(octFolder);
end

%% Preform the scan
for scanI=1:length(scanOrder)
    if (v)
        fprintf('%s Scanning Volume %02d of %d\n',datestr(datetime),scanI,length(scanOrder));
    end
        
    %Move to position
    if length(in.gridZcc)>1
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0+in.gridZcc(scanI)); %Movement [mm]
    end
    if length(in.gridYcc)>1
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0+in.gridYcc(scanI)); %Movement [mm]
    end
    if length(in.gridXcc)>1
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0+in.gridXcc(scanI)); %Movement [mm]
    end
    
    %Make a folder
    s = sprintf('%s\\%s\\',octFolder,in.octFolders{scanI});
    mkdir(s);
    
    ThorlabsImagerNET.ThorlabsImager.yOCTScan3DVolume(...
        in.xCenter,in.yCenter, ... centerX, centerY [mm]
        in.xRange, in.yRange,  ... rangeX,rangeY [mm]
        0,       ... rotationAngle [deg]
        in.nXPixels,in.nYPixels, ... SizeX,sizeY [# of pixels]
        in.nBScanAvg,       ... B Scan Average
        s ... Output directory, make sure it exists before running this function
        );
    
    if(scanI==1)
		%Figure out which OCT System are we scanning in
		a = dir(s);
		names = {a.name}; names([a.isdir]) = [];
		nm = names{round(end/2)};
        if (contains(lower(nm),'ganymede'))
			in.OCTSystem = 'Ganymede_SRR';
		else
			in.OCTSystem = 'NA';
        end
    end
end

%% Finalize

%Home (if required)
pause(0.5);
if length(in.gridZcc)>1
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0); %Movement [mm]
end
if length(in.gridYcc)>1
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0); %Movement [mm]
end
if length(in.gridXcc)>1
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0); %Movement [mm]
end
pause(0.5);

fprintf('%s Finalizing\n',datestr(datetime));
ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose(); %Close scanner

%Save scan configuration parameters
awsWriteJSON(in, [octFolder '\ScanInfo.json']);
json = in;