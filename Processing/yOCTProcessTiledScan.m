%function yOCTProcessTiledScan(varargin)
%This function Processes Tiled scan, my assumption is that scan size is
%very big, so the processed volume will not be returned directly to Matlab,
%but will be saved directly to disk (or cloud).
%For speed purposes, make sure that input and output folder are either both
%local or both on the cloud. In case, both are on the cloud - run using
%cluster.
%USAGE:
%   yOCTProcessTiledScan(tiledScanInputFolder,outputFolder,[params])
%INPUTS:
%   - tiledScanInputFolder - where tiled scan is saved. Make sure the
%       ScanInfo.json is present in the folder
%   - outputFolder - where products of the processing are saved. Make sure
%       its empty.
%   - params can be any processing parameters used by
%     yOCTLoadInterfFromFile or yOCTInterfToScanCpx or any of those below
%NAME VALUE INPUTS:
%   Parameter           Default Value   Notes
%   outputFileFormat    'tif'           Can be 'tif' or 'mat' files
%   focusSigma          20              If stitching along Z axis (multiple focus points), what is the size of each focus in z [pixel]
%   focusPositionInImageZpix NaN        Z position [pix] of focus in each scan (one number)
%   debugFolder         ''              Where to save debug information (if needed). if empty will save at the output folder: outputFolder/../debug/
%   saveYs              3               How many Y planes to save (for future reference)
%   v                   true            verbose mode      
%OUTPUT:
%   No output is returned. Will save scan Abs to outputFolder, and
%   debugFolder

%% Processing of input
p = inputParser;
addRequired(p,'tiledScanInputFolder',@isstr);

%Define the outputs
addRequired(p,'outputFolder',@isstr);
addParameter(p,'outputFileFormat','tif',@(x)(strcmpi(x,'tif') | strcmpi(x,'mat')))

%General parameters
addParameter(p,'focusSigma',20,@isnumeric);
addParameter(p,'focusPositionInImageZpix',NaN,@isnumeric);

%Debug parameters
addParameter(p,'debugFolder','',@isstr);
addParameter(p,'saveYs',3,@isnumeric);
addParameter(p,'v',true,@islogical);

p.KeepUnmatched = true;
parse(p,varargin{:});

%Gather unmatched varibles, we will use them as passing inputs
vals = struct2cell(p.Unmatched);
nams = fieldnames(p.Unmatched);
tmp = [nams(:)'; vals(:)']; 
reconstructConfig = tmp(:)';

in = p.Results;
v = in.v;
in.outputFileFormat = lower(in.outputFileFormat);

%Set credentials
if awsIsAWSPath(in.outputFolder)
    awsSetCredentials(1);
elseif awsIsAWSPath(in.tiledScanInputFolder)
    awsSetCredentials();
end

%% Setup directories

%Input & Output directories
tiledScanInputFolder = awsModifyPathForCompetability([fileparts(in.tiledScanInputFolder) '/']);
in.outputFolder = awsModifyPathForCompetability([in.outputFolder '/']);
outputFolder = in.outputFolder;

%Debug directory
if isempty(in.debugFolder)
    in.debugFolder = awsModifyPathForCompetability([outputFolder '../debug/']);
else
    in.debugFolder = [in.debugFolder '/'];
end
in.debugFolder = awsModifyPathForCompetability(in.debugFolder);
debugFolder = in.debugFolder;
if ~awsIsAWSPath(debugFolder) && ~exist(debugFolder,'dir')
    mkdir(debugFolder);
end

%Directory to save mat files
if (~strcmp(in.outputFileFormat,'mat'))
    %Final output is not mat, so save them in a temoprary location
    matYFramesFolder = awsModifyPathForCompetability([debugFolder 'yFrames/']);
else
    %Output folder
    matYFramesFolder = outputFolder;
end
awsRmDir(matYFramesFolder);
if ~awsIsAWSPath(matYFramesFolder) && ~exist(matYFramesFolder,'dir')
    mkdir(matYFramesFolder);
