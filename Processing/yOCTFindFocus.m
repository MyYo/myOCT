function zFocusPix = yOCTFindFocus(varargin)
% This function find focus z position. It works well if the scan was done 
% using high magnification lens (>10x). The way the algorithm works is by
% finding the brightest z depth in the image and associating that to the
% focus. You can configure this function to allow for manual refinment or
% be completly automatic.
% "zDepthStitchingMode" is used when inputDataFolder is a TiledScan where
% each volume is acquired at different depth
%USAGE:
%    zFocusPix = yOCTLoadScan(inputDataFolder, [,parameter1,...]);
%INPUTS:
%   - inputDataFolder - OCT file / folder in local computer or at s3://
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                  Default    Information & Values
% 'zDepthStitchingMode'      False      See description above.
% 'manualRefinment'          False      See description above.
%OUTPUTS:
%   - zFocusPix - zDepth (in pixels) of the focus position in the scan. In
%                 case of tild scan, one zFocusPix will be given for each
%                 scan.

% Questions
% 1. Do we need to factor in optical path correction here? Yes
% 2. How does this function relate to the other findFocusInBScan function?
%    Does the manual refinement relate to the user input section of the
%    findFocusInBScan function? Where will the focus correction fit in?
% 3. How should we structure the json focusPositionInImageZpix param? 
% 4. What is the manualRefinement parameter for?
% 5. What is the path for the input folder?

%% Parse Inputs from varargin and load certain fields from json

scanInfoJson = awsReadJSON([inputDataFolder 'ScanInfo.json']);
dispersionQuadraticTerm = scanInfoJson.DefaultDispersionQuadraticTerm;
optPathCorrection = scanInfoJson.OpticalPathCorrectionPolynomial;
yFramesPerBatch = 1; %How many Y frames to load in a single batch, optimzie this parameter to save computational time


%% If zDepthStitchingMode is set
if zDepthStitchingMode
    % Count the number of Data folders within the inputDataFolder
    file = dir(intputDataFolder);
    filenames = {file.name};
    numDataFiles = sum(~cellfun(@isempty, strfind(filenames, 'Data')));

    % Create zFocusPix array to store the focus of each volume
    zFocusPix = zeros(1, numDataFiles);

    % Iterate through all data folders in inputDataFolder and reconstruct each 
    % volume which was acquired at a different depth. Optical Path Correction?
    for ind = 1:numDataFiles
        if ind < 10
            filePath = [inputDataFolder, '/Data0' int2str(ind) '/'];
        else
            filePath = [inputDataFolder, '/Data' int2str(ind) '/'];
        end
        
        % Process
        [meanAbs,speckleVariance,dimensions] = yOCTProcessScan(filePath, ...
            {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
            'dispersionQuadraticTerm', dispersionQuadraticTerm, ...
            'nYPerIteration', yFramesPerBatch, ...
            'showStats',true);
        
        % Optical Path Correction
        [meanAbsPathCorrected, meanAbsValidDataMap] = yOCTOpticalPathCorrection(meanAbs, dimensions, optPathCorrection);


        % Use the reconstructed volume to find the focus and store it in zFocusPix
        [max_val, focus] = max(mean(squeeze(log(meanAbsPathCorrected)), [2 3]));
        zFocusPix(focusInd) = focus;
        focusInd = focusInd + 1;
    end
%% Else if zDepthStitchingMode is NOT set repeat above process for just the specified OCT file
else   
    % Reconstruct the chosen volume. Optical Path Correction?
    [meanAbs,speckleVariance,dimensions] = yOCTProcessScan(inputDataFolder, ...
            {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
            'dispersionQuadraticTerm', dispersionQuadraticTerm, ...
            'nYPerIteration', yFramesPerBatch, ...
            'showStats',true);
        
    % Optical Path Correction
    scanInfoJson = awsReadJSON([inputDataFolder 'ScanInfo.json']);
    [meanAbsPathCorrected, meanAbsValidDataMap] = yOCTOpticalPathCorrection(meanAbs, dimensions, optPathCorrection);

    % Use the reconstructed volume to find the focus and store it in zFocusPix
    [max_val, zFocusPix] = max(mean(squeeze(log(meanAbsPathCorrected)), [2 3]));
end

