function awsEC2TerminateInstance(ec2Instance,v)
%Terminates instances
%INPUTSL
%   - ec2Instance - instance created by awsEC2StartInstance
%   - v - verbose mode

%% Input checks
if ~exist('v','var')
    v = true;
end

origRegion = awsGetRegion();

%% Terminate the instance
for i=1:length(ec2Instance)
    %Delete the pem file
    if exist(ec2Instance(i).pemFilePath,'file')
        rmdir(fileparts(ec2Instance(i).pemFilePath),'s');
    end
    
    %Set region
    if (~strcmp(awsGetRegion(),ec2Instance(i).region))
        awsSetRegion(ec2Instance(i).region,1);
    end
    
    %Terminate
    [err,txt]=system(sprintf('aws ec2 terminate-instances --instance-ids %s',ec2Instance(i).id));
    if (err~=0)
        error('Faild to terminate instance, ID: %s. Message: %s',ec2Instance(i).id,txt);
    end
end

%% Return to original region
if (~strcmp(awsGetRegion(),origRegion))
    awsSetRegion(origRegion,1);
end

%% Print
if(v)
    disp('Instance Terminated');
end