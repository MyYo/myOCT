function [reslicedVolume,xyzNew2Original,dimensions_n] = yOCTReslice(varargin)
% This function reslices a 3D volume along n direction
% USAGE
%   [reslicedVolume,xyzNew2Original] = yOCTReslice(volume, xyzNew2Original, 
%           x1_n, y1_n, z1_n, [ ,'parameterName', value, ...])
% INPUTS:
%   - volume - OCT volume to reslice, can be:
%       + 3D matrix, dimensions (z,x,y)
%       + Path to a tif file, saved with yOCT2Tif. This option
%           is memory intensive, its better to use the folder option below.
%       + Path to a folder with multiple tif files, each representing a y
%           slice, also known as tif stack (more info at help yOCT2Tif)
%   - xyzNew2Original defining how to convert x' y' z' coordinates from new
%       coordinate system to original aka volume's coordinate system. 
%       Can be:
%       + 3*1 vector defining the normal to x'-z' plane, than xyzNew2Original
%           is a rotation matrix, x' will be parallel to x-y plane
%       + function handle - see xyzNew2Original at the output section.
%   - x1_n, y1_n, z1_n - positions in new coordinate system in mm
% OPTIONAL PARAMETERS:
%   - dimensions - can be:
%       + dimensions structure as output by yOCTInterfToScanCpx.
%       + a path to json file containing dimensions structure
%       + empty or not specified, will try to retreave dimensions from meta
%           data associated with volume file
%   - outputFileOrFolder - if specified, will not return reslicedVolume
%       but would save it directly to hard drive (or s3). Use this option
%       if running on the cloud as it will save the data transfer back and
%       forth, and would output data directly to the cloud. outputFileOrFolder
%       can be a path to a Tif file, or TifStack folder (see yOCT2Tif)
%
% OUTPUTS:
%   - reslicedVolume - 3D volume of the resliced volume.
%       dimensions are (z',x',y'), y' is along n direction, x' is parallel 
%           to x-y plane, y' is right complementary.
%       If outputFolder is set, then reslicedVolume will be [].
%       See outputFolder above for explenation.
%   - xyzNew2Original - function handle that tells how to convert new x'y'z'
%       to original / volume xyz coordinates. Usage: 
%           xyz = xyzNew2Original(x',y',z'); x = xyz(1,:); y = xyz(2,:); .. 
%   - dimensions_n - dimensions structure for output volume

%% Input Processing
p = inputParser;
addRequired(p,'volume');
addRequired(p,'xyzNew2Original');
addRequired(p,'x1_n',@(w)(all(w==sort(w))));
addRequired(p,'y1_n',@(w)(all(w==sort(w))));
addRequired(p,'z1_n',@(w)(all(w==sort(w))));

addParameter(p,'dimensions',[])
addParameter(p,'outputFileOrFolder','');

parse(p,varargin{:});
in = p.Results;

% Figure out input & dimensions
volume = in.volume;
dimensions = in.dimensions;
if isnumeric(volume)
    if isempty(dimensions)
        error('If inputing volume as 3D matrix, make sure to provide dimensions, we can''t tel what they are');
    end
end

if isempty(dimensions)
   [~,dimensions] = yOCTFromTif(volume,[]);
end
dimensions = yOCTChangeDimensionsStructureUnits(dimensions,'mm');

% outputFolder
outputFileOrFolder = in.outputFileOrFolder;
if isempty(outputFileOrFolder)
    % User would like to receive output directly
    returnReslicedVolume = true; 
    outputFileOrFolder = awsModifyPathForCompetability([tempname '\']);
else
    % Save output to folder, don't return it
        
    % Make sure output folder is a cell
    if ~iscell(outputFileOrFolder)
        outputFileOrFolder = {outputFileOrFolder};
    end
    
    % Check that folder doesn't exist
    for i=1:length(outputFileOrFolder)
        if awsExist(outputFileOrFolder{i},'dir')
            error('Output folder must not exist for this function to run properly: %s',outputFileOrFolder{i});
        end
    end
    
    returnReslicedVolume = false; 
end

% Coordinate systems
x1_n = in.x1_n;
y1_n = in.y1_n;
z1_n = in.z1_n;

%% Coordinate system conversion parameters

if (isa(in.xyzNew2Original,'function_handle'))
    % We got an explicit function handle.
    xyzNew2Original = in.xyzNew2Original;
elseif isnumeric(in.xyzNew2Original) && numel(in.xyzNew2Original) == 3
    rectifyVect = @(v)(v(:)/norm(v(:)));
    
    % we got a normal vector.
    n = rectifyVect(in.xyzNew2Original);
    
    % Y direction points towards n.
    my = n;
   
    % X direction parallel to x-y plane
    mx = cross([0;0;-1],n); 
    mx = rectifyVect(mx);
    
    % Z is right hand complement
    mz = cross(mx,my);
    mz = rectifyVect(mz);
    
    % Rotation matrix
    M_n2o = [mx my mz];
    
    xyzNew2Original = @(x,y,z)(M_n2o*[x(:)' ; y(:)' ; z(:)']);
else 
    error('can''t parse xyzNew2Original');
end

%% Generate new dimensions in new reference frame (for saving)
dimensions_n.z.order = 1;
dimensions_n.z.values = z1_n;
dimensions_n.z.index = 1:length(z1_n);
dimensions_n.z.units = 'mm';
dimensions_n.x.order = 2;
dimensions_n.x.values = x1_n;
dimensions_n.x.index = 1:length(x1_n);
dimensions_n.x.units = 'mm';
dimensions_n.y.order = 3;
dimensions_n.y.values = y1_n;
dimensions_n.y.index = 1:length(y1_n);
dimensions_n.y.units = 'mm';

%% Compute output volume
yOCT2Tif([],outputFileOrFolder,'partialFileMode',1);
parfor yi1_n=1:length(y1_n) % Each for acts on one output y plane
    
    % Create grid of the new coordinates (x-z)
    [xx1_n,zz1_n] = meshgrid(x1_n,z1_n);
    
    % Convert new volume's coordinate to original coordinates
    xyz1_o = xyzNew2Original(...
        xx1_n, ...
        y1_n(yi1_n)*ones(size(xx1_n)), ...
        zz1_n);
    x1_o = reshape(xyz1_o(1,:),size(xx1_n));
    y1_o = reshape(xyz1_o(2,:),size(xx1_n));
    z1_o = reshape(xyz1_o(3,:),size(xx1_n));
    
    % Load & compute slice
    slice = yOCTReslice_Slice(volume, dimensions, x1_o, y1_o, z1_o);
    
    % Save plane to folder 
    yOCT2Tif(slice, outputFileOrFolder,...
        'partialFileMode',2,'partialFileModeIndex',yi1_n);
end
yOCT2Tif([],outputFileOrFolder,'partialFileMode',3,'metadata',dimensions_n);

%% Cleanup & Finish
if returnReslicedVolume
    reslicedVolume = yOCTFromTif(outputFileOrFolder);
    awsRmDir(outputFileOrFolder);
else
    reslicedVolume = [];
end