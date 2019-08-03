function awsEC2TerminateInstance(instanceIds,TempPEMFilePath,v)
%Terminates instances
%INPUTSL
%   - instanceIds can be a string or a cell array if multiple instances exist
%   - TempPEMFilePath - Do we wish to delete temporary PEM file, if so, provide path
%   - v - verbose mode

%% Input checks
if ~exist('TempPEMFilePath','var')
    TempPEMFilePath = '';
end

if ~iscell(instanceIds)
    instanceIds = {instanceIds};
end

if ~exist('v','var')
    v = true;
end

%% Terminate the instance
for i=1:length(instanceIds)
    [err,txt]=system(sprintf('aws ec2 terminate-instances --instance-ids %s',instanceIds{i}));
    if (err~=0)
        error('Faild to terminate instance, ID: %s. Message: %s',instanceIds{i},txt);
    end
end

%% Delete PEM
if ~isempty(TempPEMFilePath)
    %Cleanup PEM
    delete(TempPEMFilePath);
    rmdir(fileparts(TempPEMFilePath))
end

%% Print
if(v)
    disp('Instance Terminated');
end