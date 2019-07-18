function myOCTBatchProcess(OCTFolders,config)
%This goes over multiple folders one after the other and preforms basic 
%processing: B scan averaging and speckle variance. which will be saved as
%tifs at the same folder with the data.
%This function can prcoess 2D or 3D volumes but not 1D (will skip 1D)
%
%USAGE:
%   myOCTBatchProcess(OCTFolders,config);
%INPUTS:
%   - OCTFolders - OCT Folder to process, can be a pointer to a specific
%       folder or a folder that contains folders to process
%   config - cell array with key-value pairs. For example:
%       {'outputFilePrefix','a'}.
%       It contains parameters to be passed to yOCTLoadInterfFromFile
%       and yOCTInterfToScanCpx, yOCTProcessScan or any of the parameters
%       below:
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                 Default     Information & Values
% 'outputFilePrefix'        ''          output file name prefix. Example:
%                                       outputFilePrefix = 'band1_' will
%                                       generate file: band1_scanAbs.tif
% 'parallelOption'          []          Force select which paralelization
%                                       option to choose, see PARALLEL COMPUTING below
% 'isSaveMat'               false       Should save mat files as well as
%                                       tif? set to true to recive higher
%                                       resolution results.
%OUTPUTS:
%   - No Outputs, will save data directly to the input folder 
%
%PARALLEL COMPUTING:
%This function utilizes parallel computing, two options exists:
%   (1) Every file in OCTFolders is processed in parallel to other files.
%   (2) OCTFoldres are processed one after the other, but B Scan averages
%       within a volume are processed in parallel.
%Option (1):
%parfor OCT_i=1:length(OCTFolders)
%   for BScan_i=1:length(BScans)
%       ... Processing
%   end
%end
%Option (2):
%for OCT_i=1:length(OCTFolders)
%   parfor BScan_i=1:length(BScans)
%       ... Processing
%   end
%end
%Because of Matlab restrictions, only option: (1) or option (2) can run but
%not both (see neasted parfor entrey on mathworks).
%Option (1) has less overhead and is prefered unless number of OCTFolders 
%is low which means some workers will not be utilized. 
%User can select whic configuration to run:
%   parallelOption = 1 will force Option (1).
%   parallelOption = 2 will force Option (2).
%   parallelOption = [] (default) will let this code decide which option to
%       take

%% Get configuration out of config
outputFilePrefix = '';
parallelOption = [];
isSaveMat = false;
if exist('config','var')
    for i=1:2:length(config)
       eval([config{i} ' = config{i+1};']); %<-TBD - there should be a safer way
    end
else
    config = {};
end
config = config(:)';

%% Get what OCT Folders are in the path provided
OCTFolders = strtrim(OCTFolders); %Delete leading and trailing whitespace charecters from path
[OCTFolders_,folderNames] = yOCTGetOCTFoldersInPath (OCTFolders);
if (isempty(OCTFolders_))
	error([ '"' OCTFolders '" Does not have any OCT files or folders']);
end
OCTFolders = OCTFolders_;

%% Preprocess
for i=1:length(OCTFolders)   
    %See if folder is an .OCT file. if so, unzip it first
    if (strcmpi(OCTFolders{i}(end+(-3:0)),'.oct'))
        disp(['Unzipping ' OCTFolders{i}]);
       
		yOCTUnzipOCTFolder(OCTFolders{i},OCTFolders{i}(1:(end-4)));
		OCTFolders{i} = OCTFolders{i}(1:(end-4));
		folderNames{i} = folderNames{i}(1:(end-4));
    end
end

%% Make a choice between parallel options
if (isempty(parallelOption))
    if (length(OCTFolders) > 4)
        %Break even point - if number of folders is more than 3 it is better to
        %run on the cloud than local server where faster speed can be utilized
        %even without fully using all the workers. Plus less overhead on data
        %transfer, everything is done on the cloud
        parallelOption = 1;
    else
        parallelOption = 2;
    end
end

%% Process
fprintf('Starting parallel processing, option #%d\n',parallelOption);
gcp; %Start Parallel Processs
overview = cell(size(OCTFolders));
if (parallelOption == 1)
    parfor i=1:length(OCTFolders)
        tic;
        fprintf('Processing File: %s (%d of %d) ...\n',folderNames{i},i,length(OCTFolders));
        o = process(OCTFolders{i},config,outputFilePrefix,isSaveMat);
        overview{i} = o;
        fprintf('Done, total time: %.1f[min]\n',toc()/60);
    end
else
    for i=1:length(OCTFolders)
        tic
        fprintf('Processing File: %s (%d of %d) ...\n',folderNames{i},i,length(OCTFolders));
        o = process(OCTFolders{i},config,outputFilePrefix,isSaveMat);
        overview{i} = o;
        fprintf('Done, total time: %.1f[min]\n',toc()/60);
    end
end
	
%% Generate Tiffs with overviews
for i=1:length(overview)
    o = overview{i};
    folderName = folderNames{i};
    
    yOCT2Tif(o.BScan_MidWay,[folderName outputFilePrefix 'BScan.tif']);
    yOCT2Tif(o.SpeckleVarMaxProjection,[folderName outputFilePrefix 'speckleVarMaxProjection.tif']);
end

%% Where files are located in the cloud
fid = fopen('WhereAreMyFiles.txt','w'); 
fprintf(fid,'here');
fclose(fid);
%TBD


%% This function preforms the actual processing
%It is written as a function so we can run it as parallel loop or for loop
%depending on the configuration
function overview = process(OCTFolder,config,outputFilePrefix,isSaveMat)

%Make sure this is atleast a 2D scan, otherwise we don't sopport it
pk = yOCTLoadInterfFromFile(OCTFolder,'PeakOnly',true);
if (length(pk.x.values) == 1)
    fprintf('This is a 1D file, not supported as we might not have apodization data\n');
    return;
end	

%Load OCT Data, parllel computing style
[meanAbs,speckleVariance] = yOCTProcessScan([{OCTFolder, ...
    {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
    'nYPerIteration', 1, ...
    'showStats',true} config]);

%Modify Speckle Variance
speckleVariance = speckleVariance./meanAbs;

%Save data
yOCT2Tif(mag2db(meanAbs),[OCTFolder '/' outputFilePrefix 'scanAbs.tif']); %Save to File
if (isSaveMat)
    yOCT2Mat(meanAbs,        [OCTFolder '/' outputFilePrefix 'scanAbs.mat']); %Save raw data to File
end
yOCT2Tif(speckleVariance,[OCTFolder '/' outputFilePrefix 'speckleVariance.tif']); %Save to File

%% Make overview files

%Selected B Scan
overview.BScan_MidWay = mag2db(squeeze(meanAbs(:,:,ceil(end/2))));

%Max Projection
overview.SpeckleVarMaxProjection
mpSV = speckleVariance;
mpSV(meanAbs<1) = 0; %Threshlod
mpSV(1:50,:,:) = 0; %Top of the image is usually noise
mp = squeeze(max(mpSV,[],1)); %Project along z
overview.SpeckleVarMaxProjection = mp;
