function [interferogram, dimensions, apodization,prof] = yOCTLoadInterfFromFile(varargin)
%USAGE:
% [interferogram, dimensions, apodization] = 
%       yOCTLoadInterfFromFile(inputDataFolder,'param1',value1,'param2',value2,...)
%This function loads an interferogram data from file and preforms initial
%interferogram preprocessing
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
% 'PeakOnly'                false       when set to true, this function will return 
%                                       dimensions without computing the interferogram
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
for i=2:2:length(varargin)
    switch(lower(varargin{i}))
        case 'octsystem'
            OCTSystem = varargin{i+1};
    end
end

%% System Specific Configuration
switch(OCTSystem)
    case {'Ganymede','Telesto'}
        [interferogram, dimensions, apodization,prof] = yOCTLoadInterfFromFile_Thorlabs(varargin);
    case {'Wasatch'}
        [interferogram, dimensions, apodization,prof] = yOCTLoadInterfFromFile_Wasatch(varargin);
    otherwise
        error('ERROR: Wrong OCTSystem name! (yOCTLoadInterfFromFile)')
end
