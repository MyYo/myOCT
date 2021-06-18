function json = yOCTPhotobleachTile(varargin)
%This function photobleaches a pattern, the pattern can be bigger than the FOV of the scanner and then the script wii tile around to
%stitch together multiple scans.
%INPUTS:
%   ptStart - point(s) to strat line x and y (in mm). Can be 2Xn matrix for
%       drawing multiple lines. Line position is relative to current's stage position and relative to the opical axis of the lens 
%   ptEnd - corresponding end point (x,y), in mm
%NAME VALUE INPUTS:
%   Parameter               Default Value   Notes
%Probe defenitions:
%   octProbePath            'probe.ini'     Where is the probe.ini is saved to be used
%Photobleaching Parameters:
%   z                       0               Photobleaching depth (compared to corrent position in mm)
%   exposure                15              How much time to expose each spot to laser light. Units sec/mm 
%                                           Meaning for each 1mm, we will expose for exposurePerLine sec  
%   nPasses                 2               Should we expose to laser light in single or multiple passes over the same spot? 
%                                           The lower number of passes the better 
%   oct2stageXYAngleDeg     0               The angle to convert OCT coordniate system to motor coordinate system, see yOCTStageInit
%Constraints
%   enableZone              ones evrywhere  a function handle returning 1 if we can photobleach in that coordinate, 0 otherwise.
%                                           For example, this function will allow photobleaching only in a circle:
%                                           @(x,y)(x^2+y^2 < 2^2). enableZoon accuracy is 10um.
%Debug parameters:
%   v                       true            verbose mode  
%   skipHardware            false           Set to true if you would like to calculate only and not move or photobleach 
%OUTPUT:
%   json with the parameters used for photboleach
  
%% Input Parameters
p = inputParser;
addRequired(p,'ptStart');
addRequired(p,'ptEnd');

%General parameters
addParameter(p,'octProbePath','probe.ini',@isstr);
addParameter(p,'z',0,@isnumeric);
addParameter(p,'exposure',15,@isnumeric);
addParameter(p,'nPasses',2,@isnumeric);
addParameter(p,'enableZone',NaN);
addParameter(p,'oct2stageXYAngleDeg',0,@isnumeric);

addParameter(p,'v',true);
addParameter(p,'skipHardware',false);

parse(p,varargin{:});
json = p.Results;
json.units = 'mm or mm/sec';

enableZone = json.enableZone;
json = rmfield(json,'enableZone');

%Check probe 
if ~exist(json.octProbePath,'file')
	error(['Cannot find probe file: ' json.octProbePath]);
end

%Load probe ini
ini = yOCTReadProbeIniToStruct(json.octProbePath);

%Load FOV
json.FOV = [ini.RangeMaxX ini.RangeMaxY];
FOV = json.FOV;

v = json.v;
json = rmfield(json,'v');

%% Pre processing

if isempty(json.ptStart) || isempty(json.ptEnd)
	%Nothing to photobleach
	return;
end

% Apply Enable Zone
if isa(enableZone,'function_handle')
    [ptStart,ptEnd] = yOCTApplyEnableZone(json.ptStart, json.ptEnd, enableZone, 10e-3);
    json.ptStart = ptStart;
    json.ptEnd = ptEnd;
end

%Find what FOVs should we go to
minX = min([json.ptStart(1,:) json.ptEnd(1,:)]);
maxX = max([json.ptStart(1,:) json.ptEnd(1,:)]);
minY = min([json.ptStart(2,:) json.ptEnd(2,:)]);
maxY = max([json.ptStart(2,:) json.ptEnd(2,:)]);
xCenters = unique([0:(-FOV(1)):(minX-FOV(1)) 0:FOV(1):(maxX+FOV(1))]);
yCenters = unique([0:(-FOV(2)):(minY-FOV(1)) 0:FOV(1):(maxY+FOV(2))]);

%Generate what lines we should draw for each center
[xcc,ycc]=meshgrid(xCenters,yCenters);
xcc = xcc(:); ycc = ycc(:);
ptStartcc = cell(length(xcc),1);
ptEndcc = ptStartcc;
for i=1:length(ptStartcc)
    
    epsilon = 10e-3; % mm, small buffer number 
    [ptStart,ptEnd] = yOCTApplyEnableZone(json.ptStart, json.ptEnd, ...
        @(x,y)( ...
            abs(x-xcc(i))<FOV(1)/2-epsilon*2 & ...
            abs(y-ycc(i))<FOV(2)/2-epsilon*2 ) ...
        , epsilon);

    %Remove lines which are too short to photobleach
    d = sqrt(sum((ptStart - ptEnd).^2,1));
    ptStart(:,d<epsilon) = [];
    ptEnd(:,d<epsilon) = [];
 
    if any( d>FOV(1) | d>FOV(2) )
        error('One (or more) of the photobleach lines is longer than the allowed size, this might cause photobleaching errors!');
    end

    ptStartcc{i} = ptStart;
    ptEndcc{i} = ptEnd;
