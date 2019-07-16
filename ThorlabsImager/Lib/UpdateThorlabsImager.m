%Run this script to update library protofile

%% Add Path
currentFileFolder = fileparts(mfilename('fullpath'));
addpath(genpath(currentFileFolder));

%% Copy Subfolders to here
copyfile('LaserDiode\*.*','.')
copyfile('MotorController\*.*','.')
copyfile('ThorlabsOCT\*.*','.')

%% Build Protofile
loadlibrary('ThorlabsImager','ThorlabsImager','mfilename','ThorlabsImager');