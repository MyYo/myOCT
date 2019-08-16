function awsCopyFile_MW1(source, dest)
%This function is a hack to copy files to the cloud from a Matlab Worker.
%Matlab Parallel Computing Clusterdoes not have AWS CLI support (August 2019), so
%we cannot use awsCopyFileFolder from a Matlab cluster. 
%But what if you did want to upload a file to AWS within a worker use this
%function.
%
%This function comes in two pars:
%
%parfor ..
%   <compute on the cloud>
%   awsCopyFile_MW1(localFile,s3Path); <-- this function uploads to the
%   cloud but in a wired format
%end
%awsCopyFile_MW2(s3Path); <-- this function runs at the end, locally and uses AWS CLI to align the path
%
%INPUTS:
%   - source - local file
%   - dest - S3 path, make sure file or folder does not already exist with that name!
%WORKS ON SINGLE FILES ONLY AT THIS POINT

if awsIsAWSPath(dest)
    awsSetCredentials;
end

%% Get source file name
sourceFolder = fileparts(source);
if isempty(sourceFolder)
    %Replace with fullpath of source
    source = [pwd '/' source];
end

if ~isfile(source)
    error('Cannot find: %s',source);
end

%% Get dest file name
[destfolder,name,ext] = fileparts(dest);
destfileName = [name ext];
if isempty(ext)
    destfolder = [destfolder '/' name];

    %No name at the dest file, use the name from the source
    [~,name,ext] = fileparts(source); 
    destfileName = [name ext];
end

if isempty(destfileName)
    error('Cannot upload folders');
end

%Generate a location, but also a random temporary name 
[~,nm] = fileparts([tempname '.']);
awsLocation = awsModifyPathForCompetability(...
    sprintf('%s/%s/%s/*.getmeout',destfolder,destfileName,nm)...
    );

%% Do The job
T = tall({source});
evalc(... use evalc to reduce number of screen prints
    'write(awsLocation,T,''WriteFcn'',@tallWriter)' ... %Not a trivial implementation but it works
    ); 

if true %For debug, verify that file exists where we wrote it
    try
    ds = fileDatastore(strrep(awsLocation,'*','1'),'ReadFcn',@(x)(x));
    catch
        error('Cannot find a file here:%s',awsLocation);
    end
end

end

function tallWriter (info, data)
if (info.PartitionIndex ~= 1) || (info.BlockIndexInPartition ~= 1)
    error('info.PartitionIndex = %d, info.BlockIndexInPartition = %d - both should be 1',...
        info.PartitionIndex,info.BlockIndexInPartition);
end
ff = strrep(info.RequiredFilePattern,'*','1');%Remove required pattern, its easier that way
filename1 = sprintf('%s/%s',info.RequiredLocation, ff);
%filename1 = info.SuggestedFilename;

copyfile(data{:},filename1);
end