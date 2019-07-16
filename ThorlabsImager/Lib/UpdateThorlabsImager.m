%Run this script to update library protofile

%% Add Path
currentFileFolder = fileparts(mfilename('fullpath'));
addpath(genpath(currentFileFolder));

%% Build Protofile
loadlibrary('ThorlabsImager','ThorlabsImager','mfilename','ThorlabsImager');