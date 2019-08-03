function awsCopyFileFolder(source,dest,v)
%This function copys files and folders to from/aws

if ~exist('v','var')
    v = true; %Verboose mode
end

%% Set Credentials
awsSetCredentials (1); %Write cridentials are required  

if (awsIsAWSPath(source))
    source = awsModifyPathForCompetability(source,true);
    isSourceAWS = true;
else
    isSourceAWS = false;
end
if (awsIsAWSPath(dest))
    dest = awsModifyPathForCompetability(dest,true);
    isDestAWS = true;
else
    isDestAWS = false;
end

%% Figure out what is been done
mode = '';
if ~isSourceAWS
    if exist(source,'dir')
        mode = 'UploadDir';
        
        %Remove last '\'
        if (source(end)=='\' || source(end)=='/')
            source(end)=[];
        end
        d = dir([source '\**\*.*']);
        if (...
                length(d)>10 && ...
                sum([d.bytes])/1024^3 > 5.0 ... Threshold size GBytes
                )
            mode = 'UploadDirManySmallFiles';
            source = d(1).folder; %Switch to a full path, its better
        end
    else
        mode = 'UploadFile';
    end
end

%% Preform the upload
switch(mode)
    case 'UploadFile'
        [err] = system(['aws s3 cp "' source '" "' dest '"']);
    case 'UploadDir'
        [err] = system(['aws s3 sync "' source '" "' dest '"']);
    case 'UploadDirManySmallFiles'
        err = 0;
        awsCopyFileFolder_ManySmallFiles(source,dest,v);
    otherwise
        error('Couldn''t figure out the mode of operation');
end

if err~=0
    error(['error happend while using aws: ']);
end

%% Fast copy of many small files
function awsCopyFileFolder_ManySmallFiles(localSource,s3Dest,v)

%% Init
if (v)
    disp('Initializing');
end
awsSetCredentials(1);
ec2RunStructure = My_ec2RunStructure(); 

[~,folderName] = fileparts([localSource '.txt']); %Get the file path of the folder

%% Tar
if exist('C:\Program Files\7-Zip\','dir')
    sevenZipFolder = 'C:\Program Files\7-Zip\';
elseif exist('C:\Program Files (x86)\7-Zip\','dir')
    sevenZipFolder = 'C:\Program Files (x86)\7-Zip\';
else
    error('Please Install 7-Zip');
end

if (v)
    fprintf('Tarring... ');
    tic;
end
[status,txt] = system(sprintf('"%s7z.exe" a -ttar "%s" "%s"',sevenZipFolder,'tmp.tar',localSource));
if (status ~= 0)
    error('Tar error: %s',txt);
end
if (v)
    fprintf('Total Tar: %.1f[min]\n',toc()/60);
end

%% Copy Files to EC2
if (v)
    fprintf('Copying files to EC2... ');
    tic;
end
[instanceId,DNS,TempPEMFilePath] = awsEC2StartInstance(ec2RunStructure,'m4.2xlarge',1,v); %Start EC2 
[status,txt] = awsEC2RunCommandOnInstance (DNS,TempPEMFilePath,...
    'mkdir -p ~/Input'             ... Make a directory
    );
awsEC2UploadDataToInstance(DNS,TempPEMFilePath,'tmp.tar','~/Input'); %Copy
delete('tmp.tar'); %Cleanup
if (v)
    fprintf('Total Copy: %.1f[min]\n',toc()/60);
end

%% Untar
if (v)
    fprintf('Untarring... ');
    tic;
end
[status,txt] = awsEC2RunCommandOnInstance (DNS,TempPEMFilePath,{...
    'mkdir -p ~/Output'             ... Make a directory
    'cd Input'                      ... Move to input directory
    'tar -xvf tmp.tar -C ~/Output'  ... Untar
    });
if (status ~= 0)
    awsEC2TerminateInstance(instanceId,TempPEMFilePath);%Terminate
    error('Untar error: %s',txt);
end
if (v)
    fprintf('Total Untar: %.1f[min]\n',toc()/60);
end

%% Sync with S3
if (v)
    fprintf('Uploading EC2 data to S3... ');
    tic;
end
s3Dest = awsModifyPathForCompetability (s3Dest,true);
[status,txt] = awsEC2RunCommandOnInstance (DNS,TempPEMFilePath,{...
    ['aws s3 sync ~/Output/' folderName ' ''' s3Dest ''''] ... Go Inside the folder that was created by tar such that the sync will not change the name
    });
if (status ~= 0)
    awsEC2TerminateInstance(instanceId,TempPEMFilePath);%Terminate
    error('Sync with S3 error: %s',txt);
end
if (v)
    fprintf('Total upload time: %.1f[min]\n',toc()/60);
end

%% Done
awsEC2TerminateInstance(instanceId,TempPEMFilePath);%Terminate

if (v)
    disp('Done');
end

