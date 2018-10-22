function [interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions,interpMethod)
%In most cases, the interferogram recorded by OCT system is not equispaced
%in k. Therefore yOCTInterfToScanCpx has to preform DFT which is slower
%then FFT (run speed of n^2 instead of n*log n. yOCTEquispaceInterf
%interpolates data such that interf is equispaced in k allowing for fast
%execution of yOCTInterfToScanCpx. 
%The down side of this function is that the run time of the intrepolation +
%FFT is slower then just doing DFT. So if you intend on using
%"yOCTInterfToScanCpx" just once. Don't use this equuspace step. But if you
%intend on running "yOCTInterfToScanCpx" multiple times (spectral analysis
%or finding dispersion parameter), using this function will save time.
%
%USAGE:
%   [interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions [, interpMethod])
%
% INPUTS:
%   - interf - interferogram as loaded using yOCTLoadIntefFromFile
%   - dimensions - dimensions structure as loaded using yOCTLoadIntefFromFile
%   - interpMethod - what method to use for interpolation of the image.
%       Can be: 'pchip' for interp1 pchip or 'sincX' (replace X with number of samples) for sinc interpolation.
%       Default: 'pchip'. Use, 'sinc20' for high quality (but longer computation time)
% OUTPUTS:
%   - interf - equispaced interferogram
%   - dimensions - dimensions structure corrected to acount for equispacing

%% Input
if ~exist('interpMethod','var')
    interpMethod = 'pchip';
end

%% Data Structure
s = size(interf);
lambda = dimensions.lambda.values(:);

k = 2*pi./(lambda); %Get wave lumber in [1/nm]
kLin = linspace(max(k),min(k),length(k))'; %Linear

%% Interferogram

%Reshape interferogram for easy parallelization
interf = reshape(interf,s(1),[]);

%Interpolate
interfe = myInterp(k,interf,kLin,interpMethod);

%Reshape back
interfe = reshape(interfe,[size(interfe,1) s(2:end)]);

%% Dimensions
dimensionse = dimensions;
dimensionse.lambda.values = (myInterp(k,lambda,kLin,'pchip'))'; %'pchip' interpolation is sufficent for lambda

function out = myInterp(x,v,xq,interpMethod)

if (strcmpi(interpMethod,'pchip'))
    out = interp1(x,v,xq,'pchip');
elseif (strcmpi(interpMethod(1:4),'sinc'))
    
    %Sinc Interpolation
    
    %Convert to Samples
    nSamples = str2double(interpMethod(5:end));
    nq = (xq-min(xq))/(max(xq)-min(xq))*(length(xq)-1);
    n = (x-min(x))/(max(x)-min(x))*(length(x)-1);
    
    %Measure mean
    mv = mean(v,1);
    
    %Preform interpolation
    out = zeros(length(xq),size(v,2));
    for i = 1:length(xq)
         
        filt = sinc(nq(i)-n);
        ii = abs(nq(i)-n) < nSamples; %Samples to use
       
        out(i,:) = sum((v(ii,:) - mv).*repmat(filt(ii),[1 size(v,2)]),1);
    end
    
    %Reinstate the mean
    out = out + mv;
end

