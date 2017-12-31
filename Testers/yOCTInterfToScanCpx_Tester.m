%This function tests code package for bit exact competability

clear;

%% Inputs
isUpdateTesterResults = false; %Update tester data according to the new results?

%% Preform Tests
disp('Testing yOCTInterfToScanCpx');
load('yOCTInterfToScanCpx_TesterData1.mat');

tic
scanCpx2 = yOCTInterfToScanCpx(interf,dim.lambda.k_n,'dispersionParameterA',dispersionParameterA);
runtime2 = toc;

%% Check results
if (max(abs(scanCpx2(:)-scanCpx(:))) < 1e-10)
    disp('Bit Exact');
else
    disp('Problem');
    max(abs(scanCpx2(:)-scanCpx(:)))
end
fprintf('Current Runtime: %.2f[sec], Reference: %.2f[sec]\n',runtime2,runtime);

if (isUpdateTesterResults)
    scanCpx = scanCpx2;
    runtime = runtime2;
end