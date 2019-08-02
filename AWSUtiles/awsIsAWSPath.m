function tf = awsIsAWSPath(filePath)
%This function returns true if filePath contains an AWS stream

if (strcmpi(filePath(1:3),'s3:'))
    tf = true;
else
    tf = false;
end