function ThorlabsImagerNETLoadLib()
%This function loads the library

global ThorlabsImagerNETLoaded
if isempty(ThorlabsImagerNETLoaded) || ~ThorlabsImagerNETLoaded
    % Copy Subfolders to here
	copyfile('Lib\LaserDiode\*.*','.')
	copyfile('Lib\MotorController\*.*','.')
	copyfile('Lib\ThorlabsOCT\*.*','.')
    
    %Load Assembly
    currentFileFolder = fileparts(mfilename('fullpath'));
    asm = NET.addAssembly([currentFileFolder '\Lib\ThorlabsImagerNET.dll']);
    ThorlabsImagerNETLoaded = true;
end