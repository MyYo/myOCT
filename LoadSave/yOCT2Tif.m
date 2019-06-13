function yOCT2Tif (data, filepath, c)
%This function saves a grayscale version of data to a Tiff stack file.
%Dimensions are (z,x) and each frame is y
%INPUTS
%   filpath - filepath of output tif file
%   data - scan data (dimensions are (z,x,y)
%   c - [min, max] of the grayscale

%% Input check
if ~exist('c','var') || isempty(c)
    c = [prctile(data(:),20), prctile(data(:),99.9)]; %min value is at percentile 20 because most of volume has no signals
end

%% Do we need AWS?
if (strcmpi(filepath(1:3),'s3:'))
    %Load Data from AWS
    isAWS = true;
    yOCTSetAWScredentials(1); %Use the advanced version as uploading is more challenging
    awsFilePath = filepath;
    awsFilePath = myOCTModifyPathForAWSCompetability(awsFilePath,true); %We will use this path for AWS CLI
    filepath = [tempname '.tif'];
else
    isAWS = false;
end

%% Preform writing to the file
d = sprintf('min:%.5g,max:%.5g',c(1),c(2));
for yi=1:size(data,3)
    color = uint8( (squeeze(data(:,:,yi))-c(1))/(c(2)-c(1))*255);
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
