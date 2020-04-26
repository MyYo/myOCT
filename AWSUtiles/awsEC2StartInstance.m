function [ec2Instance] = awsEC2StartInstance(ec2RunStructure,instanceType,numberOfInstances,v)
%This function starts an EC2 instance, returns instance hendle, it will
%wait until instance is up and ready to recive commands
%INPUT:
%   ec2RunStructure - see awsEC2BuildRunStructure for details
%   instanceType - see https://aws.amazon.com/ec2/instance-types/. Example: m4.2xlarge
%   numberOfInstances - number of computers to load, default: 1
%   v - verbose mode, default true
%OUTPUTS:
%   ec2Instance - structure (or structure array if multiple instances)
%       see awsEC2RunstructureToInstance for more information.

if ~exist('numberOfInstances','var') || isempty(numberOfInstances)
    numberOfInstances = 1;
end

if ~exist('v','var')
    v = true;
end

%% Set region
if (~strcmp(awsGetRegion(),ec2RunStructure.region))
    %Change region
    originRegion = awsGetRegion();
    awsSetRegion(ec2RunStructure.region,1);
else
    originRegion = '';
end

%% Start Instance
if (v)
    disp('Starting Instances...');
end
% run instance with HistologyRole (grants access to s3)
[status,txt]=system( ...
    ['aws ec2 run-instances --image-id ' ...
    'ami-' ec2RunStructure.ami ...
    ' --iam-instance-profile ' ...
    'Name="' ec2RunStructure.IAMRole '" ' ... 
    '--count ' num2str(numberOfInstances) ' ' ... 
    '--instance-type ' instanceType ' ' ...
    '--key-name ' ec2RunStructure.pemName ' '...
    '--security-groups ' ec2RunStructure.securityGroup ' '...
    ]);
if status~=0 
    error('Could not start instance: %s',txt);
end
json = jsondecode(txt);
instanceIds = {json.Instances.InstanceId};

%How to shut down instances
tmp = '';
for i=1:length(instanceIds)
    tmp = [tmp instanceIds{i} ', '];
end
tmp(end+(-1:0)) = [];
howToShutDownInstance = sprintf(['To shutdown your instances, go to: \n' ...
    'https://us-west-1.console.aws.amazon.com/ec2/v2/home?region=%s#Instances:sort=instanceId\n' ... 
    'Look for %s'],...
    ec2RunStructure.region ,...
    tmp);

%% Wait for all instances to load (Get a DNS)
if (v)
    disp('Waiting for Instances To Start Running...');
end
for i=1:length(instanceIds)
    [err,txt] = system(sprintf('aws ec2 wait instance-running --instance-ids %s',instanceIds{i}));
    if err~=0
        error('Wait Instance Running Failed: %s',txt);
    end
end

%% Retrieve Instance DNS
DNSs = cell(size(instanceIds));
for i=1:length(instanceIds)
    [err,txt] = system(sprintf('aws ec2 describe-instances --instance-ids %s',...
        instanceIds{i}));
    if err~=0
        error('Error while getting DNS of instance #%d: %s\n%s',i,txt,howToShutDownInstance);
    end
    json2 = jsondecode(txt);
    DNSs{i} = json2.Reservations.Instances.PublicDnsName;
end

%% Wait for status OK from all instances
if (v)
    disp('Waiting for Instances To Return Status OK...');
end
for i=1:length(instanceIds)
    [err,txt] = system(sprintf('aws ec2 wait instance-status-ok --instance-ids %s',instanceIds{i}));
    if err~=0
        error('Wait Instance Status Ok Failed: %s\n%s',txt,howToShutDownInstance);
    end
end

%% Setup SSH for connecting to EC2 Instances
if ssh()
    %SSH Installed
    %Add DNS to ssh to prevent promt
    for i=1:length(DNSs)
        [err,txt] = ssh(sprintf('-tt -o "StrictHostKeyChecking=no" %s@%s',...
            ec2RunStructure.userName,DNSs{i}));
        
        if (err ~= 0)
            %Process the error
            stxt = split(txt,newline);
            permDeniedError = sprintf(...
                '%s@%s: Permission denied (publickey,gssapi-keyex,gssapi-with-mic).',...
                ec2RunStructure.userName,DNSs{i});
            txtNew = [];
            for j=1:length(stxt)
                if isempty(stxt{j})
                    %Empty line, skip
                    continue;
                elseif (strcmpi(stxt{j},permDeniedError))
                    %Expected permission error, skip
                    continue;
                elseif (strncmpi(stxt{j},'warning',7))
                    %This is a warning, skip
                    break;
                end
                txtNew = [txtNew stxt{j} newline];
            end
            
            if ~isempty(txtNew)
                %Error exists
                error('Error in adding DNS: %s.\n The error that we care about: %s.\n%s',txt,txtNew,howToShutDownInstance);
            end
        end
    end
end

%% Generate output structure
clear ec2Instance
for i=1:length(instanceIds)
    ec2Instance(i) = awsEC2RunstructureToInstance(...
        ec2RunStructure,instanceIds{i},DNSs{i});
end

%% Cleanup
%Return to original region
if ~isempty(originRegion)
    awsSetRegion(originRegion,1);
end


%% Done
if(v)
    disp('Instance Running');
end
