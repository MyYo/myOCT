function [data,dim] = yOCTFromMat (filepath)
%This function loads a mat file if it is in the cloud or on local
%directory. Opposit of yOCT2Mat
%OUTPUTS:
%   data - data saved from tif
%   dim - dimention structure, if present as meta data

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

%% Set meta data
if isfield(out,'dim')
    dim = out.dim;
else
    dim = [];
end