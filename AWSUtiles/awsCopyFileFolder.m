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
        d = dir([baseLibrary '\**\*.*']);
        if (length(d)>10)
            mode = 'UploadDirManySmallFiles';
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
    otherwise
        error('Couldn''t figure out the mode of operation');
end

if err~=0
    error(['error happend while using aws: ']);
end