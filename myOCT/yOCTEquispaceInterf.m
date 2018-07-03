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
kn = (lambda-min(lambda))/(max(lambda)-min(lambda)).*(length(lambda)-1);

knLin = linspace(min(kn),max(kn),length(kn)); %Linear kns

%% Interferogram

%Reshape interferogram for easy parallelization
interf = reshape(interf,s(1),[]);

%Interpolate
interfe = myInterp(kn,interf,knLin);

%Reshape back
interfe = reshape(interfe,[size(interfe,1) s(2:end)]);

%% Dimensions
dimensionse = dimensions;
dimensionse.lambda.values = myInterp(kn,lambda,knLin);

function out = myInterp(x,v,xq)
out = interp1(x,v,xq,'pchip');


