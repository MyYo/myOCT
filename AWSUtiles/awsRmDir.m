function awsRmDir(myDir)
%This function will remove dir if it exists (either in AWS or locally)

if isempty(myDir)
    error('Cannot delete an empty dir');
end

if ~awsIsAWSPath(myDir)
    myDir = awsModifyPathForCompetability(myDir);
    if exist(myDir,'dir')
        rmdir(myDir,'s');
    end
else
    awsSetCredentials(1);
    myDir = awsModifyPathForCompetability(myDir,true);
    awsCmd(['aws s3 rm "' myDir '" --recursive']);
end