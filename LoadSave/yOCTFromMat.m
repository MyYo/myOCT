function data = yOCTFromMat (filepath)
%This function loads a mat file if it is in the cloud or on local
%directory. Opposit of yOCT2Mat

%% Do we need AWS?
if (awsIsAWSPath(filepath))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials; %Use the advanced version as uploading is more challenging
    filepath = awsModifyPathForCompetability(filepath);
else
    isAWS = false;
end

%% Read
ds=fileDatastore(filepath,'ReadFcn',@load);
out = ds.read();

data = out.data;