function ThorlabsImagerNETLoadLib()
%This function loads the library

global ThorlabsImagerNETLoaded
if isempty(ThorlabsImagerNETLoaded) || ~ThorlabsImagerNETLoaded

    currentFileFolder = fileparts(mfilename('fullpath'));
	libFolder = [currentFileFolder '\Lib\'];
	
    % Copy Subfolders to here
	copyfile([libFolder 'LaserDiode\*.*'],libFolder,'f');
	copyfile([libFolder 'MotorController\*.*'],libFolder,'f');
	copyfile([libFolder 'ThorlabsOCT\*.*'],libFolder,'f');
    
    %Load Assembly
    asm = NET.addAssembly([libFolder 'ThorlabsImagerNET.dll']);
    ThorlabsImagerNETLoaded = true;
end