function awsRmFile(myFile)
%This function will remove file if it exists, in AWS or Windows

if ~awsIsAWSPath(myFile)
    if exist(myFile,'file')
        delete(myFile);
    end
else
    awsSetCredentials(1);
    myFile = awsModifyPathForCompetability(myFile,true);
    awsCmd(['aws s3 rm "' myFile '"']);
end