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

%% Get file name
[~,name,ext] = fileparts(dest);
fileName = [name ext];

if isempty(fileName)
    error('Cannot upload folders');
end

%Generate a location, but also a random temporary name 
[~,nm] = fileparts([tempname '.']);
awsLocation = awsModifyPathForCompetability(...
    sprintf('%s/%s/*.getmeout',dest,fileName,nm)...
    );

%% Do The job
T = tall({source});
evalc(... use evalc to reduce number of screen prints
    'write(awsLocation,T,''WriteFcn'',@tallWriter)' ... %Not a trivial implementation but it works
    ); 

end

function tallWriter (info, data)
if (info.PartitionIndex ~= 1) || (info.BlockIndexInPartition ~= 1)
    error('info.PartitionIndex = %d, info.BlockIndexInPartition = %d - both should be 1',...
        info.PartitionIndex,info.BlockIndexInPartition);
end
ff = strrep(info.RequiredFilePattern,'*','1');%Remove required pattern, its easier that way
filename1 = sprintf('%s/%s',info.RequiredLocation, ff);
%filename1 = info.SuggestedFilename;

movefile(data{:},filename1);
end