function [surfacePosition_mm, x_mm, y_mm] = yOCTScanAndFindTissueSurface(varargin)
% This function uses the OCT to scan and then identify tissue surface from 
% the OCT image.
% INPUTS (all inputs are parameters):
%   xRange_mm, yRange_mm - what range to scan, default [-1 1] mm.
%   resolution_mm - pixel resolution, default: 0.050 mm.
%   isVisualize - set to true to generate image heatmap visualization
%       figure. Default is false
% OUTPUTS:
%   - surfacePosition_mm - 2D matrix. dimensions are (y,x). What
%       height is image surface (measured from focal position at z=0 scan).
%       physical dimensions of surfacePosition is mm.
%   - x_mm ,y_mm are the x,y positions that corresponds to surfacePosition(y,x).
%       Units are mm.

%% Parse inputs
p = inputParser;
addParameter(p,'isVisualize',false);
addParameter(p,'xRange_mm',[-1 1]);
addParameter(p,'yRange_mm',[-1 1]);
addParameter(p,'resolution_mm',50e-3);

parse(p,varargin{:});
in = p.Results;
xRange_mm = in.xRange_mm;
yRange_mm = in.yRange_mm;
resolution_mm = in.resolution_mm;
isVisualize = in.isVisualize;

%% Scan
% Todo

%% Estimate tissue surface
% Todo

%% Clean up
% delete temporary files

%% Return
% Tbd, this is just a demo
surfacePosition_mm = ones(10,10);
x_mm=1:10;
y_mm=1:10;
