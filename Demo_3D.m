%Run this demo to load and process 2D OCT Images

%% Iputs
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede3D_BScanAvg/';
OCTSystem = 'Ganymede';

%% Process
tic;

%Load Intef From file
[interf,dimensions] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem ...
    ,'BScanAvgFramesToProcess', 1:2 ... To save time, load first few BScans. Comment out this line to load all BScans
    ,'YFramesToProcess',1:10:500 ... To save time, don't load all frames along y direction. Comment out this line to load all frames
    );

%Generate BScans
scanCpx = yOCTInterfToScanCpx(interf,dimensions ...
    ,'dispersionParameterA', 0.0058 ...Use this dispersion Parameter for air-water interface
    );

%Average along BScanAvg dimension
scanAbs = mean(abs(scanCpx),dimensions.BScanAvg.order);

toc;
%% Visualization
%Preform max projection along z
scanAbsMxProj = squeeze(max(scanAbs,[],dimensions.lambda.order));
imagesc(dimensions.x.values, dimensions.y.values, scanAbsMxProj');
xlabel(['x direction [' dimensions.x.units ']']);
ylabel(['y direction [' dimensions.y.units ']']);
title('Max Projection');
