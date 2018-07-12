function [interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions)
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
%   [interfe,dimensionse] = yOCTEquispaceInterf(interf,dimensions)
%
% INPUTS:
%   - interf - interferogram as loaded using yOCTLoadIntefFromFile
%   - dimensions - dimensions structure as loaded using yOCTLoadIntefFromFile
% OUTPUTS:
%   - interf - equispaced interferogram
%   - dimensions - dimensions structure corrected to acount for equispacing

%% Data Structure
s = size(interf);
lambda = dimensions.lambda.values;

k = 2*pi./(lambda); %Get wave lumber in [1/nm]
kLin = linspace(max(k),min(k),length(k)); %Linear

%% Interferogram

%Reshape interferogram for easy parallelization
interf = reshape(interf,s(1),[]);

%Interpolate
interfe = myInterp(k,interf,kLin);

%Reshape back
interfe = reshape(interfe,[size(interfe,1) s(2:end)]);

%% Dimensions
dimensionse = dimensions;
dimensionse.lambda.values = myInterp(k,lambda,kLin);

function out = myInterp(x,v,xq)
out = interp1(x,v,xq,'pchip');


