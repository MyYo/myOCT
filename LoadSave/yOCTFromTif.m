function [data, metadata, c] = yOCTFromTif (varargin)
% This function tiff file.
% USAGE: 
%   [data, metadata, clim] = yOCTFromTif (filepath [, parameters])
% INPUTS:
%   filpath - filepath to load. Can be a .tif file or a tif stack folder.
%       For each file in the tif stack, image dimensions are z-x.
%       Progressing along the stack is like moving along y axis.
%       Path can be local or s3 path.
% PARAMETERS:
%   'xI','yI','zI' - specify which index of area of the data to load. 
%       For example 'yI',1:10 will load first 10 frames. Default: load all.
%       Notice that if you would like par
%   'isLoadMetadataOnly' - when set to true will set data to [] and return
%       metadata only. Default: false.
% OUTPUTS:
%   data - data saved from tif, dimensions are (z,x,y)
%   metaData - dimention structure, if present as meta data
%   c - limits used to create the file

%% Input Processing

p = inputParser;
addRequired(p,'filepath',@ischar);
addParameter(p,'xI',[])
addParameter(p,'yI',[]);
addParameter(p,'zI',[]);
addParameter(p,'isLoadMetadataOnly',false);

parse(p,varargin{:});
in = p.Results;

filepath = in.filepath;
yI = in.yI;
xI = in.xI;
zI = in.zI;

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

%% Read Metadata
if (isInputFile)
    % Read meta from file
    info = imfinfo(filepath);

    % Get meta data
    if isfield(info(1),'ImageDescription')
        description = info(1).ImageDescription;
    else
        description = '';
    end
    [c, metadata, maxbit] = intrpertDescription(description,filepath);
    
    % Get yI dimensions
    if isempty(yI)
        yI=1:length(info);
    end
else
    % Read meta from JSON
    description = awsReadJSON([filepath '/TifMetadata.json']);
    
    [c, metadata, maxbit] = intrpertDescription(description,filepath);
    
    % Get dimensions
    if isempty(yI)
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

if xor(isempty(xI),isempty(zI))
    error('If xI is defined than zI should be defined as well');
end
if ~isempty(xI)
    if (length(xI) == 1)
        xI = [xI xI];
    end
    if (length(zI) == 1)
        zI = [zI zI];
    end
    if (std(diff(xI))~=0 || std(diff(zI))~=0)
        error('xI, zI should be of the form start:jump:end');
    end

    pixRegionX = [xI(1) max(mean(diff(xI)),1) xI(end)];
    pixRegionY = [zI(1) max(mean(diff(zI)),1) zI(end)];
    imreadWrapper1 = @(filePath, frameIndex)(imreadWrapper(filePath, frameIndex, pixRegionX, pixRegionY));
else
    % Load all
    imreadWrapper1 = @(filePath, frameIndex)(imreadWrapper(filePath, frameIndex, [], []));
end

%% If metadata only mode, we are done
if in.isLoadMetadataOnly
    data = [];
    
    if isAWS && isInputFile
        %Remove temporary file
        delete(filepath);
    end
    return;
end

%% Get the data
for i=1:length(yI)
    
    %% Load data
    try
        if (isInputFile)
            % Single file mode
            bits = imreadWrapper1(filepath,yI(i));
        else
            % Folder mode
            ds = fileDatastore(...
                awsModifyPathForCompetability(sprintf('%s/y%04d.tif',filepath,yI(i))), ...
                'ReadFcn',@(fp)(imreadWrapper1(fp,[])));
            bits = ds.read();
        end
    catch ME
        if isInputFile
            s = 'from a tif file';
        else
            s = 'from a tif folder';
        end
        fprintf('yOCTFromTif failed to load an image %s.\nPath: "%s".\nslice %d.\n',...
            s,filepath,yI(i)); 
        for j=1:length(ME.stack) 
            ME.stack(j) 
        end 
        disp(ME.message); 
        error('Error in yOCTFrimTif, see information below');
    end

    %% Basic processing
    if (i==1)
        data = zeros(size(bits,1),size(bits,2),length(yI),'single');
    end
    
    data(:,:,i) = yOCT2Tif_ConvertBitsData(bits,c,true,maxbit); %Rescale to the original values
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
    maxbit = 2^8-1;
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
        maxbit = []; %Latest version
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

function im = imreadWrapper(imagePath, frameIndex, pixRegionX, pixRegionY)

if isempty(frameIndex) && isempty(pixRegionX)
    %Simplest version of load
    im = imread(imagePath);
elseif ~isempty(frameIndex) && isempty(pixRegionX)
    %No pixel regions
    im = imread(imagePath,'index',frameIndex);
elseif isempty(frameIndex) && ~isempty(pixRegionX)
    %No frame index
    im = imread(imagePath,'PixelRegion',{pixRegionY pixRegionX});
else
    im = imread(imagePath,'index',frameIndex,'PixelRegion',{pixRegionY pixRegionX});
end