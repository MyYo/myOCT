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
%   FOV                     [Inf, Inf]      FOV of the lens (mm, x and y)
%Photobleaching Parameters:
%   z                       0               Photobleaching depth (compared to corrent position in mm)
%   exposure                15              How much time to expose each spot to laser light. Units sec/mm 
%                                           Meaning for each 1mm, we will expose for exposurePerLine sec  
%   nPasses                 2               Should we expose to laser light in single or multiple passes over the same spot? 
%                                           The lower number of passes the better 
%Constraints
%   enableZone              ones evrywhere  a function handle returning 1 if we can photobleach in that coordinate, 0 otherwise.
%                                           For example, this function will allow photobleaching only in a circle:
%                                           @(x,y)(x^2+y^2 < 2^2). enableZoon accuracy is 10um.
%Debug parameters:
%   v                       true            verbose mode  
%OUTPUT:
%   json with the parameters used for photboleach
  
%% Input Parameters
p = inputParser;
addRequired(p,'ptStart');
addRequired(p,'ptEnd');

%General parameters
addParameter(p,'octProbePath','probe.ini',@isstr);
addParameter(p,'FOV',[Inf Inf],@isnumeric);
addParameter(p,'z',0,@isnumeric);
addParameter(p,'exposure',15,@isnumeric);
addParameter(p,'nPasses',2,@isnumeric);
addParameter(p,'enableZone',NaN);

addParameter(p,'v',true);

parse(p,varargin{:});
json = p.Results;
json.units = 'mm or mm/sec';

enableZone = json.enableZone;
json = rmfield(json,'enableZone');

%If one value is given for FOV, assume x and y FOVs are the same
if length(json.FOV) == 1
    json.FOV = json.FOV*[1 1];
end
FOV = json.FOV;

%Check probe 
if ~exist(json.octProbePath,'file')
	error(['Cannot find probe file: ' json.octProbePath]);
end

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
    [ptStart,ptEnd] = yOCTApplyEnableZone(json.ptStart, json.ptEnd, ...
        @(x,y)( abs(x-xcc(i))<FOV(1)/2 & abs(y-ycc(i))<FOV(2)/2 ) ...
        , 10e-3);

    %Remove lines which are too short to photobleach
    d = sqrt(sum((ptStart - ptEnd).^2,1));
    ptStart(:,d<10e-3) = [];
    ptEnd(:,d<10e-3) = [];

    ptStartcc{i} = ptStart;
    ptEndcc{i} = ptEnd;
end

%Remove empty slots
em = cellfun(@isempty,ptStartcc);
ptStartcc(em) = [];
ptEndcc(em) = [];
xcc(em) = [];
ycc(em) = [];

%% Initialize Hardware

if (v)
    fprintf('%s Initialzing Hardware\n',datestr(datetime));
end

ThorlabsImagerNETLoadLib(); %Init library

%Initialize x,y stage if we need to translate
if length(xcc) > 1
    x0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('x'); %Init stage
    y0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('y'); %Init stage
end

%Initialize z translation if photobleaching is not in the current plane
if json.z ~= 0
    z0=ThorlabsImagerNET.ThorlabsImager.yOCTStageInit('z'); %Init stage
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0+json.z); %Movement [mm]
end

%Initialize scanner
ThorlabsImagerNET.ThorlabsImager.yOCTScannerInit(json.octProbePath); %Init OCT

%% Photobleach
for i=1:length(xcc)
    if (v && length(xcc) > 1)
        fprintf('%s Moving to positoin (x = %.1fmm, y = %.1fmm) #%d of %d\n',datestr(datetime),xcc(i),ycc(i),i,length(xcc));
    end

    if length(xcc) > 1
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0+xcc(i)); %Movement [mm]
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0+ycc(i)); %Movement [mm]
    end

    if (i==1)
        ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(true); %Switch light on, write to screen only for first line
    else
        evalc('ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(true);'); %Switch light on, use evalc to prevent writing to window
    end

    %Find lines to photobleach, center along current position of the stage
    ptStart = ptStartcc{i} - [xcc(i);ycc(i)];
    ptEnd   = ptEndcc{i}   - [xcc(i);ycc(i)];
    
    %Loop over all lines
    for j=1:size(ptStart,2)
        if (v)
            fprintf('%s \tPhotobleaching Line #%d of %d\n',datestr(datetime),j,size(ptStart,2));
        end

        d = sqrt(sum( (ptStart(:,j) - ptEnd(:,j)).^2));
        ThorlabsImagerNET.ThorlabsImager.yOCTPhotobleachLine( ...
            ptStart(1,j),ptStart(2,j), ... Start X,Y
            ptEnd(1,j),  ptEnd(2,j)  , ... End X,y
            json.exposure*d,  ... Exposure time sec
            json.nPasses); 

    end

    evalc('ThorlabsImagerNET.ThorlabsImager.yOCTTurnLaser(false);'); %Switch light off, use evalc to prevent writing to window
    pause(0.5);
end

%% Finalize
if (v)
    fprintf('%s Finalizing\n',datestr(datetime));
end

%Return stage to original position
if length(xcc) > 1
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('x',x0);
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('y',y0);
end
if json.z ~= 0
    ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition('z',z0);
end

ThorlabsImagerNET.ThorlabsImager.yOCTScannerClose(); %Close scanner


