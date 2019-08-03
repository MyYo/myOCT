%% Initialize
disp('Demo Using EC2 Connection');
awsSetCredentials(1);

%% Build structrue
if exist('My_ec2RunStructure.m','file')
    %Private function with our lab's keys
    ec2RunStructure = My_ec2RunStructure(); 
else
    %Example:
    ec2RunStructure = awsEC2BuildRunStructure('09ff3322b1111911f','MyPem.pem',...
        'AccessRole','Security','us-west-1');
    error('Need to generate ec2RunStructure, see line above for an example');
end

%% Start Instance
[instanceId,DNS,TempPEMFilePath] = awsEC2StartInstance(ec2RunStructure,'t2.micro'); %m4.2xlarge

%% Run commands & Copy files
disp('Run commands and copy');

%Upload a temp file
save('tmp1.mat');
awsEC2UploadDataToInstance (DNS,TempPEMFilePath,'tmp1.mat');
delete('tmp1.mat');

%Run command
[~,randDirName] = fileparts([tempname '.txt']);
[status,txt] = awsEC2RunCommandOnInstance (DNS,TempPEMFilePath,{...
    ['mkdir ' randDirName] ... Command #1
    'ls'... Command #2
    });
fprintf('Status: %d. Output:\n%s\n',status,txt)

%% Terminate
awsEC2TerminateInstance(instanceId,TempPEMFilePath);

disp ('Demo Done!');

