function ThorlabsImagerNETLoadLib()
%This function loads the library

if isempty(which('ThorlabsImagerNET.ThorlabsImager')) %If the dll was not loaded

    currentFileFolder = fileparts(mfilename('fullpath'));
	libFolder = [currentFileFolder '\Lib\'];
	
	if ~exist([libFolder 'SpectralRadar.dll'],'file')
		% Copy Subfolders to main lib folder
		copyfile([libFolder 'LaserDiode\*.*'],libFolder,'f');
		copyfile([libFolder 'MotorController\*.*'],libFolder,'f');
		copyfile([libFolder 'ThorlabsOCT\*.*'],libFolder,'f');
	end
    
    %Load Assembly
    asm = NET.addAssembly([libFolder 'ThorlabsImagerNET.dll']);
end