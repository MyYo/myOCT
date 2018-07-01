%Run this demo to load and process 3D OCT Images in a batch format due to
%memory limitations

%% Iputs

%Ganymede
%filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede3D_BScanAvg/';
%OCTSystem = 'Ganymede';
%dispersionParameterA = 0.0058; %Use this dispersion Parameter for air-water interface

%Wasatch
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Wasatch3D/';
OCTSystem = 'Wasatch';
dispersionParameterA = 2.271e-02; %Use this dispersion Parameter for air-water interface

yFramesPerBatch = 10; %How many Y frames to load in a single batch, 
                      %optimzie this parameter to save computational time

%% Gather Header Data
dimensions = yOCTLoadInterfFromFile(filePath, 'OCTSystem', OCTSystem, 'PeakOnly', true);
ys = dimensions.y.index;

yStart = ys(1:yFramesPerBatch:(end-1));
yEnd   = ys([yFramesPerBatch:yFramesPerBatch:end end]);

%Stucture containing the output of the process (z,x,y)
outputMat = zeros(...
    length(dimensions.lambda.values)/2,length(dimensions.x.values),length(dimensions.y.values), ...
    'single');

%% Process
tic;
for yi=1:length(yStart) %Loop for each batch
    y = yStart(yi):yEnd(yi);
    
    %% Load and Preprocess data
    
    %Load Intef From file
    [interf,dimensionsItr] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem ...
        ,'YFramesToProcess',y ... To save time, don't load all frames along y direction. Comment out this line to load all frames
    );

    %Generate BScans
    scanCpx = yOCTInterfToScanCpx(interf,dimensionsItr ...
        ,'dispersionParameterA', dispersionParameterA ...
        );
    
    %% Add processing code here, process your frame as you would like

    %Example:
    %Average along BScanAvg dimension
    if isfield(dimensionsItr,'BSCanAvg')
        scanAbs = mean(abs(scanCpx),dimensionsItr.BScanAvg.order);
    else
        scanAbs = abs(scanCpx);
    end
    
    %% Save processed result to main matrix
    outputMat(:,:,y) = scanAbs;
end
toc;

%% Visualization - example
%Preform max projection along z
figure(1);
scanAbsMxProj = squeeze(max(outputMat,[],dimensions.lambda.order));
imagesc(dimensions.x.values, dimensions.y.values, scanAbsMxProj');
xlabel(['x direction [' dimensions.x.units ']']);
ylabel(['y direction [' dimensions.y.units ']']);
title('Max Projection');

%Present one of the B-Scans
figure(2);
imagesc(log(squeeze(scanAbs(:,:,round(end/2)))));
title('Middle B-Scan');
xlabel('x direction');
ylabel('z direction');