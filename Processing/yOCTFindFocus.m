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
% 'zDepthStitchingMode'      false      See description above.
% 'manualRefinment'          false      See description above.
% 'tissueRefractiveIndex'    []         Tissue index of refraction. Set to
%                                       [] to use the default json value.
% 'outputDebugImages'		 true		Would you like to see debug images?
% 'verbose'                  false      More information.
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

p = inputParser;
addRequired(p,'inputDataFolder');

addParameter(p,'zDepthStitchingMode',false)
addParameter(p,'manualRefinment',false)
addParameter(p,'outputDebugImages',false)
addParameter(p,'verbose',false);
addParameter(p,'tissueRefractiveIndex',[]); 

parse(p,varargin{:});
in = p.Results;

inputDataFolder = in.inputDataFolder;
n = in.tissueRefractiveIndex; 
scanInfoJson = awsReadJSON([inputDataFolder 'ScanInfo.json']);
dispersionQuadraticTerm = scanInfoJson.DefaultDispersionQuadraticTerm;
optPathCorrection = scanInfoJson.OpticalPathCorrectionPolynomial;
if isempty(n)
    n = scanInfoJson.tissueRefractiveIndex;
end

if ~zDepthStitchingMode
    error('zDepthStitchingMode=false is not implemented yet');
end

%% If zDepthStitchingMode is set
if zDepthStitchingMode
    % Count the number of Data folders within the inputDataFolder
    numDataFiles = length(scanInfoJson.scanOrder);

    % Create zFocusPix array to store the focus of each volume
    zFocusPix = zeros(1, numDataFiles);

    % Iterate through all data folders in inputDataFolder and reconstruct each 
    % volume which was acquired at a different depth. Optical Path Correction?
    for ind = 1:numDataFiles
        filePath = sprintf('%s/Data%02d',inputDataFolder,ind);
        
        % Process        
        yToLoad = dim.y.index(round(linspace(1,length(dim.y.index),5)));
        
        [meanAbs,dimensions] = yOCTProcessScan(filePath, ...
            {@yOCTFindFocus_FlatenData}, ... 
            'dispersionQuadraticTerm', dispersionQuadraticTerm, ...
            'n',n, ...
            'YFramesToProcess',yToLoad, ...
            'showStats',in.verbose, ...
            'applyPathLengthCorrection', true);

        % Use the reconstructed volume to find the focus and store it in zFocusPix
        [max_val, focus] = max(mean(squeeze(log(meanAbs)), [2 3]));
        zFocusPix(focusInd) = focus;
        focusInd = focusInd + 1;
        
        
        if this volume is for refinement (5)
            [meanAbs,dimensions] = yOCTProcessScan(filePath, ...
                {'meanAbs'}, ... 
                'dispersionQuadraticTerm', dispersionQuadraticTerm, ...
                'n',n, ...
                'YFramesToProcess',yToLoad(end/2)+[-1:01], ...
                'showStats',in.verbose, ...
                'applyPathLengthCorrection', true);
            
            figure(1)
            imagesc(meanAbs);
            % plot where the automated focus was selected
            save corrected focus to zFocusPix_manual
        end
    end
    
    %% Correct all focus positions according to manual 
        
%% Else if zDepthStitchingMode is NOT set repeat above process for just the specified OCT file
else   %<--- delete for now
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

