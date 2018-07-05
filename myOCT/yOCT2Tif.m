function yOCT2Tif (scanAbs, filpath, c)
%This function saves a grayscale version of scanAbs to a Tiff stack file.
%Dimensions are (z,x) and each frame is y
%INPUTS
%   filpath - filepath of output tif file
%   scanAbs - scan data (dimensions are (z,x,y)
%   c - [min, max] of the grayscale

if ~exist('c','var')
    c = [min(scanAbs(:)), max(scanAbs(:))];
end

d = sprintf('min:%.5g,max:%.5g',c(1),c(2));
for yi=1:size(scanAbs,3)
    color = uint8( (squeeze(scanAbs(:,:,yi))-c(1))/(c(2)-c(1))*255);
    color(color>255) = 255;
    color(color<0) = 0;
    if (yi==1)
        imwrite(color,filpath,...
            'Description',d ... Description contains min & max values
            );
    else
        imwrite(color,filpath,...
            'writeMode','append');     
    end
end