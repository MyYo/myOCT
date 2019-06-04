function yOCTTestMyOCTLibrary()
%This is the master tester, runs all!
%Designed to be run using runme_Jenkins

%Where to find files to be tested
mainTestVectorFolder1 = '\\171.65.17.174\s3\Users\Jenkins\';
mainTestVectorFolder2 = 's3://delazerdamatlab/Users/Jenkins/'; %S3 version

%Get main OCT folder
currentFileFolder = fileparts(mfilename('fullpath'));
yOCTMainFolder = [currentFileFolder '\..\'];

%% Test Reconstruction
disp('Reconstruction Tests... (Local Folder Version)');
yOCTTestReconstruction([mainTestVectorFolder1 'SampleOCTVolumes\'];
disp('Reconstruction Tests... (S3 Version)');
yOCTTestReconstruction([mainTestVectorFolder2 'SampleOCTVolumes/'];

%% Test that Demos are working
disp('Demo Tests');
d = dir([yOCTMainFolder 'Demo*']);
for i=1:length(d)
    [~,functionName] = fileparts(d(i).name);
    disp(functionName);
    eval(functionName);
    close all;
end

%% Test Unzip
disp('Unzip Tests... (Local Folder Version)');
yOCTTestReconstruction([mainTestVectorFolder1 'ZippedOCTFolder\'];
disp('Unzip Tests... (S3 Version)');
yOCTTestReconstruction([mainTestVectorFolder2 'ZippedOCTFolder/'];

%% Done!
disp('All Tests Completed');