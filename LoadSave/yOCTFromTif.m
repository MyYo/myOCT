function [data, metaData, c] = yOCTFromTif (filepath, yI)
%This function loads a grayscale version of scanAbs from a Tiff stack file.
%Dimensions are (z,x,y)
%INPUTS
%   filpath - filepath of output tif file (stack is z,x and each frame is y)
%   yI - Optional, which y frames to load
%OUTPUTS:
%   data - data saved from tif
%   metaData - dimention structure, if present as meta data
%   c - limits used to create the file

%% Is AWS?
if (awsIsAWSPath(filepath))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials; %Use the advanced version as uploading is more challenging
    filepath = awsModifyPathForCompetability(filepath);
else
    isAWS = false;
end

%% Is Tif Folder or File
[~,~,f] = fileparts(filepath);
    
if (~isempty(f))
    isInputFile = true;
else
    isInputFile = false;
end

%% Copy file locally, if its at the cloud
if (isAWS && isInputFile)
    % Download file locally for easy access
    ds=fileDatastore(filepath,'ReadFcn',@copyFileLocally);
    filepath=ds.read(); % Update file path
end

%% Read Meta data
if (isInputFile)
    % Read meta from file
    info = imfinfo(filepath);

    % Get meta data
    if isfield(info(1),'ImageDescription')
        description = info(1).ImageDescription;
    else
        description = '';
    end
    [c, metaData, maxbit] = intrpertDescription(description,filepath);
    
    % Get dimensions
    if (~exist('yI','var'))
        yI=1:length(info);
    end
else
    % Read meta from JSON
    description = awsReadJSON([filepath '/TifMetadata.json']);
    
    [c, metaData, maxbit] = intrpertDescription(description,filepath);
    
    % Get dimensions
    if (~exist('yI','var'))
        l = awsls(filepath);
        isTifFile = cellfun(@(x)(contains(x,'.tif')),l);
        yI=1:sum(isTifFile);
    end
end 

%No Scaling information, use default
if isempty(c) || length(c)~=2
    c(1) = maxbit;
    c(2) = 0;
end

%% Get the data

for i=1:length(yI)

    if (isInputFile)
        % Single file mode
        dat = imread(filepath,'index',yI(i));
    else
        % Folder mode
        ds = fileDatastore(...
            awsModifyPathForCompetability(sprintf('%s/y%04d.tif',filepath,yI(i))), ...
            'ReadFcn',@imread);
        dat = ds.read();
    end

    if (i==1)
        data = zeros(size(dat,1),size(dat,2),length(yI),'single');
    end
    data(:,:,i) = double(dat)*(c(2)-c(1))/maxbit+c(1); %Rescale to the original values
end

if isAWS && isInputFile
    %Remove temporary file
    delete(filepath);
end   
    
function out = copyFileLocally(filepath)
%Copy filename to other temp name
out = [tempname '.tif'];
copyfile(filepath,out);

function [c, metaData, maxbit] = intrpertDescription(description,filepath)

if isempty(description)
    c = [];
    metaData = [];
    return;
end

%% Read version
isDepricatedVersion = false;
if (~isstruct(description) && description(1) ~= '{')
    % Support for version 1, depriciated!
    c = sscanf(description,'min:%g,max:%g');
    isDepricatedVersion = true;
    metaData = [];
    maxbit = 2^8-1;
else
    if ~isstruct(description)
        jsn = jsondecode(description);
    else
        jsn = description;
    end
    
    if (jsn.version == 2)
        metaData = jsn.dim;
        c = jsn.c;
        isDepricatedVersion = true;
        maxbit = 2^8-1;
    elseif (jsn.version == 3)
        % Good version
        metaData = jsn.metadata;
        c = jsn.clim;
        maxbit = 2^16-1;
    end
end

%% Warning if needed
persistent timeOfLastWarningHappend; 
if isDepricatedVersion && (isempty(timeOfLastWarningHappend) || ...
        timeOfLastWarningHappend < now-1/86400) %Last warning is old by n seconds
    warning(['%s has a depriciated version of meta data, ' ...
        'update please by running this command:\n' ...
        'filePath = ''%s'';\n' ...
        '[data, meta] = yOCTFromTif(filePath);\n' ...
        'yOCT2Tif(data, filePath, ''metadata'', meta);'],filepath,filepath);
    
    timeOfLastWarningHappend = now;
end
