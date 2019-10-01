function yOCTProcessTiledScan(varargin)
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
awsMkDir(debugFolder,false); %Dont clean directory before creating

%Directory to save mat files
if (~strcmp(in.outputFileFormat,'mat'))
    %Final output is not mat, so save them in a temoprary location
    matYFramesFolder = awsModifyPathForCompetability([debugFolder 'yFrames/']);
else
    %Output folder
    matYFramesFolder = outputFolder;
end
awsMkDir(matYFramesFolder,true);

%Directory to save tif files (if required)
if (strcmp(in.outputFileFormat,'tif'))
    tifYFramesFolder = outputFolder;
    awsMkDir(tifYFramesFolder,true);
    
    tifYFrameAllFP = [tifYFramesFolder(1:(end-1)) '_All.tif'];
    awsRmFile(tifYFrameAllFP);
end

%Saved stacks dir (if required)
if (in.saveYs > 0)
    yToSaveMatDir = awsModifyPathForCompetability([debugFolder 'savedStacksMat/']);
    awsMkDir(yToSaveMatDir,true);
    
    yToSaveTotalWeightsDir = awsModifyPathForCompetability([debugFolder 'savedStacksWeight/']);
    awsMkDir(yToSaveTotalWeightsDir,true);
    
    if (strcmp(in.outputFileFormat,'tif'))
        %Create a Tif version
        yToSaveTifDir = awsModifyPathForCompetability([debugFolder 'savedStacksTif/']);
        awsMkDir(yToSaveTifDir,true);
    end
else
    %For parfor transperency 
    yToSaveMatDir = []; 
    yToSaveTotalWeightsDir = [];
end

%% Load configuration file & set parameters
json = awsReadJSON([tiledScanInputFolder 'ScanInfo.json']);

