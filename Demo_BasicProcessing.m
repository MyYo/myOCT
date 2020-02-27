%Run this demo to load and process OCT Images and generate Abs and Speckle
%Variance Images

addpath(genpath('.')); %Add library to matlab path

%% Iputs
%Define file paths of the files to process
filePaths = {...
    '\\171.65.17.174\MATLAB_Share\Jenkins\myOCT Build\TestVectors\Wasatch_3D\' ...
    ...'\\171.65.17.174\MATLAB_Share\Jenkins\myOCT Build\TestVectors\Wasatch_3D\' ...
    };
dispersionQuadraticTerm = 100; %Use Demo_DispersionCorrection to find the term

isSaveSpeckleVariance = true; %Set to false to skip speckle variance

%% Loop over files
for i=1:length(filePaths)
    disp(['Processing ' filePaths{i} ' ...']);
    tic;
    
    %% Load & PreProcess Data
    [meanAbs,speckleVariance,dimensions] = yOCTProcessScan(filePaths{i}, ...
        {'meanAbs','speckleVariance'}, ... 
        'dispersionQuadraticTerm', dispersionQuadraticTerm, ...
        'nYPerIteration', 1, ...
        'showStats',true);
    
    %Build file prefix
    filePrefix = fileparts(filePaths{i});
    filePrefix = filePrefix((find(filePrefix == '\',1,'last')+1):end);
    
    %% Process and Save
    %Save ScanAbsLog
    yOCT2Tif(log(meanAbs),[filePrefix '_ScanAbs.tif']);
    
    %Compute and Save Speckle Variance
    if (isSaveSpeckleVariance)
        sv = speckleVariance./meanAbs;
        sv = sv.*(meanAbs > 0.8); %Gate speckle variance, threshold should be based on air signal
        
        yOCT2Tif(sv,[filePrefix '_SpeckleVariance.tif'],...
            'clim',[0 1]); %Image grayscale border
    end
    
    toc;
end

disp('Done');