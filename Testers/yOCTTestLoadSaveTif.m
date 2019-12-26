% This script tests loading and saving of Tif files

%% Define data for this plane
data = rand(1024,1024,5);
clear meta;
meta.mymeta = (1:10)';

fp_localFile   = [pwd '\tmp.tif'];
fp_localFolder = [pwd '\tmp\'];
fp_s3File      = 's3://delazerdamatlab/Users/Jenkins/Tmp.tif';
fp_s3Folder    = 's3://delazerdamatlab/Users/Jenkins/Tmp/';

%% Cleanup files and folders if required
if awsExist(fp_localFolder,'dir')
    awsRmDir(fp_localFolder);
end
if awsExist(fp_s3Folder,'dir')
    awsRmDir(fp_s3Folder);
end
if awsExist(fp_localFile,'file')
    awsRmFile(fp_localFile);
end
if awsExist(fp_s3File,'file')
    awsRmFile(fp_s3File);
end

%% Save to Tif File
fprintf('%s Tif File Tests Started\n',datestr(now))
LoadReadSeeItsTheSame(data(:,:,1),fp_localFile); %Save 2D
LoadReadSeeItsTheSame(data,fp_localFile); %Save 3D
LoadReadSeeItsTheSame(data,fp_localFile,[],[],0,0,2:3); %Load only part of the data
LoadReadSeeItsTheSame(data(:,:,1),fp_localFile,meta); %Save 2D with meta
LoadReadSeeItsTheSame(data(:,:,1),fp_s3File); %Save to cloud

%% Save to Tif Folder
fprintf('%s Tif Folder Tests Started\n',datestr(now))
LoadReadSeeItsTheSame(data,fp_localFolder); %Save 3D to folder
LoadReadSeeItsTheSame(data,fp_localFolder,[],[],0,0,2:3); %Load only part of the data
LoadReadSeeItsTheSame(data(:,:,1),fp_localFolder,meta); %Save 2D with meta
LoadReadSeeItsTheSame(data(:,:,1),fp_s3Folder); %Save to cloud

%% Save both
fprintf('%s Saving both outputs\n',datestr(now))
LoadReadSeeItsTheSame(data,{fp_localFolder, fp_localFile}); %Save 3D to folder

%% Test Partial Save
fprintf('%s Saving in partial mode\n',datestr(now))

LoadReadSeeItsTheSame(data(:,:,1),fp_localFolder,[],[],1,1);
LoadReadSeeItsTheSame(data(:,:,2),fp_localFolder,[],[],1,2);
LoadReadSeeItsTheSame(data(:,:,1:2),fp_localFolder,[],[],2,[]); %Data is not required but useful for comparing output

LoadReadSeeItsTheSame(data(:,:,1),fp_localFile,[],[],1,1)
LoadReadSeeItsTheSame(data(:,:,2),fp_localFile,[],[],1,2)
LoadReadSeeItsTheSame(data(:,:,1:2),fp_localFile,[],[],2,[]); %Data is not required but useful for comparing output

LoadReadSeeItsTheSame(data(:,:,1),{fp_localFolder,fp_localFile},[],[],1,1)
LoadReadSeeItsTheSame(data(:,:,2),{fp_localFolder,fp_localFile},[],[],1,2)
LoadReadSeeItsTheSame(data(:,:,1:2),{fp_localFolder,fp_localFile},meta,[0 2],2,[]); %Data is not required but useful for comparing output

fprintf('%s Test Done\n',datestr(now))

function LoadReadSeeItsTheSame(data,filePath,meta,clim,partialFileMode,partialFileModeIndex, loadYIndex)

%% Inputs
if ~exist('meta','var')
    meta = [];
end
if ~exist('clim','var')
    clim = [];
end
if ~exist('partialFileMode','var')
    partialFileMode = 0;
end
if ~exist('partialFileModeIndex','var')
    partialFileModeIndex = 0;
end

if ~exist('loadYIndex','var')
    loadYIndex = [];
end

%% Save & Load
yOCT2Tif(data,filePath,'metadata',meta,'clim',clim,...
    'partialFileMode',partialFileMode,'partialFileModeIndex',partialFileModeIndex);
if ~iscell(filePath)
    filePaths = {filePath};
else
    filePaths = filePath;
end

if (partialFileMode == 1)
	return; %No time for testing just yet
end

% Loop over all files
for i=1:length(filePaths)
    filePath = filePaths{i};
    
    if (isempty(loadYIndex))
        [data_, meta_] = yOCTFromTif(filePath);
    else
        %Load only part of the data
        [data_, meta_] = yOCTFromTif(filePath,loadYIndex);
        data = data(:,:,loadYIndex);
    end
    d=dir(filePath);

    %% Cleanup
    [~,~,a] = fileparts(filePath);
    if ~isempty(a)
        %File
        awsRmFile(filePath);
    else
        %Directory
        awsRmDir(filePath);
    end

    %% Compare
    if max(abs(data(:)-data_(:)))>1/2^14
        fprintf('Max difference between original and loaded data: %.1f%%. File Size: %.2f Bytes/Data Point\n',...
            max(abs(data(:)-data_(:)))*100,sum([d(:).bytes])/numel(data))
        error('Saving Data is not lossless');
    end

    if exist('meta','var') && ~isequaln(meta,meta_)
        error('meta not equal')
    end
end %Filepath (i)
end