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
%   - outputFolder - where products of the processing are saved.
%   - params can be any processing parameters used by
%     yOCTLoadInterfFromFile or yOCTInterfToScanCpx or any of those below
%NAME VALUE INPUTS:
%   Parameter           Default Value   Notes
%   focusSigma          20              If stitching along Z axis (multiple focus points), what is the size of each focus in z [pixel]
%   focusPositionInImageZpix NaN        Z position [pix] of focus in each scan (one number)
%   debugFolder         ''              Where to save debug information (if needed)
%   saveYs              3               How many Y planes to save (for future reference)
%   isKeepDebugFiles    false           Save debug files for future debuging
%   v                   true            verbose mode      
%OUTPUT:
%   No output is returned. Will save scan Abs to outputFolder, and
%   debugFolder


%% Processing of input
p = inputParser;
addRequired(p,'tiledScanInputFolder',@isstr);
addRequired(p,'outputFolder',@isstr);

%General parameters
addParameter(p,'focusSigma',20,@isnumeric);
addParameter(p,'focusPositionInImageZpix',NaN,@isnumeric);

%Debug parameters
addParameter(p,'debugFolder','',@isstr);
addParameter(p,'saveYs',@isnumeric);
addParameter(p,'v',true,@islogical);
addParameter(p,'isKeepDebugFiles',false,@islogical);

p.KeepUnmatched = true;
parse(p,varargin{:});

%Gather unmatched varibles, we will use them as passing inputs
vals = struct2cell(p.Unmatched);
nams = fieldnames(p.Unmatched);
tmp = [nams(:)'; vals(:)']; 
reconstructConfig = tmp(:)';

in = p.Results;
tiledScanInputFolder = awsModifyPathForCompetability([fileparts(in.tiledScanInputFolder) '/']);
outputFolder = in.outputFolder;
v = in.v;

%Set credentials
if awsIsAWSPath(outputFolder)
    awsSetCredentials(1);
elseif awsIsAWSPath(tiledScanInputFolder)
    awsSetCredentials();
end

%% Load configuration file & set output folders
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

%Remove Z positions that are way out of focus
zAll( ...
    ( zAll < min(zToScan) + zOneTile(round(max(focusPositionInImageZpix - 4*in.focusSigma,0))) ) ...
    | ...
    ( zAll > max(zToScan) + zOneTile(round(min(focusPositionInImageZpix + 4*in.focusSigma,length(zOneTile)))) ) ...
    ) = []; 

imOutSize = [length(zAll) length(xAll) length(yAll)];

%For each Y, what files are being used
yGroupSF = zeros(length(json.yToScan),2); %[start yI, end yI]
yGroupFP =  cell(length(json.yToScan),1); %Which files to use for each group
for i=1:length(yGroupFP)
    yI = abs(yAll-json.yToScan(i)) <= json.yRange/2;
    yGroupSF(i,:) = [find(yI,1,'first') find(yI,1,'last')];
    yGroupFP(i) = {fp(json.gridYcc == json.yToScan(i))};
end

%% Prepatre for main loop
dirToSaveProcessedYFrames = awsModifyPathForCompetability([outputFolder '/yFrames_db/']);

%% Main loop
printStatsEveryyI = floor(length(yAll)/20);
ticBytes(gcp);
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
                factor = repmat(exp(-(zI-focusPositionInImageZpix).^2/(2*focusSigma)^2), [1 size(scan1,2)]);
                
                %Figure out what is the x,z position of each pixel in this file
                x = xOneTile+xToScan(xxI);
                z = zOneTile+zToScan(zzI);
                
                %Add to stack
                [xxAll,zzAll] = meshgrid(xAll,zAll);
                stack = stack + interp2(x,z,scan1.*factor,xxAll,zzAll,'linear',0);
                totalWeights = totalWeights + interp2(x,z,factor,xxAll,zzAll,'linear',0);
            end
        end
            
        %Normalization
        stackmean = stack./totalWeights;
        
        %Save results to temporary files to be used later (once we know the
        %scale of the images to write
        tn = [tempname '.mat'];
        yOCT2Mat(stackmean,tn)
        awsCopyFile_MW1(tn, ...
            awsModifyPathForCompetability(sprintf('%s/y%04d.mat',dirToSaveProcessedYFrames,yI))...
            ); %Matlab worker version of copy files
        delete(tn);

        %Is it time to print statistics?
        if mod(yI,printStatsEveryyI)==0 && v
            %Stats time!
            ds = fileDatastore(dirToSaveProcessedYFrames,'ReadFcn',@(x)(x),'FileExtensions','.getmeout','IncludeSubfolders',true); %Count all artifacts
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
end

if (v)
    fprintf('Done stitching, toatl time: %.0f[min]\n',toc(tt)/60);
    tocBytes(gcp)
end



return;
%% Set up for paralel processing
yIndexes=dimOneTile.y.index;
thresholds = zeros(length(yIndexes),1);
cValues = zeros(length(yIndexes),2);
imOutSize = [length(dim.z.values) length(dim.x.values) length(yIndexes)]; %z,x,y

%Directory structure
dirToSaveProcessedYFrames = awsModifyPathForCompetability([OCTVolumesFolder '/yFrames_db/']);
LogFolder = awsModifyPathForCompetability([SubjectFolder '\Log\02 OCT Preprocess\']);
dirToSaveStackDemos = awsModifyPathForCompetability([OCTVolumesFolder '/SomeStacks_db/']);
dirToSaveStackDemosTif = awsModifyPathForCompetability([LogFolder '/SomeStacks/']);
tiffOutputFolder = awsModifyPathForCompetability([OCTVolumesFolder '/VolumeScanAbs/']);
tiffOutput_AsOneFile = [tiffOutputFolder(1:(end-1)) '_OneFile.tif'];


%% ->
%Save some Ys
yToSave = dim.y.index(...
    round(linspace(1,length(dim.y.index),saveYs)) ...
    );


tiledScanInputFolder