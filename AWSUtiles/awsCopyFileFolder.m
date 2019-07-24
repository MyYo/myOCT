function awsCopyFileFolder(source,dest)
%This function copys files and folders from/aws

awsSetCredentials (1); %Write cridentials are required  
if (strcmpi(source(1:3),'s3:'))
    source = awsModifyPathForCompetability(source,true);
end
if (strcmpi(dest(1:3),'s3:'))
    dest = awsModifyPathForCompetability(dest,true);
end

if (exist(source,'dir'))
    %Make the copy of a folder
    [err,errTxt] = system(['aws s3 cp "' source '" "' dest '" --recursive']);
elseif (exist(source,'file'))
    [err,errTxt] = system(['aws s3 cp "' source '" "' dest '"']);
else
    error(['Source file does not exist:' source]);
end

if err~=0
    error(['error happend while using aws: ' errTxt]);
end