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

% If input folder points to a specific Data folder, extract the path to the
% OCTVolumes folder in order to access the ScanConfig.json file 
if contains(inputDataFolder,'Data')
    configDataFolder = [fileparts(fileparts(inputDataFolder)) '/'];
else
    configDataFolder = inputDataFolder;
end
    
n = in.tissueRefractiveIndex; 
scanConfigJson = awsReadJSON([configDataFolder 'ScanConfig.json']);
dispersionQuadraticTerm = scanConfigJson.DefaultDispersionQuadraticTerm;

if isempty(n)
    n = scanConfigJson.tissueRefractiveIndex;
end

%% If zDepthStitchingMode is set
if in.zDepthStitchingMode
    % Count the number of Data folders within the inputDataFolder
    numDataFiles = length(scanConfigJson.scanOrder);

    % Create zFocusPix array to store the focus of each volume
    zFocusPix = zeros(1, numDataFiles);
    
    OCTVolumesFolderVolume = [inputDataFolder '/Volume/'];
    
    % Iterate through all data folders in inputDataFolder and reconstruct each 
    % volume which was acquired at a different depth. Optical Path Correction?
    for ind = 1:numDataFiles
        filePath = sprintf('%s/Data%02d',OCTVolumesFolderVolume,ind);
        
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
        zFocusPix(ind) = focus;
        
        if in.outputDebugImages && (ind == 1 || mod(ind, 10) == 0)
            
            x=-6:6;%x=-2.5:1:2.5;
            %y=x;z=0;filter = '(XY filter)';
            y=x;z=-2:2;filter = '(XZ filter)';
            rel = 2;
            [z,x,y]=ndgrid(z,x,y);
            a = exp(-(x.^2+y.^2+(z*2).^2)/rel.^2); % Z step = 2um, X and Y step = 1um
            a =a /sum(a (:));
            INTabsF = log10(convn(meanAbs,a,'same'));
            clear x y z a;

            % intensity profile
            FlagBead = 0;
            profile = zeros(1,1024);
            for i=1:1024
                if FlagBead == 1
                    profile(i)=mean(max(INTabsF(i,:,:)));
                else
                    profile(i)=mean(mean(INTabsF(i,:,:)));
                end
            end

            [psor,lsor] = findpeaks(profile,(1:1024),'SortStr','descend','MinPeakDistance',5);
            c1 = min(psor)+0.2;c2 =max(psor)+0.5;
            ax = figure();
            imagesc(squeeze(INTabsF(:,:,round(end/2))));
            colormap('gray');
            colorbar;
            caxis([c1 c2]);
            title(['Find Focus Plot Volume ' int2str(ind)]);
            
            %Output Tiff 
            plot_name = ['FindFocusInBScanVolume' int2str(ind) '.png'];
            saveas(ax, plot_name);
            if (awsIsAWSPath(OCTVolumesFolder))
                %Upload to AWS
                awsCopyFileFolder(plot_name,[LogFolder '/' plot_name]);
            else
                if ~exist(LogFolder,'dir')
                    mkdir(LogFolder)
                end
                copyfile(plot_name,[LogFolder '\' plot_name]);
            end   
        end
        
%         if this volume is for refinement (5)
%             [meanAbs,dimensions] = yOCTProcessScan(filePath, ...
%                 {'meanAbs'}, ... 
%                 'dispersionQuadraticTerm', dispersionQuadraticTerm, ...
%                 'n',n, ...
%                 'YFramesToProcess',yToLoad(end/2)+[-1:01], ...
%                 'showStats',in.verbose, ...
%                 'applyPathLengthCorrection', true);
%             
%             figure(1)
%             imagesc(meanAbs);
%             % plot where the automated focus was selected
%             save corrected focus to zFocusPix_manual
%         end
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

    % Use the reconstructed volume to find the focus and store it in zFocusPix
    [max_val, zFocusPix] = max(mean(squeeze(log(meanAbs)), [2 3]));
    
    if in.outputDebugImages
            
        x=-6:6;%x=-2.5:1:2.5;
        %y=x;z=0;filter = '(XY filter)';
        y=x;z=-2:2;filter = '(XZ filter)';
        rel = 2;
        [z,x,y]=ndgrid(z,x,y);
        a = exp(-(x.^2+y.^2+(z*2).^2)/rel.^2); % Z step = 2um, X and Y step = 1um
        a =a /sum(a (:));
        INTabsF = log10(convn(meanAbs,a,'same'));
        clear x y z a;

        % intensity profile
        FlagBead = 0;
        profile = zeros(1,1024);
        for i=1:1024
            if FlagBead == 1
                profile(i)=mean(max(INTabsF(i,:,:)));
            else
                profile(i)=mean(mean(INTabsF(i,:,:)));
            end
        end

        [psor,lsor] = findpeaks(profile,(1:1024),'SortStr','descend','MinPeakDistance',5);
        c1 = min(psor)+0.2;c2 =max(psor)+0.5;
        ax = figure();
        imagesc(squeeze(INTabsF(:,:,round(end/2))));
        colormap('gray');
        colorbar;
        caxis([c1 c2]);
        title('Find Focus Plot Volume 10x');

        %Output Tiff 
        plot_name = 'FindFocusInBScan.png';
        saveas(ax, plot_name);
        if (awsIsAWSPath(OCTVolumesFolder))
            %Upload to AWS
            awsCopyFileFolder(plot_name,[LogFolder '/' plot_name]);
        else
            if ~exist(LogFolder,'dir')
                mkdir(LogFolder)
            end
            copyfile(plot_name,[LogFolder '\' plot_name]);
        end   
    end
    
end

