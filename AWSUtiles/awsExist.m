function tf=awsExist(filePath,type)
%Equivalent to Matlab's exist function, but includes aws links.
%filepath - file path to check
%type - can be 'file', or 'dir' to say if file or directory is what you are
%looking for. Notice in AWS configuration, will not deffrinciate between
%both types so you can leave empty.

%% Input checks
if ~exist('type','var')
    type = 'file';
end

if (~awsIsAWSPath(filePath))
    %Not an aws file path, rerun usual exist syntax.
    tf = exist(filePath,type);
else
    awsSetCredentials(1);
    filePath = awsModifyPathForCompetability(filePath,true);
    
    [status,text] = system(['aws s3 ls "' filePath '"']);
    
    if (status == 0)
        tf = true; %Exists
    elseif (status == 1)
        tf = false; %Doen't exist
    else
        error('Error happend while searching for %s, Error: %s',filePath,text);
    end
end

    