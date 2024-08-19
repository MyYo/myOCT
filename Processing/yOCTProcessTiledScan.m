function yOCTProcessTiledScan(varargin)
%This function Processes Tiled scan, my assumption is that scan size is
%very big, so the processed volume will not be returned directly to Matlab,
%but will be saved directly to disk (or cloud).
%For speed purposes, make sure that input and output folder are either both
%local or both on the cloud. In case, both are on the cloud - run using
%cluster.
%USAGE:
%   yOCTProcessTiledScan(tiledScanInputFolder,outputPath,[params])
%   yOCTProcessTiledScan({parameters})
%INPUTS:
%   - tiledScanInputFolder - where tiled scan is saved. Make sure the
%       ScanInfo.json is present in the folder
%   - outputPath - where products of the processing are saved. can be a
%       a string (path) to tif file or folder (for tif folder). If you
%       input a cell array with both file and folder, will save both
%   - params can be any processing parameters used by
%     yOCTLoadInterfFromFile or yOCTInterfToScanCpx or any of those below
%NAME VALUE INPUTS:
%   Parameter           Default Value   Notes
%Z position stitching:
%   focusSigma                  20      If stitching along Z axis (multiple focus points), what is the size of each focus in z [pixel]
%   focusPositionInImageZpix    NaN     Z position [pix] of focus in each scan (one number)
%   zSetOriginAsFocusOfZDepth0  true    When saving volume how to set z coordinate system. 
%                                       When set to true, will consider the focus position of the volume that was scanned at zDepths=0.
%                                       When set to false, will take the top of the OCT scan for zDepths=0.
%Save some Y planes in a debug folder:
%   yPlanesOutputFolder         ''      If set will save some y planes for debug purpose in that folder
%   howManyYPlanes              3       How many y planes to save (if yPlanesOutput folder is set)
%Other parameters:
%   applyPathLengthCorrection true  Apply path link correction, if probe ini has the information.
%   v                         true        verbose mode      
%
%OUTPUT:
%   No output is returned. Will save mag2db(scan Abs) to outputPath, and
%   debugFolder

%% Parameters
% In case of using focusSigma, how far from focus to go before cutting off
% the signal, avoiding very low values (on log scale)
cuttoffSigma = 3; 

%% Input Processing
p = inputParser;
addRequired(p,'tiledScanInputFolder',@isstr);

% Define the outputs
addRequired(p,'outputPath');

% Z position stitching
addParameter(p,'dispersionQuadraticTerm',79430000,@isnumeric);
addParameter(p,'focusSigma',20,@isnumeric);
addParameter(p,'focusPositionInImageZpix',NaN,@isnumeric);
addParameter(p,'zSetOriginAsFocusOfZDepth0',true);

% Save some Y planes in a debug folder
addParameter(p,'yPlanesOutputFolder','',@isstr);
addParameter(p,'howManyYPlanes',3,@isnumeric);

% Debug
addParameter(p,'v',true,@islogical);

%TODO(yonatan) shift this parameter to ProcessScanFunction
addParameter(p,'applyPathLengthCorrection',true);

p.KeepUnmatched = true;
if (~iscell(varargin{1}))
    parse(p,varargin{:});
else
    parse(p,varargin{1}{:});
end

% Gather unmatched varibles, we will use them as passing inputs
vals = struct2cell(p.Unmatched);
nams = fieldnames(p.Unmatched);
tmp = [nams(:)'; vals(:)']; 
reconstructConfig = tmp(:)';

in = p.Results;
v = in.v;

% Fix input path
tiledScanInputFolder = awsModifyPathForCompetability([fileparts(in.tiledScanInputFolder) '/']);

% Fix output path
outputPath = in.outputPath;
if ischar(outputPath)
    outputPath = {outputPath};
end

% Set credentials
if any(cellfun(@(x)(awsIsAWSPath(x)),outputPath))
    % Any of the output folders is on the cloud
    awsSetCredentials(1);
elseif awsIsAWSPath(in.tiledScanInputFolder)
    % Input folder is on the cloud
    awsSetCredentials();
end

zSetOriginAsFocusOfZDepth0 = in.zSetOriginAsFocusOfZDepth0;
if (zSetOriginAsFocusOfZDepth0 && isnan(in.focusPositionInImageZpix))
    warning('Because no focus position was set, zSetOriginAsFocusOfZDepth0 cannot be "true", changed to "false". See help of yOCTProcessTiledScan function.');
    zSetOriginAsFocusOfZDepth0 = false;
end

%% Load configuration file & set parameters
json = awsReadJSON([tiledScanInputFolder 'ScanInfo.json']);

