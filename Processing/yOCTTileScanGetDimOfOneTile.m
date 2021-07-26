function [dimOneTileFromInterf, dimOneTileProcessed] = yOCTTileScanGetDimOfOneTile(inputVolumeFolder, units)
%%This function calculates the dimensions of one tile
%
%USAGE:
% [dimOneTileFromInterf, dimOneTileProcessed] = 
%       yOCTLoadInterfFromFile(inputVolumeFolder, units)
%
% INPUTS:
%   - inputVolumeFolder - Folder path that includes all OCT data folders
%                         and the ScanInfo.json file 
%
% OPTIONAL INPUTS:
%   - units - The desired units for the dimensions of the tile. Must be:
%             'mm' or 'millimeters' - for milimeters
%             'um' or 'microns' - for micrometers
%             'm'  or 'meters' - for meters
%             DEFAULT: 'mm'
%% Input Checks
if (not(isfolder(inputVolumeFolder)))
    error('%s does not exist.', inputVolumeFolder);
end 

if ~exist('units', 'var')
    units = 'mm';
end

%% Extract json
inputVolumeFolder = awsModifyPathForCompetability([fileparts(inputVolumeFolder) '/']);
json = awsReadJSON([inputVolumeFolder 'ScanInfo.json']);

%% Get the data folder used to find dimensions of tile
firstDataFolder = [inputVolumeFolder 'Data01'];

if (not(isfolder(firstDataFolder)))
    error('%s does not exist.', firstDataFolder);
end

%%
dimOneTileFromInterf = yOCTLoadInterfFromFile([firstDataFolder,{'OCTSystem',json.OCTSystem,'peakOnly',true}]);
tmp = zeros(size(dimOneTileFromInterf.lambda.values(:)));
dimOneTileProcessed = yOCTInterfToScanCpx ([{tmp}, {dimOneTileFromInterf}, {'n'}, {json.tissueRefractiveIndex}, {'peakOnly'},{true}]);
dimOneTileFromInterf.z = dimOneTileProcessed.z; %Update only z, not lambda [lambda is changed because of equispacing]
dimOneTileFromInterf.x.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.x.values))*json.xRange;
dimOneTileFromInterf.y.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.y.values))*json.yRange;
dimOneTileFromInterf.x.units = 'millimeters';
dimOneTileFromInterf.y.units = 'millimeters';
dimOneTileFromInterf = yOCTChangeDimensionsStructureUnits(dimOneTileFromInterf, units);
end
