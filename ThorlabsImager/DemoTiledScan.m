%This deom preforms a tiled scan

%% Scan
yOCTScanTile(...
    'MyScan\',... Output folder
    ... Optional parameters, see documentation for full list of parameters
    'octProbePath','C:\Program Files\Thorlabs\SpectralRadar\Config\Probe - Olympus 10x.ini',...
    'isVerifyMotionRange',false, ...
    'nXPixels',200, ... %Number of pixels along the x direction
    'nYPixels',10,  ... %Number of pixels along the y direction
    'xToScan' ,[-0.5 0.5], ...[mm] center of scan
    'yToScan' ,[-0.5 0.5], ...[mm] center of scan
    'zToScan', [0 0.1] ...[mm]
);