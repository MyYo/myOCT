% This script tests loading and saving of Tif files

%% Define data for this plane
data = rand(256,256,5);
data(:,:,2) = data(:,:,2)+2;
data(:,:,3) = -data(:,:,3);
data(:,:,4) = -data(:,:,4)-2;
data(:,:,5) = 10*data(:,:,5);
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

%% Test bit conversion
r1 = rand(10);
c = [0 1];
r2 = yOCT2Tif_ConvertBitsData(yOCT2Tif_ConvertBitsData(r1,c, false),c, true);
if (max(abs(r1(:)-r2(:))) > 2^-14)
    error('Bit conversion test failed');
end

r1 =[0 2 2^(16-1)-1];
c = [0 2^(16-1)-1];
r2 = yOCT2Tif_ConvertBitsData(yOCT2Tif_ConvertBitsData(r1,c, false),c, true);
if (max(abs(r2-r1)) ~= 0)
    error('Bit conversion');
end

if isnan(yOCT2Tif_ConvertBitsData(NaN,c, false)) || ...
        ~isnan(yOCT2Tif_ConvertBitsData(yOCT2Tif_ConvertBitsData(NaN,c, false),c, true))
    error('NaN Conversion Error');
end

%% Test Tif write read (simple)
fprintf('%s From Tif tests Started\n',datestr(now))
yOCT2Tif(data,fp_localFile);

data_ = yOCTFromTif(fp_localFile,'yI',1:2,'xI',1:3,'zI',1:4);
assert(max(max(max(abs(data(1:4,1:3,1:2)-data_))))<1e-3,'From Tif test failed #1');

data_ = yOCTFromTif(fp_localFile,'yI',1:2,'xI',3,'zI',1:4);
assert(max(max(max(abs(data(1:4,3,1:2)-data_))))<1e-3,'From Tif test failed #2');

%% Save to Tif File
fprintf('%s Tif File Tests Started\n',datestr(now))
LoadReadSeeItsTheSame(data(:,:,1),fp_localFile,[],[],0,[],[],false); %Save 2D, don't cleanup
LoadReadSeeItsTheSame(data(:,:,1),fp_localFile); %Save 2D
LoadReadSeeItsTheSame(data,fp_localFile); %Save 3D
LoadReadSeeItsTheSame(data,fp_localFile,[],[],0,0,2:3); %Load only part of the data
LoadReadSeeItsTheSame(data(:,:,1),fp_localFile,meta); %Save 2D with meta
LoadReadSeeItsTheSame(data(:,:,1),fp_s3File,[],[],0,[],[],false); %Save to cloud, don't cleanup
LoadReadSeeItsTheSame(data(:,:,1),fp_s3File); %Save to cloud

%% Save to Tif Folder
fprintf('%s Tif Folder Tests Started\n',datestr(now))
LoadReadSeeItsTheSame(data,fp_localFolder,[],[],0,[],[],false); %Save 3D, don't cleanup
LoadReadSeeItsTheSame(data,fp_localFolder); %Save 3D to folder
LoadReadSeeItsTheSame(data,fp_localFolder,[],[],0,0,2:3); %Load only part of the data
LoadReadSeeItsTheSame(data(:,:,1),fp_localFolder,meta); %Save 2D with meta
LoadReadSeeItsTheSame(data(:,:,1),fp_s3Folder,[],[],0,[],[],false); %Save to cloud, don't cleanup
LoadReadSeeItsTheSame(data(:,:,1),fp_s3Folder); %Save to cloud

%% Save both
fprintf('%s Saving both outputs\n',datestr(now))
LoadReadSeeItsTheSame(data,{fp_localFolder, fp_localFile}); %Save 3D to folder

%% Test Partial Save
fprintf('%s Saving in partial mode\n',datestr(now))

LoadReadSeeItsTheSame([],fp_localFolder,[],[],1,[]); % Init
LoadReadSeeItsTheSame(data(:,:,1),fp_localFolder,[],[],2,1);
LoadReadSeeItsTheSame(data(:,:,2),fp_localFolder,[],[],2,2);
LoadReadSeeItsTheSame(data(:,:,1:2),fp_localFolder,[],[],3,[],[]); %Data is not required but useful for comparing output

