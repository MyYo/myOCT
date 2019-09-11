%This deom preforms a tiled scan

%% Scan
yOCTScanTile(...
    'D:\Yonatan\s1\',... Output folder
    ... Optional parameters, see documentation for full list of parameters
    'octProbePath','C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini',...
    'isVerifyMotionRange',false, ...
    'nXPixels',200, ... %Number of pixels along the x direction
    'nYPixels',200,  ... %Number of pixels along the y direction
    'xCenters' ,[0 1], ...[mm] center of scan
    'yCenters' ,[0 1], ...[mm] center of scan
    'zDepts', [0 0.1] ...[mm]
);