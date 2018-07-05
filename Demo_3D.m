%Run this demo to load and process 3D OCT Images in a batch format due to
%memory limitations

%% Iputs
%Wasatch
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Wasatch3D/';
OCTSystem = 'Wasatch';

%Dispersion Parameter A
%dispersionParameterA = 0.0058; Ganymede
%dispersionParameterA = -7.814e-04; %Telesto
dispersionParameterA = 2.271e-02; %Wasatch

yFramesPerBatch = 1; %How many Y frames to load in a single batch, optimzie this parameter to save computational time

%% Process
tic;
[meanAbs,speckleVariance,dimensions] = yOCTProcessScan(filePath, ...
    {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
    'OCTSystem', OCTSystem, ...
    'dispersionParameterA', dispersionParameterA, ...
    'nYPerIteration', yFramesPerBatch, ...
    'showStats',true);
toc;

%% Save To File
tic;
yOCT2Tif(log(meanAbs),'tmp.tif');
meanAbs2 = yOCTFromTif('tmp.tif');
toc;

%% Visualization - example
%Preform max projection along z
figure(1);
scanAbsMxProj = squeeze(max(meanAbs,[],dimensions.lambda.order));
imagesc(dimensions.x.values, dimensions.y.values, scanAbsMxProj');
xlabel(['x direction [' dimensions.x.units ']']);
ylabel(['y direction [' dimensions.y.units ']']);
title('Max Projection');

%Present one of the B-Scans
figure(2);
imagesc(log(squeeze(meanAbs(:,:,round(end/2)))));
title('Middle B-Scan');
xlabel('x direction');
ylabel('z direction');