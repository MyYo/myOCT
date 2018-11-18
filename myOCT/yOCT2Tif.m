function yOCT2Tif (scanAbs, filepath, c)
%This function saves a grayscale version of scanAbs to a Tiff stack file.
%Dimensions are (z,x) and each frame is y
%INPUTS
%   filpath - filepath of output tif file
%   scanAbs - scan data (dimensions are (z,x,y)
%   c - [min, max] of the grayscale

%% Input check
if ~exist('c','var')
    c = [min(scanAbs(:)), max(scanAbs(:))];
end

%% Do we need AWS?
if (strcmpi(filepath(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    yOCTSetAWScredentials(1); %Use the advanced version as uploading is more challenging
    awsFilePath = filepath;
    filepath = [tempname '.tif'];
else
    isAWS = false;
end

%% Preform writing to the file
d = sprintf('min:%.5g,max:%.5g',c(1),c(2));
for yi=1:size(scanAbs,3)
    color = uint8( (squeeze(scanAbs(:,:,yi))-c(1))/(c(2)-c(1))*255);
    color(color>255) = 255;
    color(color<0) = 0;
    if (yi==1)
        imwrite(color,filepath,...
            'Description',d ... Description contains min & max values
            );
    else
        imwrite(color,filepath,...
            'writeMode','append');     
    end
end

%% Upload file to cloud if required
if (isAWS)
    [~,~] = system(['aws s3 cp "' filepath '" "' awsFilePath '"']);
    delete(filepath); %Cleanup
end