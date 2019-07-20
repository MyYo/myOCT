function yOCT2Mat(data,filepath)
%This function saves data to mat file (local or in cloud)
%INPUTS
%   filepath - filepath of output mat file
%   data - OCT Volume or B scan

%% Do we need AWS?
if (strcmpi(filepath(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials(1); %Use the advanced version as uploading is more challenging
    awsFilePath = filepath;
    awsFilePath = awsModifyPathForCompetability(awsFilePath,true); %We will use this path for AWS CLI
    filepath = [tempname '.mat'];
else
    isAWS = false;
end

save(filepath,'data');

%% Upload file to cloud if required
if (isAWS)
    [~,~] = system(['aws s3 cp "' filepath '" "' awsFilePath '"']);
    delete(filepath); %Cleanup
end