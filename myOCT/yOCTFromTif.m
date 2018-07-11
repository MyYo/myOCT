function scanAbs = yOCTFromTif (filpath, yI)
%This function loads a grayscale version of scanAbs from a Tiff stack file.
%Dimensions are (z,x,y)
%INPUTS
%   filpath - filepath of output tif file (stack is z,x and each frame is y)
%   yI - Optional, which y frames to load

info = imfinfo(filpath);
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
    scanAbs(:,:,i) = double(imread(filpath,'index',yI(i)))*(c(2)-c(1))/255+c(1); %Rescale to the original values
end