%Figure out dispersion parameters
if isempty(in.dispersionQuadraticTerm)
    if isfield(json.octProbe,'DefaultDispersionParameterA')
        warning('octProbe has dispersionParameterA which is beeing depriciated in favor of dispersionQuadraticTerm. Please adjust probe.ini');
        in.dispersionParameterA = json.octProbe.DefaultDispersionParameterA;
        reconstructConfig = [reconstructConfig {'dispersionParameterA', in.dispersionParameterA}];
    else
        in.dispersionQuadraticTerm = json.octProbe.DefaultDispersionQuadraticTerm;
        reconstructConfig = [reconstructConfig {'dispersionQuadraticTerm', in.dispersionQuadraticTerm}];
        reconstructConfig = [reconstructConfig {'n', json.tissueRefractiveIndex}];
    end
else
    reconstructConfig = [reconstructConfig {'dispersionQuadraticTerm', in.dispersionQuadraticTerm}];
    reconstructConfig = [reconstructConfig {'n', json.tissueRefractiveIndex}];
end


fp = cellfun(@(x)(awsModifyPathForCompetability([tiledScanInputFolder '\' x '\'])),json.octFolders,'UniformOutput',false);
focusPositionInImageZpix = in.focusPositionInImageZpix;
% If the focus position is the same for all volumes, create a vector that
% stores the focus position for each volume. This is mainly to enable
% compatbility with the upgraded yOCTFindFocus which returns a focus value
% for each volume 
if length(in.focusPositionInImageZpix) == 1 %#ok<ISCL>
    focusPositionInImageZpix = in.focusPositionInImageZpix * ones(1, length(json.zDepths));
end
focusSigma = in.focusSigma;
OCTSystem = json.OCTSystem; %Provide OCT system to prevent unesscecary polling of file system

%% Extract some data (refactor candidate)
% Note this is a good place for future refactoring, where we create a
% function that for every yI index specifies which scans to load and where
% their XZ coordinates are compared to the larger grid.
% This is what these varibles are here to do

if ~isfield(json,'xCenters_mm')
    % Backward compatibility
    xCenters = json.xCenters;
    yCenters = json.yCenters;
    yRange = json.yRange;
else
    xCenters = json.xCenters_mm;
    yCenters = json.yCenters_mm;
    yRange = json.tileRangeY_mm;
end
zDepths = json.zDepths;

%% Create dimensions structure for the entire tiled volume
[dimOneTile, dimOutput] = yOCTProcessTiledScan_createDimStructure(tiledScanInputFolder);

if zSetOriginAsFocusOfZDepth0
    % Remove Z positions that are way out of focus (if we are doing focus processing)

    zAll = dimOutput.z.values;

    % Remove depths that are out of focus
    zAll( ...
        ( zAll < min(zDepths) + dimOneTile.z.values(round( ...
            max(focusPositionInImageZpix(1) - cuttoffSigma*in.focusSigma,1) ...
            )) ) ...
        | ...
        ( zAll > max(zDepths) + dimOneTile.z.values(round( ...
            min(focusPositionInImageZpix(end) + cuttoffSigma*in.focusSigma,length(dimOneTile.z.values)) ...
            )) ) ...
        ) = []; 

    dimOutput.z.values = zAll(:)' - zAll(1);
    dimOutput.z.origin = 'z=0 is the focus positoin of OCT image when zDepths=0 scan was taken';
end

%% Save some Y planes in a debug folder if needed
if ~isempty(in.yPlanesOutputFolder) && in.howManyYPlanes > 0
    isSaveSomeYPlanes = true;
    yPlanesOutputFolder = awsModifyPathForCompetability([in.yPlanesOutputFolder '/']);
    
    % Clear folder if it exists
    if awsExist(yPlanesOutputFolder)
        awsRmDir(yPlanesOutputFolder);
    end
    
    %Save some stacks, which ones?
    yToSaveI = round(linspace(1,length(dimOutput.y.values),in.howManyYPlanes));
else
    isSaveSomeYPlanes = false;
    yPlanesOutputFolder = '';
    yToSaveI = [];
end

%% Create indexing reference
%This specifies how to mesh together a tiled scan, each axis seperately

imOutSize = [...
    length(dimOutput.z.values) ...
    length(dimOutput.x.values) ...
    length(dimOutput.y.values)];

%For each Y, what files are being used
yGroupSF = zeros(length(yCenters),2); %[start yI, end yI]
yGroupFP =  cell(length(yCenters),1); %Which files to use for each group
for i=1:length(yGroupFP)
    yI = abs(dimOutput.y.values-yCenters(i)) <= yRange/2;
    yGroupSF(i,:) = [find(yI,1,'first') find(yI,1,'last')];
    yGroupFP(i) = {fp(json.gridYcc == yCenters(i))};
end

%% Main loop
printStatsEveryyI = max(floor(length(dimOutput.y.values)/20),1);
ticBytes(gcp);
if(v)
    fprintf('%s Stitching ...\n',datestr(datetime)); tt=tic();
end
whereAreMyFiles = yOCT2Tif([], outputPath, 'partialFileMode', 1); %Init
clear yI; % Clean up varible to prevent confusion
parfor yI=1:length(dimOutput.y.values) 
    try
        %Create a container for all data
        stack = zeros(imOutSize(1:2)); %#ok<PFBNS> %z,x,zStach
        totalWeights = zeros(imOutSize(1:2)); %z,x
        
        %Relevant OCT files for this y
        yGroup = find(yGroupSF(:,1) <= yI & yI <= yGroupSF(:,2),1,'first'); %#ok<PFBNS>
        fps = yGroupFP{yGroup}; %#ok<PFBNS>
        fileI = 1;
        
        %What is the y index in the file corresponding to this yI
        %Example, let us assume we took two tiles in the y direction each
        %tile has 4 scans in the y direction. Total number of scans is 8.
        %The first scan in the stitched file is tile #1 scan #1, then scan 2
        %and finally tile #1 scan #3. After this one, we need to move to 
        %tile #2 scan #1. We need to have a varible specifing which scan in
        %the file to grab. Altough we are processing the overall scan #5
        %thus yI=5, the scan we would like to grab is yInFile=1
        yIInFile = yI - yGroupSF(yGroup,1)+1;
        
        %Loop over all x stacks
        for xxI=1:length(xCenters)
            %Loop over depths stacks
            for zzI=1:length(zDepths)
                
                %Frame Name
                fpTxt = fps{fileI};
                fileI = fileI+1;
                
                %Load Frame
                int1 = ...
                    yOCTLoadInterfFromFile([{fpTxt}, reconstructConfig, ...
                    {'dimensions', dimOneTile, 'YFramesToProcess', yIInFile, 'OCTSystem', OCTSystem}]);
                [scan1,~] = yOCTInterfToScanCpx([{int1} {dimOneTile} reconstructConfig]);
                int1 = []; %#ok<NASGU> %Freeup some memory
                scan1 = abs(scan1);
                for i=length(size(scan1)):-1:3 %Average BScan Averages, A Scan etc
                    scan1 = squeeze(mean(scan1,i));
                end
                
                if (in.applyPathLengthCorrection && isfield(json.octProbe,'OpticalPathCorrectionPolynomial'))
                    [scan1, scan1ValidDataMap] = yOCTOpticalPathCorrection(scan1, dimOneTile, json);
                end
                
                %Filter around the focus
                zI = 1:length(dimOneTile.z.values); zI = zI(:);
                if ~isnan(focusPositionInImageZpix(zzI))
                    factorZ = exp(-(zI-focusPositionInImageZpix(zzI)).^2/(2*focusSigma)^2) + ...
                        (zI>focusPositionInImageZpix(zzI))*exp(-3^2/2);%Under the focus, its possible to not reduce factor as much 
                    factor = repmat(factorZ, [1 size(scan1,2)]);
                else
                    factor = ones(length(dimOneTile.z.values),length(dimOneTile.x.values)); %No focus gating
                end
                factor(~scan1ValidDataMap) = 0; %interpolated nan values should not contribute to image
            
                
                %Figure out what is the x,z position of each pixel in this file
                x = dimOneTile.x.values+xCenters(xxI);
                z = dimOneTile.z.values+zDepths(zzI);
                
                %Helps with interpolation problems
                x(1) = x(1) - 1e-10; 
                x(end) = x(end) + 1e-10; 
                z(1) = z(1) - 1e-10; 
                z(end) = z(end) + 1e-10; 
                
                %Add to stack
                [xxAll,zzAll] = meshgrid(dimOutput.x.values,dimOutput.z.values);
                stack = stack + interp2(x,z,scan1.*factor,xxAll,zzAll,'linear',0);
                totalWeights = totalWeights + interp2(x,z,factor,xxAll,zzAll,'linear',0);
                
                %Save Stack, some files for future (debug)
                if (isSaveSomeYPlanes && sum(yI == yToSaveI)>0)
                    
                    tn = [tempname '.tif'];
                    im = mag2db(scan1);
                    if ~isnan(focusPositionInImageZpix(zzI))
                        im(focusPositionInImageZpix(zzI),1:20:end) = min(im(:)); % Mark focus position on sample
                    end
                    yOCT2Tif(im,tn);
                    awsCopyFile_MW1(tn, ...
                        awsModifyPathForCompetability(sprintf('%s/y%04d_xtile%04d_ztile%04d.tif',yPlanesOutputFolder,yI,xxI,zzI)) ...
                        );
                    delete(tn);
                    
                    if (xxI == length(xCenters) && zzI==length(zDepths))
                        %Save the last weight
                        tn = [tempname '.tif'];
                        yOCT2Tif(totalWeights,tn);
                        awsCopyFile_MW1(tn, ...
                            awsModifyPathForCompetability(sprintf('%s/y%04d_totalWeights.mat',yPlanesOutputFolder,yI)) ...
                            );
                        delete(tn);
                    end
                end
            end
        end
                      
        %Dont allow factor to get too small, it creates an unstable solution
        minFactor1 = exp(-cuttoffSigma^2/2);
        totalWeights(totalWeights<minFactor1) = NaN; 
            
        %Normalization
        stackmean = stack./totalWeights;
        
        % Save
        yOCT2Tif(mag2db(stackmean), outputPath, ...
            'partialFileMode', 2, 'partialFileModeIndex', yI); 
        
        % Is it time to print statistics?
        if mod(yI,printStatsEveryyI)==0 && v
            % Stats time!
            cnt = yOCTProcessTiledScan_AuxCountHowManyYFiles(whereAreMyFiles);
            fprintf('%s Completed yIs so far: %d/%d (%.1f%%)\n',datestr(datetime),cnt,length(dimOutput.y.values),100*cnt/length(dimOutput.y.values));
        end

    catch ME
        fprintf('Error happened in parfor, yI=%d:\n',yI); 
        disp(ME.message);
        for j=1:length(ME.stack) 
            ME.stack(j) 
        end  
        error('Error in parfor');
    end
end %parfor

if (v)
    fprintf('Done stitching, toatl time: %.0f[min]\n',toc(tt)/60);
    tocBytes(gcp)
end

%% Verify that all files are there
if (v)
    fprintf('%s Verifying all files are there ... ',datestr(datetime));
end

% Count how many files are in the library
cnt = yOCTProcessTiledScan_AuxCountHowManyYFiles(whereAreMyFiles);
    
if cnt ~= length(yAll)
    % Some files are missing, print debug to help trubleshoot 
    fprintf('\nDebug Data:\n');
    fprintf('whereAreMyFiles = ''%s''\n',whereAreMyFiles);
    fprintf('Number of ds files: %d\n',cnt)
    
    % Use AWS ls
    l = awsls(whereAreMyFiles);
    isFileL = cellfun(@(x)(contains(lower(x),'.json')),l);
    cntL = sum(isFileL);
    fprintf('Number of awsls files: %d\n',cntL)
    
    if (cntL ~= length(yAll))
        % Throw an error
        error('Please review "%s". We expect to have %d y planes but see only %d in the folder.\nI didn''t delete folder to allow you to debug.\nPlease remove by running awsRmDir(''%s''); when done.',...
            whereAreMyFiles,length(yAll),cnt,whereAreMyFiles);
    else
        % This is probably a datastore issue
        warning('fileDatastore returned different number of files when compared to awsls. You might want to trubleshoot why this happend.\nFor background, see: %s',...
            'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files');
    end
end

if (v)
    fprintf('Done!\n');
end

%% Reorganizing files
% Move files outside of their folder
if (v)
    fprintf('%s Finalizing saving tif file ... ',datestr(datetime));
    tt=tic;
end

% Get the main data out
yOCT2Tif([], outputPath, 'metadata', dimOutput, 'partialFileMode', 3);

% Get saved y planes out
if isSaveSomeYPlanes
    if (v)
        fprintf('%s Reorg some y planes ... ',datestr(datetime));
    end
    awsCopyFile_MW2(yPlanesOutputFolder);
end
if (v)
    fprintf('Done! took %.1f[min]\n',toc(tt)/60);
end


function cnt = yOCTProcessTiledScan_AuxCountHowManyYFiles(whereAreMyFiles)
% This is an aux function that counts how many files yOCT2Tif saved 
% Any fileDatastore request to AWS S3 is limited to 1000 files in 
% MATLAB 2021a. Due to this bug, we have replaced all calls to 
% fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
% 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
ds = imageDatastore(whereAreMyFiles,'ReadFcn',@(x)(x),'FileExtensions','.getmeout','IncludeSubfolders',true); %Count all artifacts
isFile = cellfun(@(x)(contains(lower(x),'.json')),ds.Files);
cnt = sum(isFile);

