%This demo demonstrates how to extract images from 2 bands (spectral OCT)
%this demo is for 2D but can be extanded for 3D

%% Iputs
%Ganymede
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede2D_BScanAvg/';
OCTSystem = 'Ganymede';
band1 = [800 900]; %[nm]
band2 = [900 1000]; %[nm]
dispersionParameterA = 0.0058; %Use Demo_DispersionCorrection to find the term

%% Process
tic;

%Load Intef From file
[interf,dimensions] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem ...
    ,'BScanAvgFramesToProcess', 1 ... To save time, load first few BScans. Comment out this line to load all BScans
    );

%Equispace interferogram to save computation time while re-processing
[interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions);

%Generate BScans - Band1
scanCpx1 = yOCTInterfToScanCpx(interf,dimensions ...
    ,'dispersionParameterA', dispersionParameterA ...Use this dispersion Parameter for air-water interface
    ,'band',band1);

%Generate BScans - Band2
scanCpx2 = yOCTInterfToScanCpx(interf,dimensions ...
    ,'dispersionParameterA', dispersionParameterA ...Use this dispersion Parameter for air-water interface
    ,'band',band2);

toc;

%% Visualization
subplot(2,1,1);
imagesc(log(mean(abs(scanCpx1),3)));
title('Band1');
colormap gray
subplot(2,1,2);
imagesc(log(mean(abs(scanCpx2),3)));
title('Band2');
colormap gray
