function myOCTBatchProcess(OCTFolders,config)
%OCTFolders path to folder containing a single or multiple OCT folders
%config - key - value cell array for configuration of the excecution 
% Any parameters of yOCTLoadInterfFromFile, yOCTInterfToScanCpx can be used. 
% outputFilePrefix can be set.
%Can process 2D or 3D volumes

%% Get configuration out of config
outputFilePrefix = '';
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

%% Process
gcp; %Start Parallel Processs
for i=1:length(OCTFolders)
    folderName = folderNames{i};
    fprintf('Processing File: %s (%d of %d) ...\n',folderName,i,length(OCTFolders));
	
	%Make sure this is atleast a 2D scan, otherwise we don't sopport it
	pk = yOCTLoadInterfFromFile(OCTFolders{i},'PeakOnly',true);
	if (length(pk.x.values) == 1)
		fprintf('This is a 1D file, not supported as we might not have apodization data\n');
		continue;
	end	
        
    %Load OCT Data, parllel computing style
    [meanAbs,speckleVariance] = yOCTProcessScan([{OCTFolders{i}, ...
        {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
        'nYPerIteration', 1, ...
        'showStats',true} config]);
    
    %Modify Speckle Variance
    speckleVariance = speckleVariance./meanAbs;

    %Save data
    yOCT2Tif(mag2db(meanAbs),[OCTFolders{i} '/' outputFilePrefix 'scanAbs.tif']); %Save to File
    yOCT2Mat(meanAbs,        [OCTFolders{i} '/' outputFilePrefix 'scanAbs.mat']); %Save raw data to File
    yOCT2Tif(speckleVariance,[OCTFolders{i} '/' outputFilePrefix 'speckleVariance.tif']); %Save to File
    
    %Save Overview files
    yOCT2Tif(mag2db(squeeze(meanAbs(:,:,1))),[folderName outputFilePrefix 'BScan_1.tif']);
    yOCT2Tif(mag2db(squeeze(meanAbs(:,:,ceil(end/2)))),[folderName outputFilePrefix sprintf('BScan_%d.tif',ceil(size(meanAbs,3)/2))]);
    yOCT2Tif(mag2db(squeeze(meanAbs(:,:,end))),[folderName outputFilePrefix sprintf('BScan_%d.tif',size(meanAbs,3))]);
    
    %Max Projection
    mpSV = speckleVariance;
    mpSV(meanAbs<1) = 0; %Threshlod
    mpSV(1:50,:,:) = 0; %Top of the image is usually noise
    mp = squeeze(max(mpSV,[],1)); %Project along z
    yOCT2Tif(mp,[folderName outputFilePrefix 'speckleVarMaxProjection.tif']);
end

%% Upload data to the colud
%In later versions
%Regreplace \ with /

%Where files are located in the cloud
fid = fopen('WhereAreMyFiles.txt','w'); 
fprintf(fid,'here');
fclose(fid);
%TBD