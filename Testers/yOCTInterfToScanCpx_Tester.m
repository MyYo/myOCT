%This function tests code package for bit exact competability

clear;

%% Inputs
isUpdateTesterResults = false; %Update tester data according to the new results?

%filePath = 'yOCTInterfToScanCpx_TesterData1.mat';
filePath = 's3://delazerdalab1/CodePackage/TestVectors/yOCTInterfToScanCpx_TesterData1.mat';

%% Preform Tests
disp('Testing yOCTInterfToScanCpx');
if (strcmp(filePath(1:3),'s3:'))
    %Load Data from AWS
    yOCTSetAWScredentials;
    ds=fileDatastore(filePath,'ReadFcn',@load);
    data = ds.read;
else
    %Load Data locally
    data = load(filePath);
end

tic
scanCpx2 = yOCTInterfToScanCpx(data.interf,data.dim.lambda.k_n,'dispersionParameterA',data.dispersionParameterA);
runtime2 = toc;

%% Check results
if (max(abs(scanCpx2(:)-data.scanCpx(:))) < 1e-10)
    disp('Bit Exact');
else
    disp('Problem');
    max(abs(scanCpx2(:)-data.scanCpx(:)))
end
fprintf('Current Runtime: %.2f[sec], Reference: %.2f[sec]\n',runtime2,data.runtime);

if (isUpdateTesterResults)
    scanCpx = scanCpx2;
    runtime = runtime2;
end