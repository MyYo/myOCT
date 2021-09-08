function [scanAbs,Pixel] = yOCTFromDicom (filepath, yI)
%This function loads a grayscale version of scanAbs from a Dicom stack file.
%Dimensions are (z,x,y)
%INPUTS
%   filpath - filepath of output tif file (stack is z,x and each frame is y)
%   yI - Optional, which y frames to load

if awsIsAWSPath(filepath)
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials;
    filepath = awsModifyPathForCompetability(filepath);
    
    %Download file locally for easy access
    % Any fileDatastore request to AWS S3 is limited to 1000 files in 
    % MATLAB 2021a. Due to this bug, we have replaced all calls to 
    % fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
    % 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
    ds=imageDatastore(filepath,'ReadFcn',@readfile);
    filepath=ds.read();
else
    isAWS = false;
end

info = dicominfo(filepath);
Pixel.z=info.PixelSpacing(1);
Pixel.x=info.PixelSpacing(2);
Pixel.y=info.SpacingBetweenSlices;

if (~exist('yI','var'))
    yI=1:info.NumberOfFrames;
end
    
sizeY = length(yI);
sizeX = info.Columns;
sizeZ = info.Rows;

scanAbs = zeros(sizeZ,sizeX,sizeY,'single');

c = info.EchoTime;
if isempty(c) || length(c)~=2
    %No Scaling information, use default Dicom scaling  0-65535
     c(1) = 65535;
     c(2) = 0;
end

dat = dicomread(filepath);
for i=1:size(scanAbs,3)
    scanAbs(:,:,i) = double(dat(:,:,:,i))*((c(2)-c(1))/65535)+c(1); %Rescale to the original values
end

if isAWS
    %Remove temporary file
    delete(filepath);
end

end

function out = readfile(filepath)
%Copy filename to other temp name
out = [tempname '.tif'];
copyfile(filepath,out);
end
