function Demo_DispersionCorrection
%Run this demo to find dispersionParameter. It will also demonstrate how to
%interpolate interferogram data for faster excecution of yOCTInterfToScanCpx

close all;

%% Inputs
%Ganymede
%filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede2D_BScanAvg/';
%OCTSystem = 'Ganymede';

%Wasatch
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Wasatch2D_BScanAvg/';
OCTSystem = 'Wasatch';
ROIx = 300 + (-5:5); ROIz = 50:150; %Where should we look for a sharp edge?

%% Pre-Process
%Load Intef From file
[interf,dimensions] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem ...
    ,'BScanAvgFramesToProcess', 1 ... To compute dispersion correction only 1 frame is required
    ,'YFramesToProcess',1         ... To compute dispersion correction only 1 frame is required
    );

%Equispace interferogram to save computation time while re-processing
[interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions);

%% Loop Over Dispersion Parameters
a = 10.^(linspace(-8,2,20));
for iteration = 1:4
    aBest = [];
    iBest = [];
    scanCpxAbsBest = [];
    valBest = 0;
    
    %Loop over all a values, find the best one
    for i=1:length(a)
    
        %Compute B-Scan
        scanCpxe = yOCTInterfToScanCpx( interfe(:,ROIx),dimensionse ...
            ,'dispersionParameterA', a(i) );
        scanCpxAbs = mean(abs(scanCpxe),2);
        
        %Evaluate B-Scan
        val = max(scanCpxAbs(ROIz));
        if (val>valBest)
            aBest = a(i);
            iBest = i;
            valBest = val;
            scanCpxAbsBest = scanCpxAbs;
        end
        
        %Plot
        myplot(iteration,ROIz,a(i),scanCpxAbs)
        pause(0.01);
    end
    
    %Using best itration, expand
    a = linspace(a(iBest-1),a(iBest+1),length(a));
    
    %Plot
    myplot(iteration,ROIz,aBest,scanCpxAbsBest)

end
fprintf('Best A Parameter: %.3e\n',aBest);

function myplot(iteration,ROIz,a,scanCpxAbs)
subplot(2,2,iteration);
z = 1:length(scanCpxAbs);
plot(z,scanCpxAbs,z(ROIz),scanCpxAbs(ROIz));
legend('Average A-Scan','ROI');
%title(sprintf('Dispersion Parameter A=%.3e\niteration=%d (%.3e to %.3e)',a(i),iteration,min(a),max(a)));
title(sprintf('Dispersion Parameter A=%.3e\niteration=%d',a,iteration));
grid on;
xlim(z([1 end]));