end

%Remove empty slots
em = cellfun(@isempty,ptStartcc);
ptStartcc(em) = [];
ptEndcc(em) = [];
xcc(em) = [];
ycc(em) = [];

%% Add photonleach instructions to json
clear photobleachInstructions;

for i=1:length(xcc)
    photobleachInstructions(i).stageCenterX = xcc(i);
    photobleachInstructions(i).stageCenterY = ycc(i);
    photobleachInstructions(i).linesPhotobleachedStart = ptStartcc{i};
    photobleachInstructions(i).linesPhotobleachedEnd = ptEndcc{i};
end
json.photobleachInstructions = photobleachInstructions;

%% If skip hardware mode, we are done!
if (json.skipHardware)
    return;
end

%% Initialize Hardware Library

if (v)
    fprintf('%s Initialzing Hardware Dll Library... \n\t(if Matlab is taking more than 2 minutes to finish this step, restart matlab and try again)\n',datestr(datetime));
end
ThorlabsImagerNETLoadLib(); %Init library
if (v)
    fprintf('%s Done Hardware Dll Init.\n',datestr(datetime));
end

%% Initialize Translation Stage
[x0,y0,z0] = yOCTStageInit(json.oct2stageXYAngleDeg, NaN, NaN, v);

%Initialize z translation if photobleaching is not in the current plane
if json.z ~= 0
    yOCTStageMoveTo(NaN,NaN,z0+json.z,v);
end

%Initialize scanner
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit(json.octProbePath); %Init OCT

if (v)
    fprintf('%s Initialzing Motorized Translation Stage Hardware Completed\n',datestr(datetime));
end

%% Photobleach
for i=1:length(xcc)
    if (v && length(xcc) > 1)
        fprintf('%s Moving to positoin (x = %.1fmm, y = %.1fmm) #%d of %d\n',datestr(datetime),xcc(i),ycc(i),i,length(xcc));
    end
    
    yOCTStageMoveTo(x0+xcc(i),y0+ycc(i),NaN,v);
    
    if (v && i==1)
        fprintf('%s Turning Laser Diode On For The First Time... \n\t(if Matlab is taking more than 1 minute to finish this step, restart hardware and try again)\n',datestr(datetime));
        %Switch light on, write to screen only for first line
		% ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(true);  % Version using .NET
		yOCTTurnLaser(true); % Version using Matlab directly
        fprintf('%s Laser Diode is On\n',datestr(datetime)); 
    else
		% Version using .NET
        %evalc('ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(true);'); %Switch light on, use evalc to prevent writing to window
		
		yOCTTurnLaser(true); % Version using Matlab directly
    end

    %Find lines to photobleach, center along current position of the stage
    ptStart = ptStartcc{i} - [xcc(i);ycc(i)];
    ptEnd   = ptEndcc{i}   - [xcc(i);ycc(i)];
    
    %Loop over all lines
    for j=1:size(ptStart,2)
        if (v)
            fprintf('%s \tPhotobleaching Line #%d of %d\n',datestr(datetime),j,size(ptStart,2));
        end

        if (v && i==1 && j==1)
            fprintf('%s Drawing First Line. This is The First Time Galvo Is Moving... \n\t(if Matlab is taking more than a few minutes to finish this step, restart hardware and try again)\n',datestr(datetime));
        end
        
        d = sqrt(sum( (ptStart(:,j) - ptEnd(:,j)).^2));
        ThorlabsImagerNET.ThorlabsImager.yOCTPhotobleachLine( ...
            ptStart(1,j),ptStart(2,j), ... Start X,Y
            ptEnd(1,j),  ptEnd(2,j)  , ... End X,y
            json.exposure*d,  ... Exposure time sec
            json.nPasses); 
        
        if (v && i==1 && j==1)
            fprintf('%s Drew The First Line!\n',datestr(datetime));
        end

    end

    if (v && i==1)
        fprintf('%s Turning Laser Diode Off For The First Time... \n\t(if Matlab is taking more than 1 minute to finish this step, restart hardware and try again)\n',datestr(datetime));
		%Switch light off, write to screen only for first line
		%ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(false); % Version using .NET
		yOCTTurnLaser(false); % Version using Matlab directly
        fprintf('%s Laser Diode is Off\n',datestr(datetime)); 
    else
		% Version using .NET
        %evalc('ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(false);'); %Switch light off, use evalc to prevent writing to window
		yOCTTurnLaser(false); % Version using Matlab directly
    end
    
    pause(0.5);
end

%% Finalize
if (v)
    fprintf('%s Finalizing\n',datestr(datetime));
end

%Return stage to original position
yOCTStageMoveTo(x0,y0,z0,v);

ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose(); %Close scanner


