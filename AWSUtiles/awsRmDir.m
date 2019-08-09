function awsRmDir(myDir)
%This function will remove dir if it exists (either in AWS or locally)

if ~awsIsAWSPath(myDir)
    if exist(myDir,'dir')
        rmdir(myDir,'s');
    end
else
    awsSetCredentials(1);
    myDir = awsModifyPathForCompetability(myDir,true);
    [~,~] = system(['aws s3 rm "' myDir '" --recursive']);
end