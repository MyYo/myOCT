function im = awsimread (impath)
% This function is aws equivalent of imread. reads an image using datastore
% locally or from the cloud.

if awsIsAWSPath(impath)
    awsSetCredentials(); % Read credentials are required
end

% Any fileDatastore request to AWS S3 is limited to 1000 files in 
% MATLAB 2021a. Due to this bug, we have replaced all calls to 
% fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
% 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
ds = fileDatastore(impath,'ReadFcn',@imread);
im = ds.read();
