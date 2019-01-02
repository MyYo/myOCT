function data = yOCTFromMat (filepath)
%This function loads a mat file if it is in the cloud or on local
%directory. Opposit of yOCT2Mat

%% Do we need AWS?
if (strcmpi(filepath(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    yOCTSetAWScredentials; %Use the advanced version as uploading is more challenging
    filepath = myOCTModifyPathForAWSCompetability(filepath);
else
    isAWS = false;
end

%% Read
ds=fileDatastore(filepath,'ReadFcn',@load);
out = ds.read();

data = out.data;