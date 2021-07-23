function [dimOneTileFromInterf, dimOneTileProcessed] = yOCTTileScanGetDimOfOneTile(varargin)
%%This function calculates the dimensions of one tile
%
%USAGE:
% [dimOneTileFromInterf, dimOneTileProcessed] = 
%       yOCTLoadInterfFromFile(units, inputDataFolder, 'param1',value1,'param2',value2,...)
%
% INPUTS:
%   - xRange - The x range of the scanned sample. This value can be found
%              in the ScanConfig.json file belonging to the volume
%   - yRange - The y range of the scanned sample. This value can be found
%              in the ScanConfig.json file belonging to the volume
%   - units - The desired units for the dimensions of the tile. Must be:
%       'mm' or 'millimeters' - for milimeters
%       'um' or 'microns' - for micrometers
%       'm'  or 'meters' - for meters
%   - inputDataFolder - OCT data folder / AWS data folder (s3:\)
% LIST OF OPTIONAL PARAMETERS AND VALUES
% yOCTLoadInterfFromFile PARAMETERS
% Parameter                 Default     Information & Values
% 'OCTSystem'               ''          OCT System Name, can be 'Ganymede', 'Telesto' or 'Wasatch'.
%                                       If loading SRR files use 'Ganymede_SRR' or 'Telesto_SRR'.
%                                       If set to '', will try to figure out system from the file type.
% 'BScanAvgFramesToProcess' all         What B-Scan Average frame indexies to process. 
%                                       Usefull in cases where scan size is too big to be stored in memory, thus only part of the scan is loaded    
%                                       Index starts at 1.
% 'YFramesToProcess'        all         What Y frames to process indexies (applicable only for 3D scans).
%                                       Usefull in cases where scan size is too big to be stored in memory, thus only part of the scan is loaded 
%                                       Index starts at 1.
% 'PeakOnly'                false       when set to true, this function will only read file header without reading all dataa
%                                       and return data dimensions without computing the interferogram
%                                       Usage: dimensions = yOCTLoadInterfFromFile(...)
% 'ApodizationCorrection'   'Subtract'  What kind apodization correction to do. Can be
%                                       'Subtract' - Subtract apodization signal from interferogram
%                                       'None' - No ApodizationCorrection. Raw data is loaded.
% 'Chirp'                   []          If you loaded chirpfile once andyou have the chirp data, just pass it along here. 
%                                       If not, this function will downoload it. 
%                                       Upplicable for thorlabs systems only.
%
% yOCTInterfToScanCpx PARAMETERS
% Parameter                 Default     Information & Values
% 'dispersionQuadraticTerm'   100       Quadradic phase correction units of [nm^2/rad]. 
%                                       Default value is 100[nm^2/rad], try increasing to
%                                       40*10^6 if the lens or system is not dispersion corrected.
%                                       try running Demo_DispersionCorrection, if unsure of the number
%                                       you need. This number is sometimes reffered to as beta.
%
% 'band',                     []        [start end] - Use a Hann filter to filter out part of the
%                                       spectrum. Units are [nm]. Default is all spectrum
%
% 'interpMethod'              []        See help yOCTEquispaceInterf for interpetation methods
%
% 'n'                         1.33      Medium refractive index
%
% 'peakOnly'                  False     If set to true, only returns dimensions update 
%                                       dimensions = yOCTInterfToScanCpx (varargin)
% OUTPUTS:
%   - dimOneTileFromInterf - describing the interferogram matrix dimensions. Please
%                            see the description of the OUTPUT of
%                            yOCTLoadInterfFromFile.m for more details 
%   - dimOneTileProcessed - Dimensions of tile after interferogram is
%                           converted into a complex scanCpx datastructure
%                           by yOCTInterfToScanCpx
%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

%% Extract desired xRange, yRange, and units for dimensions
xRange = varargin{1};
yRange = varargin{2};
units = varargin{3};

%%
dimOneTileFromInterf = yOCTLoadInterfFromFile({varargin{4:end}});
tmp = zeros(size(dimOneTileFromInterf.lambda.values(:)));
dimOneTileProcessed = yOCTInterfToScanCpx ([{tmp}, {dimOneTileFromInterf},varargin{5:end}]);
dimOneTileFromInterf.z = dimOneTileProcessed.z; %Update only z, not lambda [lambda is changed because of equispacing]
dimOneTileFromInterf.x.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.x.values))*xRange;
dimOneTileFromInterf.y.values = linspace(-0.5,0.5,length(dimOneTileFromInterf.y.values))*yRange;
dimOneTileFromInterf.x.units = 'millimeters';
dimOneTileFromInterf.y.units = 'millimeters';
dimOneTileFromInterf = yOCTChangeDimensionsStructureUnits(dimOneTileFromInterf, units);
end
