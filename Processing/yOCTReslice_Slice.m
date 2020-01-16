function slice = yOCTReslice_Slice(volume, dimensions, x1_o, y1_o, z1_o)
% This is a helper function to yOCTReslice, it slices a single plane out of
% a volume.
%
% USAGE:
%   slice = yOCTReslice(volume, dimensions, x1_o, y1_o, z1_o);
%
% INPUTS: (see yOCTReslice.m for more complete documentation)
%   - volume & dimensions (see yOCTReslice.m)
%   - x1_o, y1_o, z1_o - 1D or 2D array of plane's x-y-z positions at the
%       original / volume coordinate system.
% OUTPUTS:
%   slice - same size as x1_0, data sliced

%% Coordinate system notations for this code
% _o - data is expressed in the original coordinate system
% _n - data is expressed in the new coodinate system
% x0,y0,z0 - coordinate system of the original volume
% x1,y1,z1 - coordinate system of the new volume
% For example:
%   x1_n - is the x coordinate of the new volume expressed in new volume
%       coordinate system
%   x1_o - is the x coordinate of the new volume expressed in original
%       volume coordinate system

% Actual position of volume coordinate system.
x0_o  = single(dimensions.x.values(:)');
y0_o  = single(dimensions.y.values(:)');
z0_o  = single(dimensions.z.values(:)');

% Position (index).
xi0_o = single(1:length(dimensions.x.values));
yi0_o = single(1:length(dimensions.y.values));
zi0_o = single(1:length(dimensions.z.values));

% Actual position of slice  in volum's coordinate system.
matSz = size(x1_o);

% Position in terms of index
xyz2ijk = @(x,y,z)([ ...
    interp1(z0_o(:)', zi0_o(:)', z(:)', 'linear', NaN) ; ...
    interp1(x0_o', xi0_o(:)', x(:)', 'linear', NaN) ; ...
    interp1(y0_o', yi0_o(:)', y(:)', 'linear', NaN) ; ...
    ]);
ijk = xyz2ijk(x1_o, y1_o, z1_o);
xi1_o = single(reshape(ijk(2,:), matSz));
yi1_o = single(reshape(ijk(3,:), matSz));
zi1_o = single(reshape(ijk(1,:), matSz));

%% Define Batch
% We can't process the entire volume in memory, so load a few y planes at a
% time

% Batch defenition
yiJump = 5; %How many y indexes to load at a time (memory management constraint)
pad = 1; % How many extra planes to load for interpolation

yi0_oBatchPositions = unique([ ...
floor(min(yi1_o(:))), ...
floor(min(yi1_o(:))):yiJump:ceil(max(yi1_o(:))), ...
ceil(max(yi1_o(:)))]);
yi0_oBatchPositions(isnan(yi0_oBatchPositions)) = [];

% In case only one position is required, start and finish with that
if (length(yi0_oBatchPositions) == 1)
    yi0_oBatchPositions(2) = yi0_oBatchPositions(1);
end
   
%% Loop over batch, get data
slice = NaN*zeros(matSz);
for batchI = 1:(length(yi0_oBatchPositions)-1)
    betchYi0_o = (yi0_oBatchPositions(batchI)-pad):(yi0_oBatchPositions(batchI+1)+pad);
    %Remove data outside of bounds
    betchYi0_o(betchYi0_o<1) = []; 
    betchYi0_o(betchYi0_o>max(yi0_o)) = [];
    
    % Mask output frame - don't ask to compute values you know are NaN 
    willPointInOutFrameBeComputed = ...
        yi1_o>=yi0_oBatchPositions(batchI) & yi1_o<=yi0_oBatchPositions(batchI+1) & ...
        ~isnan(xi1_o) & ~isnan(zi1_o);
    
    % Mask input, dont load values that known to not be needed
    isXi0Needed = ...
        xi0_o >= (floor(min(xi1_o(willPointInOutFrameBeComputed)))-pad) & ...
        xi0_o <= (ceil(max(xi1_o(willPointInOutFrameBeComputed)))+pad);
    isZi0Needed = ...
        zi0_o >= (floor(min(zi1_o(willPointInOutFrameBeComputed)))-pad) & ...
        zi0_o <= (ceil(max(zi1_o(willPointInOutFrameBeComputed)))+pad);
        
    if ~any(willPointInOutFrameBeComputed(:))
        % No need to use this yi, no data needed here
        continue;
    end

    % Get data in
    if (ischar(volume))
        frameDataIn = yOCTFromTif(volume,'yI',betchYi0_o,...
            'xI',xi0_o(isXi0Needed),'zI',zi0_o(isZi0Needed));
    else
        frameDataIn = volume(...
            zi0_o(isZi0Needed),xi0_o(isXi0Needed),betchYi0_o);
    end
    
    % Prepeare for interpolation, change dimensions and create a meshgrid
    frameDataIn = permute(frameDataIn,[3 2 1]); % Dimensions (y,x,z)
    [xxi0_o,yyi0_o,zzi0_o] = meshgrid(xi0_o(isXi0Needed), ...
                                      betchYi0_o, ...
                                      zi0_o(isZi0Needed));
                                  
    % Interpolate 
    d = interp3(xxi0_o,yyi0_o,zzi0_o,...
        frameDataIn, ...
        xi1_o(willPointInOutFrameBeComputed), ...
        yi1_o(willPointInOutFrameBeComputed), ...
        zi1_o(willPointInOutFrameBeComputed),'linear',NaN);

    slice(willPointInOutFrameBeComputed) = d;
end

