% This script tests loading and saving of Tif files

%% Define data for this plane
data = rand(1024,1024,5);
clear meta;
meta.mymeta = (1:10)';

%% Save to Tif File
fprintf('%s Tif File Tests Started\n',datestr(now))
LoadReadSeeItsTheSame(data(:,:,1),'tmp.tif'); %Save 2D
LoadReadSeeItsTheSame(data,'tmp.tif'); %Save 3D
LoadReadSeeItsTheSame(data,'tmp.tif',[],[],0,0,2:3); %Load only part of the data
LoadReadSeeItsTheSame(data(:,:,1),'tmp.tif',meta); %Save 2D with meta
LoadReadSeeItsTheSame(data(:,:,1),'s3://delazerdamatlab/Users/Jenkins/Tmp.tif'); %Save to cloud

%% Save to Tif Folder
fprintf('%s Tif Folder Tests Started\n',datestr(now))
LoadReadSeeItsTheSame(data,'tmp\'); %Save 3D to folder
LoadReadSeeItsTheSame(data,'tmp\',[],[],0,0,2:3); %Load only part of the data

%% Save both
fprintf('%s Saving both outputs\n',datestr(now))
LoadReadSeeItsTheSame(data,{'tmp\', 'tmp.tif'}); %Save 3D to folder

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
    if max(abs(data(:)-data_(:)))>0.05
        fprintf('Max difference between original and loaded data: %.1f%%. File Size: %.2f Bytes/Data Point\n',...
            max(abs(data(:)-data_(:)))*100,d.bytes/numel(data))
        error('Saving Data is not lossless');
    end

    if exist('meta','var') && ~isequaln(meta,meta_)
        error('meta not equal')
    end
end %Filepath (i)
end