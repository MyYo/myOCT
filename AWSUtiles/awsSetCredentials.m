function awsSetCredentials (level)
%This function sets user name and password for AWS and S3 storage
%level specify how much access would the software need.
%level = 0 - Read capability using fileDataStore
%level = 1 - Write, start EC2 instances etc using AWS CLI

if ~exist('type','var')
    level = 0; %Only set password
end

global awsSetCredentialsUsed;
if isempty(awsSetCredentialsUsed) ... %Run only once
    || level > awsSetCredentialsUsed  %or if we request the advanced type, but only the basic one was set
    
    if exist('awsSetCredentials_Private','file')    
        [AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY, DefaultRegion] = awsSetCredentials_Private ();
        %awsSetCredentials_Private output example:
        %DefaultRegion='us-west-1'
        %AWS_ACCESS_KEY_ID = 'id'
        %AWS_SECRET_ACCESS_KEY = 'key'
    else
        error('Cannot find awsSetCredentials_Private which contains AWS private keys. Please ask Yonatan to send you the file or get it at \\171.65.17.174\MATLAB_Share\Jenkins. Atlertitevely create your own function - interface is just above this line, ');
    end
    
    if level == 1
       %Make sure AWS  CLI is installed, we will need it
       
       [status,~] = system('aws');
       if (status == 1)
           %AWS is not a recognized command
           error('Please install AWS Command Line Interface for advanced cloud commands. Can be downloaded from: https://aws.amazon.com/cli/');
       end
    end  
    
    %Set credentials
    setenv('AWS_ACCESS_KEY_ID', AWS_ACCESS_KEY_ID); 
    setenv('AWS_SECRET_ACCESS_KEY', AWS_SECRET_ACCESS_KEY); 
    
    %Set default region
    awsSetRegion(DefaultRegion,level);
    
    awsSetCredentialsUsed = level;
end

%Is this code executed on worker?
isOnWorker = ~isempty(getCurrentTask());
if (isOnWorker && level >= 1)
    fprintf(['WARNING:\n' ...
        'awsSetCredentials level requires CLI access, however this code runs on a worker.\n' ...
        'As of August 2019, Matlab workers running on AWS cloud don''t have the latest version of AWS CLI ' ...
        'which means you might have errors when accesing the cloud. Be ware!' ...
        ]));
end
    
