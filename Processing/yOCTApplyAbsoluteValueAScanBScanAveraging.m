function [correctedScan] = yOCTApplyAbsoluteValueAScanBScanAveraging(inputScan)
%%This function takes the absolute value of inputScan to extract the
%%intensity values of the scan. The function also applies A scan and B scan
%%averaging. 
%
% INPUTS:
%   - inputScan - 2D or 3D volume with dimensions (z,x,y). More if there is
%     A/B scan averaging, see yOCTLoadInterfFromFile for more information. 
%
% OUTPUTS:
%   - correctedScan - Scan which has the applied absolute value and scan
%                     averaging. The dimensions must be dimensions (z,x,y).

correctedScan = abs(inputScan);
for i=length(size(correctedScan)):-1:4 %Average BScan, AScan avg but no z,x,y
    correctedScan = squeeze(mean(correctedScan,i));
end