fp = cellfun(@(x)(awsModifyPathForCompetability([tiledScanInputFolder '\' x '\'])),json.octFolders,'UniformOutput',false);
focusPositionInImageZpix = in.focusPositionInImageZpix;
focusSigma = in.focusSigma;
OCTSystem = json.OCTSystem; %Provide OCT system to prevent unesscecary polling of file system
dimOneTile = ...
	yOCTLoadInterfFromFile([fp(1), reconstructConfig, {'OCTSystem',OCTSystem,'peakOnly',true}]);
tmp = zeros(size(dimOneTile.lambda.values(:)));
dimOneTileProcessed = yOCTInterfToScanCpx ([{tmp}, {dimOneTile},{'n'},{json.tissueRefractiveIndex}, reconstructConfig, {'peakOnly'},{true}]);
dimOneTile.z = dimOneTileProcessed.z; %Update only z, not lambda [lambda is changed because of equispacing]
zDepths = json.zDepths;
xCenters = json.xCenters;
yCenters = json.yCenters;

%% Create indexing reference
%This specifies how to mesh together a tiled scan, each axis seperately

%Dimensions of one tile (mm)
xOneTile = json.xOffset+json.xRange*linspace(-0.5,0.5,json.nXPixels+1); xOneTile(end) = [];
dx = diff(xOneTile(1:2));
yOneTile = json.yOffset+json.yRange*linspace(-0.5,0.5,json.nYPixels+1); yOneTile(end) = [];
dy = diff(yOneTile(1:2));
zOneTile = dimOneTile.z.values(:)'/1000; %[mm]
dz = diff(zOneTile(1:2));

%Dimensions of the entire stack
xAll = (min(xCenters)+xOneTile(1)):dx:(max(xCenters)+xOneTile(end)+dx);xAll = xAll(:);
yAll = (min(yCenters)+yOneTile(1)):dy:(max(yCenters)+yOneTile(end)+dy);yAll = yAll(:);
zAll = (min(zDepths)+zOneTile(1)):dz:(max(zDepths)+zOneTile(end)+dz);zAll = zAll(:);

%Correct for the case of only one scan
if (length(xCenters) == 1)
    xAll = xAll(1:length(dimOneTileProcessed.x.values));
end
if (length(yCenters) == 1)
    yAll = yAll(1:length(dimOneTileProcessed.y.values));
end

%Save parameters
in.xAllmm = xAll;
in.yAllmm = yAll;
in.zAllmm = zAll;

if(~isnan(focusPositionInImageZpix))
    %Remove Z positions that are way out of focus (if we are doing focus processing)
    zAll( ...
        ( zAll < min(zDepths) + zOneTile(round(max(focusPositionInImageZpix - 5*in.focusSigma,0))) ) ...
        | ...
        ( zAll > max(zDepths) + zOneTile(round(min(focusPositionInImageZpix + 5*in.focusSigma,length(zOneTile)))) ) ...
        ) = []; 
end

imOutSize = [length(zAll) length(xAll) length(yAll)];

%For each Y, what files are being used
yGroupSF = zeros(length(json.yCenters),2); %[start yI, end yI]
yGroupFP =  cell(length(json.yCenters),1); %Which files to use for each group
for i=1:length(yGroupFP)
    yI = abs(yAll-json.yCenters(i)) <= json.yRange/2;
    yGroupSF(i,:) = [find(yI,1,'first') find(yI,1,'last')];
    yGroupFP(i) = {fp(json.gridYcc == json.yCenters(i))};
end

%Save some stacks
saveYs = in.saveYs;
yToSave = round(linspace(1,length(yAll),saveYs));

%% Main loop
minmaxVals = zeros(length(yAll),2); %min and max values for each yFrame. This is used for Tif later on
printStatsEveryyI = max(floor(length(yAll)/20),1);
ticBytes(gcp);
if(v)
    fprintf('%s Stitching ...\n',datestr(datetime)); tt=tic();
end
parfor yI=1:length(yAll) 
    try
        %Create a container for all data
        stack = zeros(imOutSize(1:2)); %#ok<PFBNS> %z,x,zStach
        totalWeights = zeros(imOutSize(1:2)); %z,x
        
        %Relevant OCT files for this y
        yGroup = find(yGroupSF(:,1) <= yI & yI <= yGroupSF(:,2),1,'first'); %#ok<PFBNS>
        fps = yGroupFP{yGroup}; %#ok<PFBNS>
        fileI = 1;
        
        %What is the y index in the file corresponding to this yI
        yIInFile = yI - yGroupSF(yGroup,1)+1;
        
        %Loop over all x stacks
        for xxI = 1:length(xCenters)
            %Loop over depths stacks
            for zzI=1:length(zDepths)
                
                %Frame Name
                fpTxt = fps{fileI};
                fileI = fileI+1;
                
                %Load Frame
                [int1,dim1] = ...
                    yOCTLoadInterfFromFile([{fpTxt}, reconstructConfig, {'dimensions',dimOneTile, 'YFramesToProcess',yIInFile}]);
                [scan1,~] = yOCTInterfToScanCpx ([{int1 dim1} reconstructConfig]);
                int1 = []; %#ok<NASGU> %Freeup some memory
                scan1 = abs(scan1);
                for i=length(size(scan1)):-1:3 %Average BScan Averages, A Scan etc
                    scan1 = squeeze(mean(scan1,i));
                end
                
                %Filter around the focus
                zI = 1:length(zOneTile); zI = zI(:);
                if ~isnan(focusPositionInImageZpix)
                    factorZ = exp(-(zI-focusPositionInImageZpix).^2/(2*focusSigma)^2) + ...
                        (zI>focusPositionInImageZpix)*exp(-3^2/2);%Under the focus, its possible to not reduce factor as much 
                    factor = repmat(factorZ, [1 size(scan1,2)]); 
                else
                    factor = ones(length(zOneTile),length(xOneTile)); %No focus gating
                end
                
                %Figure out what is the x,z position of each pixel in this file
                x = xOneTile+xCenters(xxI);
                z = zOneTile+zDepths(zzI);
                
                %Helps with interpolation problems
                x(1) = x(1) - 1e-10; 
                x(end) = x(end) + 1e-10; 
                z(1) = z(1) - 1e-10; 
                z(end) = z(end) + 1e-10; 
                
                %Add to stack
                [xxAll,zzAll] = meshgrid(xAll,zAll);
                stack = stack + interp2(x,z,scan1.*factor,xxAll,zzAll,'linear',0);
                totalWeights = totalWeights + interp2(x,z,factor,xxAll,zzAll,'linear',0);
                
                %Save Stack, some files for future (debug)
                if (sum(yI == yToSave)>0)
                    tn = [tempname '.mat'];
                    yOCT2Mat(scan1,tn)
                    awsCopyFile_MW1(tn, ...
                        awsModifyPathForCompetability(sprintf('%s/y%04d_xtile%04d_ztile%04d.mat',yToSaveMatDir,yI,xxI,zzI)) ...
                        );
                    delete(tn);
                    
                    if (xxI == length(xCenters) && zzI==length(zDepths))
                        %Save the last weight
                        tn = [tempname '.mat'];
                        yOCT2Mat(totalWeights,tn)
                        awsCopyFile_MW1(tn, ...
                            awsModifyPathForCompetability(sprintf('%s/y%04d_totalWeights.mat',yToSaveTotalWeightsDir,yI)) ...
                            );
                        delete(tn);
                    end
                end
            end
        end
                      
        %Dont allow factor to get too small, it creates an unstable solution
        minFactor1 = exp(-3^2/2);
        totalWeights(totalWeights<minFactor1) = NaN; 
            
        %Normalization
        stackmean = stack./totalWeights;
        
        %Save statistics
        %minmaxVals(yI,:) = [prctile(stackmean(:),30)  prctile(stackmean(:),95)];
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
            fprintf('%s Completed yIs so far: %d/%d (%.1f%%)\n',datestr(datetime),done,length(yAll),100*done/length(yAll));
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
inToSave = rmfield(in,{'debugFolder','outputFolder','tiledScanInputFolder'});
awsWriteJSON(inToSave,[outputFolder 'processedScanConfig.json']);

%% Convert to tif if required (main dataset)
if ~strcmp(in.outputFileFormat,'tif')
    return; %We are done
end
if(v)
    fprintf('%s Converting to Tiff ...\n',datestr(datetime)); tt=tic();
end

c = [prctile(minmaxVals(:,1),30)  prctile(minmaxVals(:,2),95)];
%c = mean(minmaxVals);%[prctile(minmaxVals(:,1),30)  prctile(minmaxVals(:,2),95)];

%Load all mat files
ds = fileDatastore(matYFramesFolder,'ReadFcn',@(x)(x),'FileExtensions','.mat','IncludeSubfolders',true); 
files = ds.Files;

ticBytes(gcp);
parfor yI=1:length(files)
    try
        %Read
        slice = yOCTFromMat(files{yI});
        slice = log(slice);

        %Write
        tn = [tempname '.tif'];
        yOCT2Tif(slice,tn, log(c))
        awsCopyFile_MW1(tn, ...
            sprintf('%sy%04d.tif',tifYFramesFolder,yI)...
            ); %Matlab worker version of copy files
        delete(tn);
    catch ME
        fprintf('Error happened in parfor, iteration %d:\n',yI); 
        disp(ME.message);
        for j=1:length(ME.stack) 
            ME.stack(j) 
        end  
        error('Error in parfor');
    end
end
%Reorganize
awsCopyFile_MW2(tifYFramesFolder);

awsRmDir(matYFramesFolder); %Remove mat files, we are done

if (v)
    fprintf('Done saving as tif, toatl time: %.0f[min]\n',toc(tt)/60);
    tocBytes(gcp);
end

%% Concate all Tifs to one stack
if(v)
    fprintf('%s Concatinaing Tiffs to one file ...\n',datestr(datetime)); tt=tic();
    ticBytes(gcp('nocreate'))
end

ds = fileDatastore(awsModifyPathForCompetability(tifYFramesFolder),'ReadFcn',@(x)(x),'FileExtensions','.tif','IncludeSubfolders',true); 
files = ds.Files;
sz = [length(zAll) length(xAll) length(yAll)];
parfor(i=1:1,1) %Run once but on a worker
    yTiffAll = zeros(sz);
    
    for j=1:length(files)
       yTiffAll(:,:,j) = yOCTFromTif(files{j});
    end
    
    tn = [tempname '.tif'];
    yOCT2Tif(yTiffAll,tn,log(c));
    awsCopyFile_MW1(tn,tifYFrameAllFP); %Matlab worker version of copy files
    delete(tn);
       
end
awsCopyFile_MW2(tifYFrameAllFP);

if (v)
    fprintf('Done saving as one tif, toatl time: %.0f[min]\n',toc(tt)/60);
    tocBytes(gcp);
end

%% Convert to Tif (some stacks)
if ~isempty(yToSave)
    
    %Get the mat files
    ds = fileDatastore(yToSaveMatDir,'ReadFcn',@(x)(x),'FileExtensions','.mat','IncludeSubfolders',true); 
    matFiles = ds.Files;
    
    tifFiles = cellfun(@(x)(strrep(strrep(x,'.mat','.tif'),yToSaveMatDir,yToSaveTifDir)),matFiles,'UniformOutput',false);

    parfor i=1:length(matFiles)
        im = yOCTFromMat(matFiles{i});

        tn = [tempname '.tif'];
        yOCT2Tif(log(im),tn); %Save to temp file
        awsCopyFile_MW1(tn, tifFiles{i}); %Matlab worker version of copy files
        delete(tn);
    end
    awsCopyFile_MW2(yToSaveTifDir); %Finish the job
    
    awsRmDir(yToSaveMatDir); %Remove mat files, we are done
end
    
