%This function tests code package for bit exact competability

close all;

%% Inputs

testI = 1;
switch(testI)
    case 1
        %% Preform Test - 2D BScan Average
        filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede2D_BScanAvg/';
        tic;
        [interf2,dimensions2] = yOCTLoadInterfFromFile(filePath,'OCTSystem','Ganymede','BScanAvgFramesToProcess',1:5);
        runtime2 = toc;

        scanCpx = yOCTInterfToScanCpx(interf2,dimensions2);
        imagesc(log(mean(abs(scanCpx),3)));
        title(sprintf('2D BScan Average Image, time: %.0f [sec]',runtime2));
    case 2
        %% Preform Test - 3D BScan Average
        filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede3D_BScanAvg/';

        tic;
        [interf3,dimensions3] = yOCTLoadInterfFromFile(filePath,'OCTSystem','Ganymede','BScanAvgFramesToProcess',1,'YFramesToProcess',1:10:500);
        runtime3 = toc;

        scanCpx = yOCTInterfToScanCpx(interf3,dimensions3);

        %Create an unface image
        uf = squeeze(mean(abs(scanCpx(170+(-10:10),:,:)),1));
        imagesc (log(uf));
        title(sprintf('3D BScan Average Image, time: %.0f [sec]',runtime3));
end