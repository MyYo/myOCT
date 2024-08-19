function [dimOneTileFromInterf, dimOneTileProcessed] = yOCTTileScanGetDimOfOneTile(inputVolumeFolder, units)
%%This function calculates the dimensions of one tile
%
%USAGE:
% [dimOneTileFromInterf, dimOneTileProcessed] = 
%       yOCTLoadInterfFromFile(inputVolumeFolder, units)
%
% INPUTS:
%  inputVolumeFolder - Folder path that includes all OCT data folders
%                      and the ScanInfo.json file 
%
% OPTIONAL INPUTS:
%  units - The desired units for the dimensions of the tile. Must be:
%          'mm' or 'millimeters' - for milimeters
%          'um' or 'microns' - for micrometers
%          'm'  or 'meters' - for meters
%          DEFAULT: 'mm'
% OUTPUTS:
%  dimOneTileFromInterf - Dimension structure read by the interf function
%  dimOneTileProcessed  - Dimension structure after interferogram converted
%                         to z values.

%% Input Checks
if (not(awsExist(inputVolumeFolder,'dir')))
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

if (not(awsExist(firstDataFolder,'dir')))
    error('%s does not exist.', firstDataFolder);
end

%% G
dimOneTileFromInterf = yOCTLoadInterfFromFile([firstDataFolder,{'OCTSystem',json.OCTSystem,'peakOnly',true}]);
tmp = zeros(size(dimOneTileFromInterf.lambda.values(:)));
dimOneTileProcessed = yOCTInterfToScanCpx ([{tmp}, {dimOneTileFromInterf}, {'n'}, {json.tissueRefractiveIndex}, {'peakOnly'},{true}]);
dimOneTileFromInterf.z = dimOneTileProcessed.z; %Update only z, not lambda [lambda is changed because of equispacing]
if isfield(json,'xRange')
    warning('Note, that "%s" contains an old version scan, this will be depricated by Jan 1st, 2025')
    dimOneTileFromInterf.x.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.x.values))*json.xRange;
    dimOneTileFromInterf.y.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.y.values))*json.yRange;
else
    dimOneTileFromInterf.x.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.x.values))*json.tileRangeX_mm;
    dimOneTileFromInterf.y.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.y.values))*json.tileRangeY_mm;
end
dimOneTileFromInterf.x.units = 'millimeters';
dimOneTileFromInterf.y.units = 'millimeters';
dimOneTileFromInterf = yOCTChangeDimensionsStructureUnits(dimOneTileFromInterf, units);
