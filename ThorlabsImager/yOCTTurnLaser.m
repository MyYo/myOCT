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

[~,txt] = runSystem(['"' lib 'DiodeCtrl.exe" ' newStateText]); 

%% Process output
if ~isempty(txt)
	disp(txt);
end

if ~isempty(strfind(lower(txt),'error'))
	% Try again
	pause(0.5);
	runSystem(['"' lib 'DiodeCtrl.exe" ' newStateText]); 
end


function [output1, output2] = runSystem(text)
t = tic();

[output1, output2] = system(text); 

t = toc(t);
% Check to see if the time it took to switch off/on the laser is more than a few seconds raise an error
if (t>4)
    warning('Laser diode took way too long to switch off (%d seconds), this may be a problem.\n This is what we tried to run "%s", These are the outputs "%s","%s"',...
        t,text,output1,output2);
end