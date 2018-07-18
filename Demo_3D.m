%Run this demo to load and process 3D OCT Images in a batch format due to
%memory limitations

%% Iputs
%Wasatch
filePath = ['\\171.65.17.174\MATLAB_Share\Jenkins\myOCT Build\TestVectors\' ...
    'Wasatch2D_BScanAvg\'];
dispersionParameterA = 100; %Use Demo_DispersionCorrection to find the term
OCTSystem = 'Wasatch';

%Dispersion Parameter A [nm^2/rad], Quadratic Term
dispersionParameterA = 100;

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