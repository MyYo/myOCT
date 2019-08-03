function tf = awsIsAWSPath(filePath)
%This function returns true if filePath contains an AWS stream

if (strncmpi(filePath,'s3:',3))
    tf = true;
else
    tf = false;
end