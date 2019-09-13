function awsMkDir(myDir, cleanUp)
%This function will make dir if it doesnt exist (either in AWS or locally)
%When cleanUp set to true (default) will delete dir content prior to
%creating folder

%% Input checks
if ~exist('cleanUp','var')
    cleanUp = true;
end

%% Create dir

%Cleanup if required
if (cleanUp)
    awsRmDir(myDir);
end

%Create dir
if ~awsIsAWSPath(myDir) && ~exist(myDir,'dir')
    mkdir(myDir);
end