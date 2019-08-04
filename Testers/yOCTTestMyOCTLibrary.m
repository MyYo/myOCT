function yOCTTestMyOCTLibrary()
%This is the master tester, runs all!
%Designed to be run using runme_Jenkins

%% TMP
tempDir = tempname();
mkdir(tempDir); %Create directory

%Create a local pem file path that we can modify restrictions to
[~,tmp] = fileparts('\\171.65.17.174\MATLAB_Share\Jenkins\HistologyWest.pem');
TempPEMFilePath = [tempDir '\' tmp '.pem'];
copyfile('\\171.65.17.174\MATLAB_Share\Jenkins\HistologyWest.pem',TempPEMFilePath);

%Modify restrictions
s=['ICACLS "' TempPEMFilePath '" /inheritance:r /grant "' getenv('USERNAME') '":(r)']
[err,txt] = system(s) %grant read permission to user, remove all other permissions
if (err~=0)
    error('Error in moidifing pem restrictions %s\n%s',txt,howToShutDownInstance);
end
delete(TempPEMFilePath)
disp('DONE');
error('DONE');

%% Pre test configuration
%Where to find files to be tested
mainTestVectorFolder1 = '\\171.65.17.174\s3\Users\Jenkins\';
mainTestVectorFolder2 = 's3://delazerdamatlab/Users/Jenkins/'; %S3 version

%Get main OCT folder
currentFileFolder = fileparts(mfilename('fullpath'));
yOCTMainFolder = [currentFileFolder '\..\'];

%% Test Reconstruction
disp('Reconstruction Tests... (Local Folder Version)');
yOCTTestReconstruction([mainTestVectorFolder1 'SampleOCTVolumes\']);
disp('Reconstruction Tests... (S3 Version)');
yOCTTestReconstruction([mainTestVectorFolder2 'SmallSampleOCTVolumes/']);

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
yOCTTestUnzip([mainTestVectorFolder1 'ZippedOCTFolder\']);
disp('Unzip Tests... (S3 Version)');
yOCTTestUnzip([mainTestVectorFolder2 'ZippedOCTFolder/']);

%% Test batch processing
disp('Testing Batch Processing... (S3 Version)');
myOCTBatchProcess([mainTestVectorFolder2 'SmallSampleOCTVolumes/'],{'parallelOption',1});
myOCTBatchProcess([mainTestVectorFolder2 'SmallSampleOCTVolumes/'],{'parallelOption',2});


%% Done!
disp('All Tests Completed');