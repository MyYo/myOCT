%This function Loads OCT Scan using cloud

close all;

%% Inputs
filepath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede3D_BScanAvg/';

%parpool('local');

m = yOCTLoadScan(filepath,'OCTSystem','Ganymede',...
    'YFramesToProcess',1:10, ... %160,...
    'nYPerIteration',10,'showStats',true);
