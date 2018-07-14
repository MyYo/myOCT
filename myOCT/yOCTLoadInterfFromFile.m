function [interferogram, dimensions, apodization,prof] = yOCTLoadInterfFromFile(varargin)
%%This function loads an interferogram data from file and preforms initial
%interferogram preprocessing
%
%USAGE:
% [interferogram, dimensions, apodization] = 
%       yOCTLoadInterfFromFile(inputDataFolder,'param1',value1,'param2',value2,...)
%
% INPUTS:
%   - inputDataFolder - OCT data folder / AWS data folder
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                 Default     Information & Values
% 'OCTSystem'               'Ganymede'  OCT System Name, can be 'Ganymede' or 'Telesto'
% 'BScanAvgFramesToProcess' all         What B-Scan Average frame indexies to process. 
%                                       Usefull in cases where scan size is too big to be stored in memory, thus only part of the scan is loaded    
%                                       Index starts at 1.
% 'YFramesToProcess'        all         What Y frames to process indexies (applicable only for 3D scans).
%                                       Usefull in cases where scan size is too big to be stored in memory, thus only part of the scan is loaded 
%                                       Index starts at 1.
% 'PeakOnly'                false       when set to true, this function will only read file header without reading all dataa
%                                       and return data dimensions without computing the interferogram
%                                       Usage: dimensions = yOCTLoadInterfFromFile(...)
% OUTPUTS:
%   - interferogram - interferogram data, apodization corrected. 
%       Dimensions order (lambda,x,y,AScanAvg,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - dimensions  - interferogram dimensions structure, containing dimension order and values. 
%   - apodization - OCT baseline intensity, without the tissue scatterers.
%       Dimensions order (lambda,podization #,y,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - prof - profiling data - for debug purposes 

%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

%Optional Parameters
OCTSystem = 'Ganymede';
peakOnly = false;
for i=2:2:length(varargin)
    switch(lower(varargin{i}))
        case 'octsystem'
            OCTSystem = varargin{i+1};
        case 'dimensions'
            dimensions = varargin{i+1};
        case 'yframestoprocess'
            YFramesToProcess = varargin{i+1};
            YFramesToProcess = YFramesToProcess(:);
        case 'bscanavgframestoprocess'
            BScanAvgFramesToProcess = varargin{i+1};
            BScanAvgFramesToProcess = BScanAvgFramesToProcess(:);
        case 'peakonly'
            peakOnly = varargin{i+1};
    end
end

%% Fix input data folder if required
inputDataFolder = varargin{1};
if (inputDataFolder(end) ~='/' && inputDataFolder(end) ~='\')
    inputDataFolder = [inputDataFolder '/'];
end

%% Load Header file, get dimensions
tt = tic;
if ~exist('dimensions','var')
    %Load header information
    switch(OCTSystem)
        case {'Ganymede','Telesto'}
            dimensions = yOCTLoadInterfFromFile_ThorlabsHeader(inputDataFolder,OCTSystem);
        case {'Wasatch'}
            dimensions = yOCTLoadInterfFromFile_WasatchHeader(inputDataFolder);
        otherwise
            error('ERROR: Wrong OCTSystem name! (yOCTLoadInterfFromFile)')
    end
else
    %Header information given by user
end

%Correct dimensions according to what user asks to process
if exist('YFramesToProcess','var')
    if (length(dimensions.y.index) == 1)
        %Only y frame to process already
        if (dimensions.y.index ~= YFramesToProcess)
            error('Asking to process non existant frame');
        else
            %We already have one frame to process, nothing to do..
        end
    else
        %Process only subset of Y frames
        dimensions.y.values = dimensions.y.values(YFramesToProcess);
        dimensions.y.index = dimensions.y.index(YFramesToProcess);
        dimensions.y.index = dimensions.y.index(:)';

        if (length(YFramesToProcess)==1) %Practically, we don't have a Y dimenson
            dimensions.y.order = NaN;
            if isfield(dimensions,'AScanAvg')
                dimensions.AScanAvg.order = dimensions.AScanAvg.order - 1;
            end
            if isfield(dimensions,'BScanAvg')
                dimensions.BScanAvg.order = dimensions.BScanAvg.order - 1;
            end
        end
    end
end
if exist('BScanAvgFramesToProcess','var') && isfield(dimensions,'BScanAvg')
    %Process only subset of BScanAvgFramesToProcess frames
    dimensions.BScanAvg.index = dimensions.BScanAvg.index(BScanAvgFramesToProcess);
    dimensions.BScanAvg.index = dimensions.BScanAvg.index(:)';
end

headerLoadTimeSec = toc(tt);
 
if (peakOnly == true)
    %We are done!
    interferogram = dimensions;
    apodization = dimensions;
    prof.headerLoadTimeSec = headerLoadTimeSec;
    return;
end

%% Load Data - System Specific Configuration
switch(OCTSystem)
    case {'Ganymede','Telesto'}
        [interferogram, apodization, prof] = yOCTLoadInterfFromFile_ThorlabsData([varargin {'dimensions'} {dimensions}]);
    case {'Wasatch'}
        [interferogram, apodization, prof] = yOCTLoadInterfFromFile_WasatchData([varargin {'dimensions'} {dimensions}]);
end

%% Correct For Apodization
[~, sizeX, sizeY, AScanAvgN, BScanAvgN] = yOCTLoadInterfFromFile_DataSizing(dimensions);   
apod = mean(apodization,2); %Mean across x axis
s = size(apod);
s = [s 1 1 1 1 1]; %Pad with ones

interferogram = interferogram - repmat(apod,[1 sizeX sizeY/s(3) AScanAvgN BScanAvgN/s(5)]);

%% Finish
interferogram = squeeze(interferogram);
apodization = squeeze(apodization);
prof.headerLoadTimeSec = headerLoadTimeSec;