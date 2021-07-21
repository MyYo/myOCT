function [correctedScan] = yOCTOpticalPathCorrection(inputScan, inputScanDimensions, octProbePath)
%%This function performs optical path correction on the inputScan
%
% INPUTS:
%   - inputScan - 2D or 3D volume with dimensions (z,x,y). More if there is
%     A/B scan averaging, see yOCTLoadInterfFromFile for more information. 
%     Note: inputScan must only contain real values (not complex). A-scan 
%           and B-scan averaging must be performed before calling
%           yOCTOpticalPathCorrection
%   - inputScanDimensions - The dimension information of the inputScan 
%   - octProbePath - Path to the OCT probe JSON information 
%
% OUTPUTS:
%   - correctedScan - Scan which has the applied optical path correction

% Extract optical path polynomial from OCT probe JSON
OP_p = octProbePath.OpticalPathCorrectionPolynomial;
OP_p = OP_p(:)';

% Instantiate variable for scan with optical path correction
correctedScan = zeros(size(inputScan));

%Change dimensions to microns units
inputScanDimensions = yOCTChangeDimensionsStructureUnits(inputScanDimensions,'microns');

% Iterate through the inputScan volume and apply optical path correction to
% each individual scan 
for i=1:size(inputScan,3)
    
    % Extract B-scan from volume
    scan = squeeze(inputScan(:,:,i));
    
    %Lens abberation / optical path correction
    correction = @(x,y)(x*OP_p(1)+y*OP_p(2)+x.^2*OP_p(3)+y.^2*OP_p(4)+x.*y*OP_p(5)); %x,y are in microns
    [xx,zz] = meshgrid(inputScanDimensions.x.values,inputScanDimensions.z.values); %um                
    scan_min = min(scan(:));
    scan = interp2(xx,zz,scan,xx,zz+correction(xx,inputScanDimensions.y.values(i)),'nearest');
    scan_nan = isnan(scan);
    scan(scan<scan_min) = scan_min; %Dont let interpolation value go too low
    scan(scan_nan) = 0; %interpolated nan values should not contribute to image
    
    correctedScan(:,:,i) = scan;
end
