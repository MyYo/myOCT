function j = awsReadJSON(fp)
%This function reads a JSON file from AWS or locally

if (awsIsAWSPath(fp))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials;
    fp = awsModifyPathForCompetability(fp,false);
else
    fp = awsModifyPathForCompetability(fp);
    isAWS = false;
end

%Load JSON to get scan configuration and different depths
% Any fileDatastore request to AWS S3 is limited to 1000 files in 
% MATLAB 2021a. Due to this bug, we have replaced all calls to 
% fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
% 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
ds = fileDatastore(fp,'ReadFcn',@readJSON,'FileExtensions','.json');
j = ds.read();

function o = readJSON(filename)
txt=fileread(filename);
o = jsondecode(txt);