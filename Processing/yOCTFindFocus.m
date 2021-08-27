function zFocusPix = yOCTFindFocus(varargin)
% This function find focus z position. It works well if the scan was done 
% using high magnification lens (>10x). The way the algorithm works is by
% finding the brightest z depth in the image and associating that to the
% focus. You can configure this function to allow for manual refinment or
% be completly automatic.
% "zDepthStitchingMode" is used when inputDataFolder is a TiledScan where
% each volume is acquired at different depth
%USAGE:
%    zFocusPix = yOCTLoadScan(inputDataFolder, [,parameter1,...]);
%INPUTS:
%   - inputDataFolder - OCT file / folder in local computer or at s3://
% LIST OF OPTIONAL PARAMETERS AND VALUES
% Parameter                  Default    Information & Values
% 'zDepthStitchingMode'      False      See description above.
% 'manualRefinment'          False      See description above.
%OUTPUTS:
%   - zFocusPix - zDepth (in pixels) of the focus position in the scan

