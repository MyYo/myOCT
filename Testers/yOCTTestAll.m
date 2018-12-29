%This is the master tester, runs all!
%Designed to be run using Jenkins

%try

%% Setup environment
currentFileFolder = fileparts(mfilename('fullpath'));
yOCTMainFolder = [currentFileFolder '\..\'];
addpath(genpath(yOCTMainFolder)); %Add current files to path
gcp; %Start Parallel Processs
opengl('save', 'software'); %Increase stubility in OPEN GL

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
%disp('All Tests Completed');
%catch ME
%    for i=1:length(ME.stack)
%        ME.stack(i)
%    end
%    disp(ME.message); %Write
%    exit(1); %Problem Happend
%end
%exit(0); %Safe exist