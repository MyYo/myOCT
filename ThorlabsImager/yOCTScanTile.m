function [json] = yOCTScanTile(varargin)
%This function preforms an OCT Scan of a volume, and them tile around to
%stitch together multiple scans. Tiling will be done by 3D translation
%stage.
%INPUTS:
%   octFolder - folder to save all output information
%
%NAME VALUE INPUTS:
%   Parameter               Default Value   Notes
%   octProbePath            'probe.ini'     Where is the probe.ini is saved to be used
%   oct2stageXYAngleDeg     0               The angle to convert OCT coordniate system to motor coordinate system, see yOCTStageInit
%   isVerifyMotionRange     true            Try the full range of motion before scanning, to make sure we won't get 'stuck' through the scan
%   tissueRefractiveIndex   1.4             Refractive index of tissue
%Parameter controling each tile:
%   xOffset,yOffset         0               (0,0) means that the center of the tile scaned is at the center of the galvo range aka lens optical axis. 
%                                           By appling offset, the center of the tile will be positioned differently.Units: mm
%   xRange, yRange          1               Total range of scan x & y in mm. For example, if xOffset=0, xRange=1, OCT will scan from -0.5 to 0.5mm.
%   nXPixels                1000            Number of pixels in x direction (equally spaced)
%   nYPixels                1000            Number of pixels in y direction (equally spaced)
%   nBScanAvg               1               How many B Scan Averaging to scan
%Scan tiling parameters, these will cerate a meshgrid relative to position
%   of stage at the beginning of the scan.
%   x,y,z parameters in tiling, are in the same direction as x,y,z of the
%   sacn, you can look at it as an extention of the size of the lens. 
%   xCenters,yCenters       0               Center positions of each tiles to scan (x,y) Units: mm. 
%                                           Example: 'xCenters', [0 1], 'yCenters', [0 1], 
%                                           will scan 4 OCT volumes centered around [0 0 1 1; 0 1 0 1] + [xOffset; yOffset]
%   zDepths                 0               Scan depths to scan. Positive value is deeper). Units: mm
%	unzipOCTFile			true			Scan will scan .OCT file, if you would like to automatically unzip it set this to true.
%Debug parameters:
%   v                       true            verbose mode      
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
addParameter(p,'oct2stageXYAngleDeg',0,@isnumeric);
addParameter(p,'isVerifyMotionRange',true,@islogical);
addParameter(p,'tissueRefractiveIndex',1.4,@isnumeric);

%Single scan parmaeters
addParameter(p,'xOffset',0,@isnumeric);
addParameter(p,'yOffset',0,@isnumeric);
addParameter(p,'xRange',1,@isnumeric);
addParameter(p,'yRange',1,@isnumeric);
addParameter(p,'nXPixels',1000,@isnumeric);
addParameter(p,'nYPixels',1000,@isnumeric);
addParameter(p,'nBScanAvg',1,@isnumeric);

%Tile Parameters
addParameter(p,'xCenters',0,@isnumeric);
addParameter(p,'yCenters',0,@isnumeric);
addParameter(p,'zDepths',0,@isnumeric);
addParameter(p,'unzipOCTFile',true);

%Debugging
addParameter(p,'v',true,@islogical);

parse(p,varargin{:});

in = p.Results;
octFolder = in.octFolder;
v = in.v;
in = rmfield(in,'octFolder');
in = rmfield(in,'v');
in.units = 'mm'; %All units are mm
in.version = 1.1; %Version of this file

if ~exist(in.octProbePath,'file')
	error(['Cannot find probe file: ' in.octProbePath]);
end

%% Parse our parameters from probe
in.octProbe = yOCTReadProbeIniToStruct(in.octProbePath);

% If set, will protect lens from going into deep to the sample hiting the lens. Units: mm.
% The way it works is it computes what is the span of zDepths, compares that to working distance + safety buffer
% If the number is too high, abort will be initiated.
if isfield(in.octProbe,'ObjectiveWorkingDistance')
    objectiveWorkingDistance = in.octProbe.ObjectiveWorkingDistance;
else
    objectiveWorkingDistance = Inf;
end


%% Scan center list

