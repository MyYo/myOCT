%This script corrects Thorlabs` OCT Apodization (lambda & intensity),
% based on measurments done using two long-pass filters at each
% interferogram edge points.
% See full report at: https://github.com/MyYo/myOCT/tree/master/Reports
%Author: Ziv Lautman (August 31, 2019)

%% Inputs

filePath850 = ['/Users/Stanford/Desktop/Calibration08-08-2019/850/850_0001_Mode2D'];
filePath950 = ['/Users/Stanford/Desktop/Calibration08-08-2019/950/950_0001_Mode2D'];
filePathNoFilter = ['/Users/Stanford/Desktop/Calibration08-08-2019/NoFilter/NoFilter_0001_Mode2D'];

%% Load interf, dimensions & equispacing

[interf850,dimensions850] = yOCTLoadInterfFromFile(filePath850,'ApodizationCorrection','None');
[interf850,dimensions850] = yOCTEquispaceInterf(interf850,dimensions850);

[interf950,dimensions950] = yOCTLoadInterfFromFile(filePath950,'ApodizationCorrection','None');
[interf950,dimensions950] = yOCTEquispaceInterf(interf950,dimensions950);

[interfNoFilter,dimensionsNF] = yOCTLoadInterfFromFile(filePathNoFilter,'ApodizationCorrection','None');
[interfNoFilter,dimensionsNF] = yOCTEquispaceInterf(interfNoFilter,dimensionsNF);

%% Process Interf

lambda850=dimensions850.lambda.values;
lambda950=dimensions950.lambda.values;
lambdaNF=dimensionsNF.lambda.values;

Intensity850=mean(interf850,3); Intensity850=mean(Intensity850(:,round(end/2)),2); 
Intensity950=mean(interf950,3); Intensity950=mean(Intensity950(:,round(end/2)),2); 
IntensityNoFilter=mean(interfNoFilter,3); IntensityNoFilter=mean(IntensityNoFilter(:,round(end/2)),2);

%% Read Thorlabs Transmission data
Thorlabs850 = readtable('/Users/Stanford/Desktop/Calibration08-08-2019/Thorlabs850/FELH0850_Transmission.xlsx');
%find where lambda 1 & End are
Thorlabs850Index1 = find(table2array(Thorlabs850(2:end,3))>lambda850(1),1);
Thorlabs850IndexEnd = find(table2array(Thorlabs850(2:end,3))>lambda850(end),1);
Thorlabs850lambda1toEnd = table2array(Thorlabs850(Thorlabs850Index1:Thorlabs850IndexEnd,3));
Thorlabs850Intensity1toEnd = table2array(Thorlabs850(Thorlabs850Index1:Thorlabs850IndexEnd,4))./100;

Thorlabs950 = readtable('/Users/Stanford/Desktop/Calibration08-08-2019/Thorlabs950/FELH0950_Transmission.xlsx');
%find where lambda 1 & End are
Thorlabs950Index1 = find(table2array(Thorlabs950(2:end,3))>lambda950(1),1);
Thorlabs950IndexEnd = find(table2array(Thorlabs950(2:end,3))>lambda950(end),1);
Thorlabs950lambda1toEnd = table2array(Thorlabs950(Thorlabs950Index1:Thorlabs950IndexEnd,3));
Thorlabs950Intensity1toEnd = table2array(Thorlabs950(Thorlabs950Index1:Thorlabs950IndexEnd,4))./100;

%% Noise Removal in Intensity & Thorlabs Data

Intensity850Indexdiff=find(diff(Intensity850)>4,1);
Intensity850(1:Intensity850Indexdiff)=0;

Intensity950Indexdiff=find(diff(Intensity950)>4,1);
Intensity950(1:Intensity950Indexdiff)=0;

Thorlabs850IntensityIndex=find(diff(Thorlabs850Intensity1toEnd)>4,1);
Thorlabs850Intensity1toEnd(1:Thorlabs850IntensityIndex)=0;

Thorlabs950IntensityIndex=find(diff(Thorlabs950Intensity1toEnd)>4,1);
Thorlabs950Intensity1toEnd(1:Thorlabs950IntensityIndex)=0;

%% Process Transmission

Transmission850 = sqrt(Intensity850./IntensityNoFilter);
Transmission950 = sqrt(Intensity950./IntensityNoFilter);

%% Interf Visualization
figure1=figure (1);
subplot(3,1,1)
plot(lambda850,Intensity850);
title('Ganymede Apodization - 850nm Filter');
grid on;
ylim([0 2000]);
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
subplot(3,1,2)
plot(lambda950,Intensity950);
title('Ganymede Apodization - 950nm Filter');
grid on;
ylim([0 2000]);
xlabel(['Wavelength - ' dimensions950.lambda.units '']);
subplot(3,1,3)
plot(lambdaNF,IntensityNoFilter);
title('Ganymede Apodization');
grid on;
ylim([0 2000]);
xlabel(['Wavelength - ' dimensionsNF.lambda.units '']);
saveas(figure1,'GanymedeCompare.png')

%% Transmission Visualization
figure2=figure (2);
subplot(2,1,1)
plot(lambda850,Transmission850);
title('Transmission - 850nm Filter');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
subplot(2,1,2)
plot(lambda950,Transmission950);
title('Transmission - 950nm Filter');
grid on;
xlabel(['Wavelength - ' dimensions950.lambda.units '']);
saveas(figure2,'TransmissionCompare.png')

%% Transmission & Thorlabs Visualization
figure3=figure (3);
subplot(2,1,1)
plot(lambda850,Transmission850,Thorlabs850lambda1toEnd,Thorlabs850Intensity1toEnd);
title('Transmission - 850nm Compare');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
subplot(2,1,2)
plot(lambda950,Transmission950,Thorlabs950lambda1toEnd,Thorlabs950Intensity1toEnd);
title('Transmission - 950nm Compare');
grid on;
xlabel(['Wavelength - ' dimensions950.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
saveas(figure3,'TransmissionWithThorlabsCompare.png')

%% Interpolating Thorlabs data

Thorlabs850lambda1toEndInterp=transpose(interp1(Thorlabs850lambda1toEnd,Thorlabs850Intensity1toEnd,lambda850));
Thorlabs850lambda1toEndInterp(end)=Thorlabs850lambda1toEndInterp(end-1);
Thorlabs950lambda1toEndInterp=transpose(interp1(Thorlabs950lambda1toEnd,Thorlabs950Intensity1toEnd,lambda950));
Thorlabs950lambda1toEndInterp(end)=Thorlabs950lambda1toEndInterp(end-1);

figure4=figure (4);
subplot(2,1,1)
plot(lambda850,Transmission850,lambda850,Thorlabs850lambda1toEndInterp);
title('Transmission - 850nm Compare - Interp');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
subplot(2,1,2)
plot(lambda950,Transmission950,lambda950,Thorlabs950lambda1toEndInterp);
title('Transmission - 950nm Compare - Interp');
grid on;
xlabel(['Wavelength - ' dimensions950.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
saveas(figure4,'TransmissionWithThorlabsCompareInterp.png')

%% Correcting Ganymede - fminsearch
FirstGuess850 = [23, 1.2, 0.95];
FirstGuess950 = [23, 1.2, 0.95];

ShiftCorrect850 = @(x) sum(abs((abs(x(2))*(replaceNaN(transpose(interp1(lambda850,Transmission850,x(3)*(lambda850+x(1)))))) - Thorlabs850lambda1toEndInterp)));
MinGuess850 = fminsearch(@(c) ShiftCorrect850(c),FirstGuess850);

ShiftCorrect950 = @(x) sum(abs((abs(x(2))*(replaceNaN(transpose(interp1(lambda950,Transmission950,x(3)*(lambda950+x(1)))))) - Thorlabs950lambda1toEndInterp)));
MinGuess950 = fminsearch(@(c) ShiftCorrect950(c),FirstGuess950);

MinGuessAvgTranslation = mean([MinGuess950(1) MinGuess850(1)]);
MinGuessAvgScaling = mean([MinGuess950(2) MinGuess850(2)]);
MinGuessAvgTranslationFactor = mean([MinGuess950(3) MinGuess850(3)]);


figure5=figure (5);
subplot(2,1,1)
plot(MinGuessAvgTranslationFactor*(lambda850 - MinGuessAvgTranslation) ,Transmission850 * MinGuessAvgScaling,lambda850,Thorlabs850lambda1toEndInterp);
title('Transmission - 850nm - Optimization (fmin)');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
subplot(2,1,2)
plot(MinGuessAvgTranslationFactor*(lambda950 - MinGuessAvgTranslation) ,Transmission950 * MinGuessAvgScaling,lambda950,Thorlabs950lambda1toEndInterp);
title('Transmission - 950nm - Optimization (fmin)');
grid on;
xlabel(['Wavelength - ' dimensions950.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
saveas(figure5,'TransmissionWithThorlabsCompareOptimization_fmin.png')

%% Correcting Ganymede - Globalminsearch
FirstGuess = [23 1.2 0.95];

rng default
ms = MultiStart;
ShiftCorrect850g = @(x) sum(abs((abs(x(2))*(replaceNaN(transpose(interp1(lambda850,Transmission850,x(3)*(lambda850+x(1)))))) - Thorlabs850lambda1toEndInterp)));
problem = createOptimProblem('fmincon','x0',FirstGuess,...
    'objective',ShiftCorrect850g,'lb',[0,0.1,0.5],'ub',[50,2,1.5]);
[xmin3,fmin3,flag3,outpt3,allmins3] = run(ms,problem,30);

figure12=figure(12);
plot(arrayfun(@(x)x.Fval,allmins3),'k*')
xlabel('Solution number')
ylabel('Function value')
title('850 - Globalminsearch  - Solution Function Values')
saveas(figure12,'850 - Globalminsearch  - Solution Function Values.png')

figure13=figure(13);
plot(arrayfun(@(x)x(:).X(1),allmins3),arrayfun(@(x)x(:).X(2),allmins3),'k*')
hold on
plot(xmin3(1),xmin3(2),'r*');
xlabel('Translation value')
ylabel('Scaling value')
title('850 - Globalminsearch  - Solutions Value')
saveas(figure13,'850 - Globalminsearch  - Solutions Value.png')

rng default
ms = MultiStart;
ShiftCorrect950g = @(x) sum(abs((abs(x(2))*(replaceNaN(transpose(interp1(lambda950,Transmission950,x(3)*(lambda950+x(1)))))) - Thorlabs950lambda1toEndInterp)));
problem = createOptimProblem('fmincon','x0',FirstGuess,...
    'objective',ShiftCorrect950g,'lb',[0,0.1,0.5],'ub',[50,2,1.5]);
[xmin4,fmin4,flag4,outpt4,allmins4] = run(ms,problem,30);

figure14=figure(14);
plot(arrayfun(@(x)x.Fval,allmins4),'k*')
xlabel('Solution number')
ylabel('Function value')
title('950 - Globalminsearch  - Solution Function Values')
saveas(figure14,'950 - Globalminsearch  - Solution Function Values.png')

figure15=figure(15);
plot(arrayfun(@(x)x(:).X(1),allmins4),arrayfun(@(x)x(:).X(2),allmins4),'k*')
hold on
plot(xmin4(1),xmin4(2),'r*');
xlabel('Translation value')
ylabel('Scaling value')
title('950 - Globalminsearch  - Solutions Value')
saveas(figure15,'950 - Globalminsearch  - Solutions Value.png')

xminNew(1) = mean([xmin3(1) xmin4(1)]);
xminNew(2) = mean([xmin3(2) xmin4(2)]);
xminNew(3) = mean([xmin3(3) xmin4(3)]);

figure16=figure (16);
subplot(2,1,1)
plot(lambda850 - xminNew(1) ,Transmission850 * xminNew(2),lambda850,Thorlabs850lambda1toEndInterp);
title('Transmission - 850nm - Optimization (Globalminsearch )');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
subplot(2,1,2)
plot(lambda950 - xminNew(1) ,Transmission950 * xminNew(2),lambda950,Thorlabs950lambda1toEndInterp);
title('Transmission - 950nm - Optimization (Globalminsearch )');
grid on;
xlabel(['Wavelength - ' dimensions950.lambda.units '']);
legend('GanymedeTransmission','Thorlabs','Location','northeastoutside')
saveas(figure16,'TransmissionWithThorlabsCompareOptimizationGlobalminInterp.png')

%% print new Apodization

figure6=figure (6);
plot(MinGuessAvgTranslationFactor*(lambdaNF - MinGuessAvgTranslation) ,IntensityNoFilter * MinGuessAvgScaling)
title('Corrected Apodization - fmin');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
saveas(figure6,'CorrectedApodizationfmin.png')

figure7=figure (7);
plot(MinGuessAvgTranslationFactor*(lambdaNF - MinGuessAvgTranslation) ,(IntensityNoFilter * MinGuessAvgScaling)./max(IntensityNoFilter))
hold on
plot(lambdaNF,IntensityNoFilter./max(IntensityNoFilter));
hold on
plot(lambda850,Thorlabs850lambda1toEndInterp,lambda950,Thorlabs950lambda1toEndInterp);
title('Corrected Apodization Compared - fmin');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
legend('CorrectedApodization','OriginalApodization','Thorlabs850','Thorlabs850','Location','northeastoutside')
saveas(figure7,'CorrectedApodizationComparedfmin.png')

figure17=figure (17);
plot(xminNew(3)*(lambdaNF - xminNew(1)) ,IntensityNoFilter * xminNew(2))
title('Corrected Apodization - Global');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
saveas(figure17,'CorrectedApodizationGlobal.png')

figure18=figure (18);
plot(xminNew(3)*(lambdaNF - xminNew(1)) ,(IntensityNoFilter * xminNew(2))./max(IntensityNoFilter))
hold on
plot(lambdaNF,IntensityNoFilter./max(IntensityNoFilter));
hold on
plot(lambda850,Thorlabs850lambda1toEndInterp,lambda950,Thorlabs950lambda1toEndInterp);
title('Corrected Apodization Compared - Global');
grid on;
xlabel(['Wavelength - ' dimensions850.lambda.units '']);
legend('CorrectedApodization','OriginalApodization','Thorlabs850','Thorlabs850','Location','northeastoutside')
saveas(figure18,'CorrectedApodizationComparedGlobal.png')

%% print results

fprintf('FminSearch Results are: \n')
fprintf('Translation: %f %s \n',MinGuessAvgTranslation,dimensions850.lambda.units)
fprintf('Translation Factor: %f %s \n',MinGuessAvgTranslationFactor,dimensions850.lambda.units)
fprintf('Scaling factor: %f  \n',MinGuessAvgScaling)

fprintf('GlobalMinSearch Results are: \n')
fprintf('Translation: %f %s \n',xminNew(1),dimensions850.lambda.units)
fprintf('Translation Factor: %f %s \n',xminNew(3),dimensions850.lambda.units)
fprintf('Scaling factor: %f  \n',xminNew(2))

fprintf('New Lambda values, based on GlobalMinSearch are: \n')
fprintf('Starting wavelength: %f \n',xminNew(3)*(lambdaNF(1) - xminNew(1)))
fprintf('Ending wavelength: %f \n',xminNew(3)*(lambdaNF(end) - xminNew(1)))

fprintf('New Lambda values, based on fminsearch are: \n')
fprintf('Starting wavelength: %f \n',MinGuessAvgTranslationFactor*(lambdaNF(1) - MinGuessAvgTranslation))
fprintf('Ending wavelength: %f \n',MinGuessAvgTranslationFactor*(lambdaNF(end) - MinGuessAvgTranslation))


function [input] = replaceNaN(input)
    index1 = find(isnan(input),1,'first');
    indexEnd = find(~isnan(input),1,'last');
    if index1 == 1
        input(index1:indexEnd-1)=input(indexEnd);
    else
        input(index1:end)=input(indexEnd);
    end
end
