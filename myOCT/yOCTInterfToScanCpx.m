function scanCpx = yOCTInterfToScanCpx (varargin)
%This function takes the interferogram loaded from yOCTLoadInterfFromFile
%and converts it to a complex scanCpx datastructure
%
%USAGE:
%       scanCpx = yOCTInterfToBScanCpx (interferogram,dimensions [,param1,value1,...])
%INPUTS
%   - interferogram - as loaded by yOCTLoadInterfFromFile. dimensions
%       should be (lambda,x,...)
%   - dimensions - Dimensions structure as loaded by yOCTLoadInterfFromFile.
%   - Optional Parameters
%       - 'dispersionParameterA', quadradic phase correction units of
%          [nm^2/rad]
%       - 'band',[start end] - Use a Hann filter to filter out part of the
%          spectrum. Units are [nm] 
%OUTPUT
%   BScanCpx - where lambda dimension is replaced by z
%
%Author: Yonatan W (Dec 27, 2017)

%% Hendle Inputs
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

interferogram = varargin{1};
dimensions = varargin{2};

%Optional Parameters
dispersionParameterA = [];
band = [];
for i=3:2:length(varargin)
   eval([varargin{i} ' = varargin{i+1};']); %<-TBD - there should be a safer way
end

%% Check if interferogram is equispaced. If not, equispace it before processing
lambda = dimensions.lambda.values;
k = 2*pi./(lambda); %Get wave lumber in [1/nm]

if (abs((max(diff(k)) - min(diff(k)))/max(k)) > 1e-10)
    %Not equispaced, equispacing needed
    [interferogram,dimensions] = yOCTEquispaceInterf(interferogram,dimensions);
    
    lambda = dimensions.lambda.values;
    k = 2*pi./(lambda); %Get wave lumber in [1/nm]
end
s = size(interferogram);

%% Filter bands
filter = zeros(size(k));
filter = filter(:);
if ~isempty(band)
    %Band filter to select sub band
    bandStartI = find(dimensions.lambda.values >= band(1),1,'first');
    bandEndI = find(dimensions.lambda.values <= band(2),1,'last');
    i = bandStartI:bandEndI;
    filter(i) = hann(length(i));
    filter = filter * (length(filter)/sum(filter)); %Normalization
else
    %No band filter, so apply Hann filter on the entire sample
    filter = hann(length(filter));
end

%% Reshape interferogram for easy parallelization
interf = reshape(interferogram,s(1),[]);

%% Dispersion & Overall filter
if ~exist('dispersionParameterA','var') || isempty(dispersionParameterA)
    disp('Please Enter dispersionParameterA (quadratic correction), recomendations [nm^2/rad]:')
    disp('100 is a good value to start from');
    disp('You can also try running Demo_DispersionCorrection, to figure out the best Value for you');
    return;
end
dispersionComp = exp(1i*(dispersionParameterA*k(:).^2)).*hann(length(k));

filter = repmat(dispersionComp.*filter,[1 size(interf,2)]);

%% Generate Cpx 
ft = ifft((interf.*filter));
scanCpx = ft(1:(size(interf,1)/2),:);

%% Reshape back
scanCpx = reshape(scanCpx,[size(scanCpx,1) s(2:end)]);