function yOCTTestMyOCTLibrary()
%This is the master tester, runs all!
%Designed to be run using runme_Jenkins

%Get main OCT folder
currentFileFolder = fileparts(mfilename('fullpath'));
yOCTMainFolder = [currentFileFolder '\..\'];

%% Test Reconstruction
disp('Reconstruction Tests');
yOCTTestReconstruction;

%% Test that Demos are working
disp('Demo Tests');
d = dir([yOCTMainFolder 'Demo*']);
for i=1:length(d)
    [~,functionName] = fileparts(d(i).name);
    disp(functionName);
    eval(functionName);
    close all;
end

%% Done!
disp('All Tests Completed');