%Scan order, z changes fastest, x after, y latest
[in.gridXcc, in.gridZcc,in.gridYcc] = meshgrid(in.xCenters,in.zDepths,in.yCenters); 
in.gridXcc = in.gridXcc(:);
in.gridYcc = in.gridYcc(:);
in.gridZcc = in.gridZcc(:);
in.scanOrder = 1:length(in.gridZcc);
in.octFolders = arrayfun(@(x)(sprintf('Data%02d',x)),in.scanOrder,'UniformOutput',false);

scanOrder = in.scanOrder;

%% Initialize hardware
if (v)
    fprintf('%s Initialzing Hardware...\n\t(if Matlab is taking more than 2 minutes to finish this step, restart hardware and try again)\n',datestr(datetime));
end
 
ThorlabsImagerNETLoadLib(); %Init library
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit(in.octProbePath); %Init OCT

if (v)
    fprintf('%s Initialzing Hardware Completed\n',datestr(datetime));
end

% Make sure depths are ok for working distance's sake 
if (max(in.zDepths) - min(in.zDepths) > objectiveWorkingDistance ...
        - 0.5) %Buffer
    error('zDepths requested are from %.1mm to %.1mm, which is too close to lens working distance of %.1fmm. Aborting', ...
        min(in.zDepths), max(in.zDepths), objectiveWorkingDistance);
end

% Init stage and verify range if needed
if in.isVerifyMotionRange
    rg_min = [min(in.xCenters) min(in.yCenters) min(in.zDepths)];
    rg_max = [max(in.xCenters) max(in.yCenters) max(in.zDepths)];
else
    rg_min = NaN;
    rg_max = NaN;
end
[x0,y0,z0] = yOCTStageInit(in.oct2stageXYAngleDeg,rg_min,rg_max,v);

if (v)
    fprintf('%s Done\n',datestr(datetime));
end

%% Make sure folder is empty
if exist(octFolder,'dir')
    rmdir(octFolder,'s');
end
mkdir(octFolder);

%% Preform the scan
for scanI=1:length(scanOrder)
    if (v)
        fprintf('%s Scanning Volume %02d of %d\n',datestr(datetime),scanI,length(scanOrder));
    end
        
    %Move to position
    yOCTStageMoveTo(x0+in.gridXcc(scanI),y0+in.gridYcc(scanI),z0+in.gridZcc(scanI));
    
    %Make a folder
    s = sprintf('%s\\%s\\',octFolder,in.octFolders{scanI});
    s = awsModifyPathForCompetability(s);
    
    ThorlabsImagerNET.ThorlabsImager.yOCTScan3DVolume(...
        in.xOffset+in.octProbe.DynamicOffsetX, in.yOffset, ... centerX, centerY [mm]
        in.xRange.*in.octProbe.DynamicFactorX, in.yRange,  ... rangeX,rangeY [mm]
        0,       ... rotationAngle [deg]
        in.nXPixels,in.nYPixels, ... SizeX,sizeY [# of pixels]
        in.nBScanAvg,       ... B Scan Average
        s ... Output directory, make sure this folder doesn't exist when starting the scan
        );
    
	if in.unzipOCTFile
		yOCTUnzipOCTFolder(strcat(s, 'VolumeGanymedeOCTFile.oct'),s,true);
	end
    
    if(scanI==1)
		%Figure out which OCT System are we scanning in
		%a = dir(s);
		%names = {a.name}; names([a.isdir]) = [];
		%nm = names{2};
        %if (contains(lower(nm),'ganymede'))
		%	in.OCTSystem = 'Ganymede';
		%else
		%	in.OCTSystem = 'NA';
        %end
        [OCTSystem, OCTSystemManufacturer] = yOCTLoadInterfFromFile_WhatOCTSystemIsIt(s);
        in.OCTSystem = OCTSystem;
    end
    
end

%% Finalize

if (v)
    fprintf('%s Homing...\n',datestr(datetime));
end

%Home 
pause(0.5);
yOCTStageMoveTo(x0,y0,z0);
pause(0.5);

if (v)
    fprintf('%s Finalizing\n',datestr(datetime));
end
ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose(); %Close scanner

%Save scan configuration parameters
awsWriteJSON(in, [octFolder '\ScanInfo.json']);
json = in;