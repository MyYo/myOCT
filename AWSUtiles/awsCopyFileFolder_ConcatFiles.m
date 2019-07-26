function [bigFilesNames] = awsCopyFileFolder_ConcatFiles (baseLibrary,outFile,d,v,maxBytesPerBigFile)
%An auxilary function for awsCopyFileFolder. Sending multiple small files
%to s3 takes a lot of time, it is much better to concatinate files to a big
%file, upload it and split it at the server (s3). This script concatinate
%files quickly
%INPUTS
%   baseLibrary - folder to start, all files in this folder will be
%       concatinated
%   outFile - name of output file
%   v - verboose mode. Default: true
%   d - dir structure if already preformed. Default: []
%   maxBytesPerBigFile - ow many bytes per big file not to exceed, default 10e9
%OUTPUT
%   bigFilesNames - cell arry with the big files created
%Author: Yonatan W. 26 July, 2019

%% Configuration
if ~exist('maxBytesPerBigFile','var') || isempty(maxBytesPerBigFile)
    maxBytesPerBigFile = 10e9; %How many bytes per big file not to exceed
end

if ~exist('v','var') || isempty(v)
    v = true;
end

%% Get all files
tall = tic();
if (v)
    disp('Listing Files...');
end
if ~exist('d','var') || isempty(d)
    d = dir([baseLibrary '\**\*.*']);
end
baseLibrary = [d(1).folder '\']; %Convert relative baseLibrary to the abselute path
d([d.isdir]) = []; %Delete all directories

%% Devide to copy commands units
%Assumption is that all files in a folder go together 
%Multiple folders can be concatinated together if maxBytesPerBigFile is not
%exceeded

folders = unique({d(:).folder})';

dBytes = [d.bytes]';
dWhichFolder =  zeros(size(d(:))); %for each file, which folder is it part of
folderBytes = zeros(size(folders)); %Folder size (direct size)

for i=1:length(folders)
    fileIdInThisfolder = cellfun(@(x)(strcmp(x,folders{i})),{d.folder})';
    dWhichFolder(fileIdInThisfolder) = i;
    
    folderBytes(i) = sum(dBytes(dWhichFolder==i));
end

%Remove empty folders
folders(folderBytes==0) = [];
folderBytes(folderBytes==0) = [];

%Make sure big file is big enugh
if (max(folderBytes)>maxBytesPerBigFile)
    warning('Big file needs to be bigger');
    maxBytesPerBigFile = max(folderBytes);
end

%Devide folders into big files based on their size
folderWhichBig = zeros(size(folders)); %For each folder, which big is it goint to
currentBig = 1;
pr = randperm(length(folders)); %Try to sperad the folders such that not all folders with the same meory size are together
for i=pr
    
    if (folderBytes(i) + sum(folderBytes(folderWhichBig==currentBig)) > maxBytesPerBigFile)
        %Adding this folder will cause us to exceed limit
        currentBig = currentBig+1;
    end
    
    folderWhichBig(i) = currentBig;
end

%% Build CMD command & Headers
cmds = cell(max(folderWhichBig),1);
cmdBytes = zeros(size(cmds));
cmdFiles = cmdBytes;
bigFilesNames = cmds;
for iB = 1:length(cmds)
    bigName = sprintf('%s%02d',outFile,iB);
    bigFilesNames{iB} = bigName;
    
    %Build command
    flds = sprintf('"%s\\*.*" /b + ',folders{folderWhichBig==iB});
    cmds{iB} = sprintf('copy %s "%s.bin" /b',flds(1:(end-2)),bigName);
    
    %Build headerfile
    fIDHeader = fopen([bigName '.txt'],'w');
    fprintf(fIDHeader,'fStart[byte] fEnd[byte] path\n');
    pos = 0;
    cmdBytes(iB) = 0;
    cmdFiles(iB) = 0;
    ff1 = find(folderWhichBig==iB);
    for folderIi = 1:length(ff1) %Loop over folders
        folderI = ff1(folderIi);
        ff2 = find(dWhichFolder == folderI);
        for fileIi = 1:length(ff2) %Loop over files in folder
            fileI = ff2(fileIi);
            fp = [d(fileI).folder '\' d(fileI).name];
            
            fStart = pos+1;
            fEnd   = pos+d(fileI).bytes;
            fprintf(fIDHeader,'%d %d %s\n',fStart,fEnd,fp((length(baseLibrary)):end));
    
            %Move pointer position
            pos = pos+d(fileI).bytes;
        end
        
        %Some statistics
        cmdBytes(iB) = cmdBytes(iB)+folderBytes(folderI);
        cmdFiles(iB) = cmdFiles(iB)+sum(dWhichFolder == folderI);
    end
    fclose(fIDHeader);
end

%% Build the big file
if (v)
    disp('Copy files ...');
end
for iB=1:length(cmds)
    
    tic();
    if false
        [err] = system(cmds{iB});
        extx = '';
    else
        [err,etxt]=system(cmds{iB});
    end
    dt = toc();
    
    if (err)
        error(['Error happend while copy ' etxt]);
    end
    
    %Provide progress report
    if (v)
        dd = cmdBytes(iB);
            
        speed = 1/1024^2 * (dd/dt);
        fprintf('%s %.1eBytes of %.1eBytes (%%%.1f), %d Files of %d Done. Speed: %.1fMB/sec\n',...
            datestr(now),...
            sum(cmdBytes(1:iB)), sum(cmdBytes),100*sum(cmdBytes(1:iB))/sum(cmdBytes),...
            sum(cmdFiles(1:iB)), sum(cmdFiles), ...
            speed);
    end
end
if (v)
    fprintf('Done. Took %.1f min all together\n',toc(tall)/60);
end