%Run this demo to load and process 2D OCT Images

%% Iputs
%Wasatch
filePath = ['\\171.65.17.174\MATLAB_Share\Jenkins\myOCT Build\TestVectors\' ...
    'Wasatch_2D_BScanAvg\'];
OCTSystem = 'Wasatch';
dispersionParameterA = 100; %Use Demo_DispersionCorrection to find the term

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