end

%Directory to save tif files (if required)
if (strcmp(in.outputFileFormat,'tif'))
    tifYFramesFolder = outputFolder;
    awsRmDir(tifYFramesFolder);
    if ~awsIsAWSPath(tifYFramesFolder) && ~exist(tifYFramesFolder,'dir')
        mkdir(tifYFramesFolder);
    end
end

%Saved stacks dir (if required)
if (in.saveYs > 0)
    yToSaveMatDir = awsModifyPathForCompetability([debugFolder 'savedStacksMat/']);
    awsRmDir(yToSaveMatDir);
    if ~awsIsAWSPath(yToSaveMatDir) && ~exist(yToSaveMatDir,'dir')
        mkdir(yToSaveMatDir);
    end
    
    if (strcmp(in.outputFileFormat,'tif'))
        %Create a Tif version
        yToSaveTifDir = awsModifyPathForCompetability([debugFolder 'savedStacksTif/']);
        awsRmDir(yToSaveTifDir);
        if ~awsIsAWSPath(yToSaveTifDir) && ~exist(yToSaveTifDir,'dir')
            mkdir(yToSaveTifDir);
        end
    end
end

%% Load configuration file & set parameters
json = awsReadJSON([tiledScanInputFolder 'ScanInfo.json']);

fp = cellfun(@(x)(awsModifyPathForCompetability([tiledScanInputFolder '\' x '\'])),json.octFolders,'UniformOutput',false);
n = json.tissueRefractiveIndex;
focusPositionInImageZpix = in.focusPositionInImageZpix;
focusSigma = in.focusSigma;
OCTSystem = json.OCTSystem; %Provide OCT system to prevent unesscecary polling of file system
dimOneTile = ...
	yOCTLoadInterfFromFile([fp(1), reconstructConfig, {'OCTSystem',OCTSystem,'peakOnly',true}]);
tmp = zeros(size(dimOneTile.lambda.values(:)));
dimOneTile = yOCTInterfToScanCpx ([{tmp}, {dimOneTile},{'n'},{json.tissueRefractiveIndex}, reconstructConfig, {'peakOnly'},{true}]);
zToScan = json.zToScan;
xToScan = json.xToScan;
yToScan = json.yToScan;

%% Create indexing reference
%This specifies how to mesh together a tiled scan, each axis seperately

%Dimensions of one tile (mm)
xOneTile = json.xCenter+json.xRange*linspace(-0.5,0.5,json.nXPixels);
dx = diff(xOneTile(1:2));
yOneTile = json.yCenter+json.yRange*linspace(-0.5,0.5,json.nYPixels);
dy = diff(yOneTile(1:2));
zOneTile = dimOneTile.z.values(:)'/1000; %[mm]
dz = diff(zOneTile(1:2));

%Dimensions of the entire stack
xAll = (min(xToScan)+xOneTile(1)):dx:(max(xToScan)+xOneTile(end)+dx);xAll = xAll(:);
yAll = (min(yToScan)+yOneTile(1)):dy:(max(yToScan)+yOneTile(end));yAll = yAll(:);
zAll = (min(zToScan)+zOneTile(1)):dz:(max(zToScan)+zOneTile(end));zAll = zAll(:);
in.xAllmm = xAll;
in.yAllmm = yAll;
in.zAllmm = zAll;

if(~isnan(focusPositionInImageZpix))
    %Remove Z positions that are way out of focus (if we are doing focus processing)
    zAll( ...
        ( zAll < min(zToScan) + zOneTile(round(max(focusPositionInImageZpix - 4*in.focusSigma,0))) ) ...
        | ...
        ( zAll > max(zToScan) + zOneTile(round(min(focusPositionInImageZpix + 4*in.focusSigma,length(zOneTile)))) ) ...
        ) = []; 
end

imOutSize = [length(zAll) length(xAll) length(yAll)];

%For each Y, what files are being used
yGroupSF = zeros(length(json.yToScan),2); %[start yI, end yI]
yGroupFP =  cell(length(json.yToScan),1); %Which files to use for each group
for i=1:length(yGroupFP)
    yI = abs(yAll-json.yToScan(i)) <= json.yRange/2;
    yGroupSF(i,:) = [find(yI,1,'first') find(yI,1,'last')];
    yGroupFP(i) = {fp(json.gridYcc == json.yToScan(i))};
end

%Save some stacks
saveYs = in.saveYs;
yToSave = round(linspace(1,length(yAll),saveYs));

%% Main loop
minmaxVals = zeros(length(yIndexes),2); %min and max values for each yFrame. This is used for Tif later on
printStatsEveryyI = min(floor(length(yAll)/20),1);
ticBytes(gcp);
if(v)
    fprintf('%s Stitching ...\n',datestr(datetime)); tt=tic();
end
for yI=1:length(yAll)
    try
        %Create a container for all data
        stack = zeros(imOutSize(1:2)); %#ok<PFBNS> %z,x,zStach
        totalWeights = zeros(imOutSize(1:2)); %#ok<PFBNS> %z,x
        
        %Relevant OCT files for this y
        yGroup = find(yGroupSF(:,1) <= yI & yI <= yGroupSF(:,2),1,'first');
        fps = yGroupFP{yGroup};
        fileI = 1;
        
        %What is the y index in the file corresponding to this yI
        yIInFile = yI - yGroupSF(yGroup,1)+1;
        
        %Loop over all x stacks
        for xxI = 1:length(xToScan)
            %Loop over depths stacks
            for zzI=1:length(zToScan)
                
                %Frame Name
                fpTxt = fps{fileI};
                fileI = fileI+1;
                
                %Load Frame
                [int1,dim1] = ...
                    yOCTLoadInterfFromFile([{fpTxt}, reconstructConfig, {'dimensions',dimOneTile, 'YFramesToProcess',yIInFile}]);
                [scan1,~] = yOCTInterfToScanCpx ([{int1 dim1} reconstructConfig]);
                int1 = []; %Freeup some memory
                scan1 = abs(scan1);
                for i=length(size(scan1)):-1:3 %Average BScan Averages, A Scan etc
                    scan1 = squeeze(mean(scan1,i));
                end
                
                %Filter around the focus
                zI = 1:length(zOneTile); zI = zI(:);
                if ~isnan(focusPositionInImageZpix)
                    factor = repmat(exp(-(zI-focusPositionInImageZpix).^2/(2*focusSigma)^2), [1 size(scan1,2)]);
                else
                    factor = 1; %No focus gating
                end
                
                %Figure out what is the x,z position of each pixel in this file
                x = xOneTile+xToScan(xxI);
                z = zOneTile+zToScan(zzI);
                
                %Add to stack
                [xxAll,zzAll] = meshgrid(xAll,zAll);
                stack = stack + interp2(x,z,scan1.*factor,xxAll,zzAll,'linear',0);
                totalWeights = totalWeights + interp2(x,z,factor,xxAll,zzAll,'linear',0);
                
                %Save Stack, some files for future (debug)
                if (sum(yI == yToSave)>0)
                    tn = [tempname '.mat'];
                    yOCT2Mat(stack,tn)
                    awsCopyFile_MW1(tn, ...
                        awsModifyPathForCompetability(sprintf('%s/y%04d_xtile%04d_ztile%04d.mat',yToSaveMatDir,yI,xxI,zzI)) ...
                        );
                    delete(tn);
                end
            end
        end
            
        %Normalization
        stackmean = stack./totalWeights;
        
        %Save statistics
        minmaxVals(yI,:) = [min(stackmean(:)) max(stackmean(:))];
        
        %Save results to temporary files to be used later (once we know the
        %scale of the images to write
        tn = [tempname '.mat'];
        yOCT2Mat(stackmean,tn)
        awsCopyFile_MW1(tn, ...
            awsModifyPathForCompetability(sprintf('%s/y%04d.mat',matYFramesFolder,yI))...
            ); %Matlab worker version of copy files
        delete(tn);

        %Is it time to print statistics?
        if mod(yI,printStatsEveryyI)==0 && v
            %Stats time!
            ds = fileDatastore(matYFramesFolder,'ReadFcn',@(x)(x),'FileExtensions','.getmeout','IncludeSubfolders',true); %Count all artifacts
            done = length(ds.Files);
            fprintf('%s Completed yIs so far: %d/%d (%.1f%%)\n',datestr(datetime),done,length(yIndexes),100*done/length(yIndexes));
        end

    catch ME
        fprintf('Error happened in parfor, iteration %d:\n',yI); 
        disp(ME.message);
        for j=1:length(ME.stack) 
            ME.stack(j) 
        end  
        error('Error in parfor');
    end
end %parfor

in.minmaxVals = minmaxVals; %Save statistics

if (v)
    fprintf('Done stitching, toatl time: %.0f[min]\n',toc(tt)/60);
    tocBytes(gcp)
end

%% Reorganizing files
% Move files outside of their folder
if (v)
    fprintf('%s Reorg files ... ',datestr(datetime));
    tt=tic;
end
awsCopyFile_MW2(debugFolder);
if (v)
    fprintf('Done! took %.1f[min]\n',toc(tt)/60);
end

%% Save a json file with the results
awsWriteJSON(in,[outputFolder 'processedScanConfig.json']);

%% Convert to tif if required (main dataset)
if ~strcmp(in.outputFileFormat,'tif')
    return; %We are done
end
if(v)
    fprintf('%s Converting to Tiff ...\n',datestr(datetime)); tt=tic();
end

c = [prctile(minmaxVals(:,1),20)  prctile(minmaxVals(:,2),95)];

%Load all mat files
s = fileDatastore(matYFramesFolder,'ReadFcn',@(x)(x),'FileExtensions','.mat','IncludeSubfolders',true); 
files = ds.Files;

ticBytes(gcp);
for yI=1:length(files)
    %Read
    slice = yOCTFromMat(files{yI});
    
    %Apply threshold
    slice(slice<th) = th;
    slice = log(slice);
    
    %Write
    tn = [tempname '.tif'];
    yOCT2Tif(slice,tn, log(c))
    awsCopyFile_MW1(tn, ...
        sprintf('%sy%04d.tif',tifYFramesFolder,yI)...
        ); %Matlab worker version of copy files
    delete(tn);
end
%Reorganize
awsCopyFile_MW2(tiffOutputFolder);

awsRmDir(matYFramesFolder); %Remove mat files, we are done

if (v)
    fprintf('Done saving as tif, toatl time: %.0f[min]\n',toc(tt)/60);
    tocBytes(gcp);
end
   

%% Convert to Tif (some stacks)
if ~isempty(yToSave)
    
    %Get the mat files
    s = fileDatastore(yToSaveMatDir,'ReadFcn',@(x)(x),'FileExtensions','.mat','IncludeSubfolders',true); 
    matFiles = ds.Files;
    
    tifFiles = cellfun(@(x)(strep(strrep(x,'.mat','.tif'),yToSaveMatDir,yToSaveTifDir)),matFiles,'UniformOutput',false);

    parfor i=1:length(yToSave)
        im = yOCTFromMat(matFiles{i});

        tn = [tempname '.tif'];
        yOCT2Tif(log(im),tn); %Save to temp file
        awsCopyFile_MW1(tn, tifFiles{i}); %Matlab worker version of copy files
        delete(tn);
    end
    awsCopyFile_MW2(tifFiles); %Finish the job
    
    awsRmDir(matFiles); %Remove mat files, we are done
end
    
