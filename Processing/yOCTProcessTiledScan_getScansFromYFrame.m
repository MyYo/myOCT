function [scanPaths, yIInFile] = ...
    yOCTProcessTiledScan_getScansFromYFrame(...
    yFrameIndexInOutputVolume, tiledScanInputFolder, focusPositionInImageZpix)
% This is an auxilary function of yOCTProcessTiledScan designed to get a y
% frame index in the overall tiled scan and return the exact tiles which
% contain the data corresponding to that y frame.
% INPUTS:
%   yFrameIndexInOutputVolume - frame index in the output coordinates
%   tiledScanInputFolder - all tile folder path
% OUTPUTS:
%   scanPaths - a cell array containing volume paths to the tiles that
%       contain yFrameIndexInOutputVolume. File paths are organized by x
%       then z index in case of stitching in those directions as well.
%   yIInFile - The y frame index in the tile (1 base index)

%% Load json and gather information
json = awsReadJSON([tiledScanInputFolder 'ScanInfo.json']);
[dimOneTile, dimOutput] = yOCTProcessTiledScan_createDimStructure(tiledScanInputFolder, focusPositionInImageZpix);

% Parse all scan paths
scanPaths = cellfun(@(x)(awsModifyPathForCompetability([tiledScanInputFolder '\' x '\'])),json.octFolders,'UniformOutput',false);

%% Figure out which scan this y is at 

% For each of the ys in dimOutput, figure out which tile it goes
if mod(length(dimOutput.y.values),length(dimOneTile.y.values)) ~=0
    error('One tile should be devisible by all')
end
whichYTile = repelem(1:length(dimOutput.y.values)/length(dimOneTile.y.values),length(dimOneTile.y.values));

% Find out which one
yTileIndex = whichYTile(yFrameIndexInOutputVolume);

% Local index
yIInFile = yFrameIndexInOutputVolume - find(whichYTile==yTileIndex,1,'first') + 1;

%% Find which tiles have the yFrameIndexInOutputVolume
if ~isfield(json,'xCenters_mm')
    warning('Note, that "%s" contains an old version scan, this will be depricated by Jan 1st, 2025',tiledScanInputFolder)
    % Backward compatibility
    relevantFilesIndex = json.gridYcc==json.yCenters(yTileIndex);
else
    relevantFilesIndex = json.gridYcc==json.yCenters_mm(yTileIndex);
end

scanPaths = scanPaths(relevantFilesIndex);