LoadReadSeeItsTheSame([],fp_localFile,[],[],1,[]); % Init
LoadReadSeeItsTheSame(data(:,:,1),fp_localFile,[],[],2,1)
LoadReadSeeItsTheSame(data(:,:,2),fp_localFile,[],[],2,2)
LoadReadSeeItsTheSame(data(:,:,1:2),fp_localFile,[],[],3,[],[]); %Data is not required but useful for comparing output

LoadReadSeeItsTheSame([],{fp_localFolder,fp_localFile},[],[],1,[]); % Init
for i=1:size(data,3)
    LoadReadSeeItsTheSame(data(:,:,i),{fp_localFolder,fp_localFile},[],[],2,i);
end
LoadReadSeeItsTheSame(data,{fp_localFolder,fp_localFile},3,[]); %Data is not required but useful for comparing output

%% Test Partial Save in cloud
fprintf('%s Saving in partial mode in cloud\n',datestr(now))

LoadReadSeeItsTheSame([],fp_s3Folder,[],[],1,[]); % Init
LoadReadSeeItsTheSame(data(:,:,1),fp_s3Folder,[],[],2,1);
LoadReadSeeItsTheSame(data(:,:,2),fp_s3Folder,[],[],2,2);
LoadReadSeeItsTheSame(data(:,:,1:2),fp_s3Folder,[],[],3,[],[],false); %Data is not required but useful for comparing output

LoadReadSeeItsTheSame([],fp_s3File,[],[],1,[]); % Init
LoadReadSeeItsTheSame(data(:,:,1),fp_s3File,[],[],2,1)
LoadReadSeeItsTheSame(data(:,:,2),fp_s3File,[],[],2,2)
LoadReadSeeItsTheSame(data(:,:,1:2),fp_s3File,[],[],3,[],[],false); %Data is not required but useful for comparing output

LoadReadSeeItsTheSame([],{fp_s3Folder,fp_s3File},[],[],1,[]); % Init
LoadReadSeeItsTheSame(data(:,:,1),{fp_s3Folder,fp_s3File},[],[],2,1)
LoadReadSeeItsTheSame(data(:,:,2),{fp_s3Folder,fp_s3File},[],[],2,2)
LoadReadSeeItsTheSame(data(:,:,1:2),{fp_s3Folder,fp_s3File},meta,[0 3],3,[]); %Data is not required but useful for comparing output

fprintf('%s Test Done\n',datestr(now))

function LoadReadSeeItsTheSame(data,filePath,meta,clim,partialFileMode,partialFileModeIndex, loadYIndex, isClearFilesWhenDone)
%isClearFilesWhenDone - set to false if you would like to test cleanup of
%next step

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

if ~exist('isClearFilesWhenDone','var') || isempty(isClearFilesWhenDone)
    isClearFilesWhenDone = true;
end

%% Save & Load
yOCT2Tif(data,filePath,'metadata',meta,'clim',clim,...
    'partialFileMode',partialFileMode,'partialFileModeIndex',partialFileModeIndex);
if ~iscell(filePath)
    filePaths = {filePath};
else
    filePaths = filePath;
end

if (partialFileMode == 1 || partialFileMode == 2)
	return; %No time for testing just yet
end

% Loop over all files
for i=1:length(filePaths)
    filePath = filePaths{i};
    
    if (isempty(loadYIndex))
        [data_, meta_] = yOCTFromTif(filePath);
    else
        %Load only part of the data
        [data_, meta_] = yOCTFromTif(filePath,'yI',loadYIndex);
        data = data(:,:,loadYIndex);
    end
    d=dir(filePath);

    %% Cleanup
    if isClearFilesWhenDone
        [~,~,a] = fileparts(filePath);
        if ~isempty(a)
            %File
            awsRmFile(filePath);
        else
            %Directory
            awsRmDir(filePath);
        end
    else
        disp('Not cleaning up!');
    end
    %% Compare
    if max(abs(data(:)-data_(:)))>2^-(16-1-4)
        fprintf('Testing: %s\n',filePath);
        fprintf('Max difference between original and loaded data: %.1f%%. File Size: %.2f Bytes/Data Point\n',...
            max(abs(data(:)-data_(:)))*100,sum([d(:).bytes])/numel(data))
        error('Saving Data is not lossless');
    end

    if exist('meta','var') && ~isequaln(meta,meta_)
        error('meta not equal')
    end
end %Filepath (i)
end