function yOCT2Mat(data,filepath,dim)
%This function saves data to mat file (local or in cloud)
%INPUTS
%   filepath - filepath of output mat file
%   data - OCT Volume or B scan
%   dim - dimention structure, to be saved along, if needed

%% Do we need AWS?
if (awsIsAWSPath(filepath))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials(1); %Use the advanced version as uploading is more challenging
    awsFilePath = filepath;
    awsFilePath = awsModifyPathForCompetability(awsFilePath,true); %We will use this path for AWS CLI
    filepath = [tempname '.mat'];
else
    isAWS = false;
end

if ~exist('dim','var')
    dim = [];
end

save(filepath,'data','dim');

%% Upload file to cloud if required
if (isAWS)
    awsCopyFileFolder(filepath,awsFilePath);
    delete(filepath); %Cleanup
end