function yOCTStageMoveTo (newx,newy,newz)
% Move stage to new position.
% INPUTS:
%   newx,newy,newz - new stage position (mm). Set to nan if you would like
%       not to move stage along some axis. These new position units are in
%       OCT coordinate system units. A conversion between OCT coordinate
%       system to the stage coordinate system is done via
%       goct2stageXYAngleDeg which is set in yOCTStageInit

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

s = 'xyz';
for i=1:3
    if abs(d_(i)) > 1e-3 % Move if motion of more than epsilon is needed 
        ThorlabsImagerNET.ThorlabsImager.yOCTStageSetPosition(s(i),gStageCurrentPosition(i)); %Movement [mm]
    end
end

