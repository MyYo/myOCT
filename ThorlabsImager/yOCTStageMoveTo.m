function yOCTStageMoveTo (newx,newy,newz,v)
% Move stage to new position.
% INPUTS:
%   newx,newy,newz - new stage position (mm). Set to nan if you would like
%       not to move stage along some axis. These new position units are in
%       OCT coordinate system units. A conversion between OCT coordinate
%       system to the stage coordinate system is done via
%       goct2stageXYAngleDeg which is set in yOCTStageInit
%   v - verbose mode (default is false)

%% Input checks
if ~exist('newx','var')
    newx = NaN;
end

if ~exist('newy','var')
    newy = NaN;
end

if ~exist('newz','var')
    newz = NaN;
end

if ~exist('v','var')
    v = false;
end

%% Compute how much translation in each axis is needed
global gStageCurrentPosition;
d = [newx;newy;newz]-gStageCurrentPosition(:);

d(isnan(d)) = 0; % Where there is nan, we don't need to move, keep as is

%% Convert Coordinate System
global goct2stageXYAngleDeg;
c = cos(goct2stageXYAngleDeg*pi/180);
s = sin(goct2stageXYAngleDeg*pi/180);
d_ = [c -s 0; s c 0; 0 0 1]*d;

%% Update position and move
gStageCurrentPosition = gStageCurrentPosition + d_;

if (v)
    fprintf('New Stage Position (in Stage Coordinate System): (%.3f, %.3f, %.3f) mm.\n',gStageCurrentPosition);
end

s = 'xyz';
for i=1:3
    if abs(d_(i)) > 0 % Move if motion of more than epsilon is needed 
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition(s(i),gStageCurrentPosition(i)); %Movement [mm]
    end
end

