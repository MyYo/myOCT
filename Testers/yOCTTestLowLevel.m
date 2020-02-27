%Unit testing of low level functions
OCTFolder = 's3://delazerdamatlab/Users/Jenkins/SampleOCTVolumes/Ganymede_2D_BScanAvg/';

%% Load the OCT
awsSetCredentials();
[interf, dim] = yOCTLoadInterfFromFile(OCTFolder,...
    'BScanAvgFramesToProcess',1:5,'YFramesToProcess',1:2);

%% Verify units conversion works
dim2 = yOCTChangeDimensionsStructureUnits(dim,'microns');
if (max(abs(dim2.lambda.values*1000-dim.lambda.values)) > 1e-5 || ...
    max(abs(dim2.x.values-dim.x.values)) > 1e-5)
    error('Dimensions convertion is broken');
end

%% See that conversion doesn't change processing
scan1 = yOCTInterfToScanCpx(interf,dim,'dispersionQuadraticTerm',100);
scan2 = yOCTInterfToScanCpx(interf,dim2,'dispersionQuadraticTerm',100);

if (max(abs(scan1(:)-scan2(:))) > 1e-5)
    error('Unit conversion breaks reconstruction');
end