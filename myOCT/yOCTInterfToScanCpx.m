function scanCpx = yOCTInterfToScanCpx (varargin)
%This function takes the interferogram loaded from yOCTLoadInterfFromFile
%and converts it to a complex scanCpx datastructure
%USAGE:
%   scanCpx = yOCTInterfToBScanCpx (interferogram,k_n [,param1,value1,...])
%INPUTS
%   - interferogram - as loaded by yOCTLoadInterfFromFile. dimensions
%       should be (lambda,x,...)
%   - k_n - for each lambda, what is the actual k value. This can be
%       extracted from dimensions.lambda.k_n from yOCTLoadInterfFromFile
%       outputs
%   - Optional Parameters
%       - 'dispersionParameterA',value - linear phase dispersion. 
%           if set to 0 cancels. Default value is water imesed sample value
%       - 'hannFilterK',[start end] - filter out some bands. 
%   - bandI - Optional Support for Hann filter of interferogram prior to extracting
%       of OCT signal (default []). bandI = [start end]. Where start, end are the
%       index of wavelength to start, and end. Since wavelength is not
%       exact conversion to wavelength is only estimated.
%       See yOCTnmtoBandI for information about how to convert from nm to
%       bandI. TBD
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
s = size(interferogram);
k_n = varargin{2};
N = length(k_n);

%Optional Parameters
dispersionParameterA = [];
hannFilterK = [];
for i=3:2:length(varargin)
   eval([varargin{i} ' = varargin{i+1};']); %<-TBD - there should be a safer way
end

%% Filter bands if required

%% Compute parallelization structure to use
%Try to optimize for how many interferogram to compute in one 'shot'. The
%more we compute, the more memory we require and the process becomes slower
nAScans = prod(s(2:end)); %Number of A scans to compute
nRecommendedAScansPerBlock = 10e3; %From experience what is the maximum number of A-Scans per block

nBlocksOptimal = nAScans/nRecommendedAScansPerBlock;

%Number of blocks will be the the closests to the optimum
dv = cumprod(factor(nAScans)); %Some of the divisabls of nAScans
NBlocks = dv(find(abs(dv-nBlocksOptimal)==min(abs(dv-nBlocksOptimal)),1,'first'));
if false
    NBlocks = 1; %Ignore calculation
end  
BlockII = reshape(1:nAScans,[],NBlocks);

%% Reshape interferogram for easy parallelization
interf = reshape(interferogram,s(1),[]);

%% Generate Cpx
%Notice that k_n is non unifrom therefore we should use non uniform fourier
%transform. We shall use Non Uniform DFT. In theory NU-FFT should work faster as described
%here:
%https://statweb.stanford.edu/~candes/math262/Lectures/Lecture11.pdf
%However runtime comparison did show advantage for matrix multiplication
%over interpolation (longest) and fft. If in the future k_n is uniform, it is
%recomended to go back to fft

% Dispersion
if ~exist('a2','var') || isempty(dispersionParameterA)
    dispersionParameterA=0.0058; %Air-water parameter
end
dispersionComp = exp(1i*(dispersionParameterA*k_n(:)'.^2/N))';
%dispersionComp = repmat(dispersionComp,[1 size(interf,2)]);
dispersionComp = repmat(dispersionComp,[1 size(BlockII,1)]);

%Make grid for DFT
m = 0:N-1;
[K, M] = meshgrid(k_n,m(1:end/2));
%DFT Matrix
DFTM = exp(2*pi*1i.*M.*K/N);

%DFT in blocks
%BScanCpx = TempFFTM*(interf.*Phase); 
scanCpx = zeros(size(DFTM,1),size(interf,2));
for i=1:size(BlockII,2)
    scanCpx(:,BlockII(:,i)) = DFTM*(interf(:,BlockII(:,i)).*dispersionComp); 
end

%% Reshape back
scanCpx = reshape(scanCpx,[size(scanCpx,1) s(2:end)]);