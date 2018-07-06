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
filePath = '\\171.65.17.174\MATLAB_Share\Itamar\2018_06_13_14-59-16\';
OCTSystem = 'Wasatch';

%% What values of dispersion parameter a should we try
a = 10.^(linspace(-8,2,30));
a = [-a a];
a = sort(a);

%% Pre-Process
%Load Intef From file
[interf,dimensions] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem ...
    ,'BScanAvgFramesToProcess', 1 ... To compute dispersion correction only 1 frame is required
    ,'YFramesToProcess',1         ... To compute dispersion correction only 1 frame is required
    );

%Equispace interferogram to save computation time while re-processing
[interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions);

%% Plot ROI
%Compute B-Scan
scanCpxe = yOCTInterfToScanCpx( interfe ,dimensionse ...
    ,'dispersionParameterA', a(round(end/2)) );
scanCpxAbs = mean(mean(abs(scanCpxe),3),4);

figure(1);

%Ask user to mark an interface
imagesc(log(scanCpxAbs));
colormap gray;
title(sprintf('Please Mark What Area a Air-Tissue Interface Should Be'));
rct = getrect;
ROIx = round(rct(1)+rct(3)/2+(-5:5));
ROIz = round(rct(2)+(0:rct(4)));

subplot(2,1,1);
imagesc(log(scanCpxAbs));
hold on;
plot(ROIx([1 end end 1 1]),ROIz([1 1 end end 1]));
hold off;
legend('ROI');
title(sprintf('Before Dispersion, A=%.4e',a(round(end/2))));

%% Loop Over Dispersion Parameters
figure(2);
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
        myplot(iteration,ROIz,a(i),scanCpxAbs,val)
        if (iteration == 1)
            pause(0.5);
        else
            pause(0.01);
        end
    end
    
    %Using best itration, expand
    if (iBest>=2 && iBest <=(length(a)-1))
        a = linspace(a(iBest-1),a(iBest+1),length(a));
    else
        %Edge case
        d = diff(a);
        d = d(min([iBest length(a)]));
        a = linspace(a(iBest)-d,a(iBest)+d,length(a));
    end
    
    %Plot
    myplot(iteration,ROIz,aBest,scanCpxAbsBest,valBest)

end
fprintf('Best A Parameter: %.3e\n',aBest);

%% Plot an "After" Image
%Compute B-Scan
scanCpxe = yOCTInterfToScanCpx( interfe ,dimensionse ...
    ,'dispersionParameterA', aBest );
scanCpxAbs = mean(mean(abs(scanCpxe),3),4);

figure(1);
subplot(2,1,2);
imagesc(log(scanCpxAbs));
hold on;
plot(ROIx([1 end end 1 1]),ROIz([1 1 end end 1]));
hold off;
legend('ROI');
title(sprintf('After Dispersion, A=%.4e',aBest));

function myplot(iteration,ROIz,a,scanCpxAbs,val)
subplot(2,2,iteration);
z = 1:length(scanCpxAbs);
plot(z,scanCpxAbs,z(ROIz),scanCpxAbs(ROIz));
legend('Average A-Scan','ROI');
%title(sprintf('Dispersion Parameter A=%.3e\niteration=%d (%.3e to %.3e)',a(i),iteration,min(a),max(a)));
title(sprintf('Dispersion Parameter A=%.3e\niteration=%d, Maximization Val: %.1f',a,iteration,val));
grid on;
xlim(z([1 end]));