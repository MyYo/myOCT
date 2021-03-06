function yOCTTurnLaser(newState)
% Instructions for this code follow thorlab's example below.
% For more information visit https://www.thorlabs.com/software_pages/ViewSoftwarePage.cfm?Code=4000_Series&viewtab=3
% This function will turn laser diode on or off
% INPUTS:
%   newState - set to true to turn on, false to turn off


%% Input processing

if newState
    newStateText = 'ON';
else
    newStateText = 'OFF';
end

%% Run utility 
currentFileFolder = fileparts(mfilename('fullpath'));
lib	 = [currentFileFolder '\Lib\'];

[~,txt] = system(['"' lib 'DiodeCtrl.exe" ' newStateText]); 

%% Process output
if ~isempty(txt)
	disp(txt);
end

if ~isempty(strfind(lower(txt),'error'))
	% Try again
	pause(0.5);
	system(['"' lib 'DiodeCtrl.exe" ' newStateText]); 
end
