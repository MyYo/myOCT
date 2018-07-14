%This is the master tester, runs all!
%Designed to be run using Jenkins

%% Setup environment
currentFileFolder = fileparts(mfilename('fullpath'));
yOCTMainFolder = [currentFileFolder '\..\'];
addpath(genpath(yOCTMainFolder)); %Add current files to path
gcp; %Start Parallel Processs

%% Test Reconstruction
disp('Reconstruction Tests');
yOCTTestReconstruction;

%% Test that Demos are working
return;
disp('Demo Tests');
d = dir([yOCTMainFolder 'Demo*']);
for i=1:length(d)
    [~,functionName] = fileparts(d(i).name)
    disp(functionName);
    eval(functionName);
end

%% Done!
disp('All Tests Completed');