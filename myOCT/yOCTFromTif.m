function scanAbs = yOCTFromTif (filpath)
%This function loads a grayscale version of scanAbs from a Tiff stack file.
%Dimensions are (z,x,y)
%INPUTS
%   filpath - filepath of output tif file (stack is z,x and each frame is y)
%   scanAbs - scan data (dimensions are (z,x,y)

info = imfinfo(filpath);
sizeY = length(info);
sizeX = info(1).Width;
sizeZ = info(1).Height;

scanAbs = zeros(sizeZ,sizeX,sizeY);

c = sscanf(info(1).ImageDescription,'min:%g,max:%g');
if isempty(c) || length(c)~=2
    %No Scaling information, use default
    c(1) = 255;
    c(2) = 0;
end

for yi=1:size(scanAbs,3)
    scanAbs(:,:,yi) = double(imread(filpath,'index',yi))*c(2)/255+c(1); %Rescale to the original values
end