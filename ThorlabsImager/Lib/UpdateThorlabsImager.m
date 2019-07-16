%Run this script to update library protofile

%% Add Path
currentFileFolder = fileparts(mfilename('fullpath'));
addpath(genpath(currentFileFolder));

%% Copy Subfolders to here
copyfile('LaserDiode\*.dll','.')
copyfile('MotorController\*.dll','.')
copyfile('ThorlabsOCT\*.dll','.')

%% Build Protofile
loadlibrary('ThorlabsImager','ThorlabsImager','mfilename','ThorlabsImager');