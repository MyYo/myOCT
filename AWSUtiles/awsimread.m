function im = awsimread (impath)
% This function is aws equivalent of imread. reads an image using datastore
% locally or from the cloud.

if awsIsAWSPath(impath)
    awsSetCredentials(); % Read credentials are required
end

ds = fileDatastore(impath,'ReadFcn',@imread);
im = ds.read();
