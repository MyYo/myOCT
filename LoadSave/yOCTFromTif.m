function scanAbs = yOCTFromTif (filepath, yI)
%This function loads a grayscale version of scanAbs from a Tiff stack file.
%Dimensions are (z,x,y)
%INPUTS
%   filpath - filepath of output tif file (stack is z,x and each frame is y)
%   yI - Optional, which y frames to load

if (strcmpi(filepath(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    yOCTSetAWScredentials; %Use the advanced version as uploading is more challenging
    filepath = myOCTModifyPathForAWSCompetability(filepath);
    
    %Download file locally for easy access
    ds=fileDatastore(filepath,'ReadFcn',@readfile);
    filepath=ds.read();
else
    isAWS = false;
end

info = imfinfo(filepath);
if (~exist('yI','var'))
    yI=1:length(info);
end
    
sizeY = length(yI);
sizeX = info(1).Width;
sizeZ = info(1).Height;

scanAbs = zeros(sizeZ,sizeX,sizeY,'single');

c = sscanf(info(1).ImageDescription,'min:%g,max:%g');
if isempty(c) || length(c)~=2
    %No Scaling information, use default
    c(1) = 255;
    c(2) = 0;
end

for i=1:size(scanAbs,3)
    dat = imread(filepath,'index',yI(i));
    scanAbs(:,:,i) = double(dat)*(c(2)-c(1))/255+c(1); %Rescale to the original values
end

if isAWS
    %Remove temporary file
    delete(filepath);
end

function out = readfile(filepath)
%Copy filename to other temp name
out = [tempname '.tif'];
copyfile(filepath,out);