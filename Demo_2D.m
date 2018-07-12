%Run this demo to load and process 2D OCT Images

%% Iputs
%Ganymede
%filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede2D_BScanAvg/';
%OCTSystem = 'Ganymede';
%dispersionParameterA = 0.0058; %Use Demo_DispersionCorrection to find the term

%Wasatch
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Wasatch2D_BScanAvg/';
OCTSystem = 'Wasatch';
dispersionParameterA = 1.22e7; %Use Demo_DispersionCorrection to find the term

%% Process
tic;

%Load Intef From file
[interf,dimensions] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem);

%Generate BScans
scanCpx = yOCTInterfToScanCpx(interf,dimensions ...
    ,'dispersionParameterA', dispersionParameterA ...
    );

toc;
%% Visualization
subplot(2,1,1);
imagesc(log(mean(abs(scanCpx),3)));
title('B Scan');
colormap gray
subplot(2,1,2);
plot(dimensions.lambda.values,interf(:,round(end/2),round(end/2)));
title('Example interferogram');
grid on;
xlabel(['Wavelength [' dimensions.lambda.units ']']);