function Demo_DispersionCorrectionManual
%Run this demo to find dispersionParameter, manually

close all;

%% Inputs
%Ganymede
filePath = 's3://delazerdalab2/CodePackage/TestVectors/Ganymede2D_BScanAvg/';
OCTSystem = 'Ganymede';

%Telesto
%filePath = '\\171.65.17.174\Telesto\Peng\2018\20180622\3D\IT1450_0009_ModeSpeckle';
%OCTSystem = 'Telesto';

%Wasatch
%filePath = 's3://delazerdalab2/CodePackage/TestVectors/Wasatch2D_BScanAvg/';
%filePath = '\\171.65.17.174\MATLAB_Share\Itamar\2018_06_13_14-59-16\';
%filePath = 'Y:\Work\_de la Zerda Lab Scripts\yOCTRef\Wasatch-Processing-Code-master\2d\tiffs\';
%OCTSystem = 'Wasatch';

%% Pre-Process
%Load Intef From file
[interf,dimensions] = yOCTLoadInterfFromFile(filePath,'OCTSystem',OCTSystem ...
    ,'BScanAvgFramesToProcess', 1 ... To compute dispersion correction only 1 frame is required
    ,'YFramesToProcess',1         ... To compute dispersion correction only 1 frame is required
    );

%Equispace interferogram to save computation time while re-processing
[interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions);

txt = uicontrol('Style','text',...
        'Position',[100 45 400 20],...
        'String','log10(dispersionParameterA)');
initialGuess = log10(100);
sld = uicontrol('Style', 'slider',...
        'Min',-10,'Max',10,'Value',initialGuess,...
        'Position', [100 20 400 20],...
        'SliderStep',[0.0005 0.1],... %[minor step, major step]
        'Callback', @(source,event)(SliderCallback(source,event,interfe,dimensionse))); 
    
%Present first image
SliderCallback(initialGuess,[],interfe,dimensionse)
end
    
function SliderCallback(source,event,interfe,dimensionse)
if ~isnumeric(source)
    val = source.Value;
else
    val = source;
end

dispersionParameterA = sign(val)*10^(abs(val));
scanCpxe = yOCTInterfToScanCpx( interfe ,dimensionse ...
    ,'dispersionParameterA', dispersionParameterA );
lg = log(abs(scanCpxe));
imagesc(lg);
caxis([-5 6]);
colormap gray;
title(sprintf('dispersionParameterA=%.3e [nm^2/rad]',dispersionParameterA));
end