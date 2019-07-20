function awsSetCredentials (type)
%This function sets user name and password for AWS and S3 storage

if ~exist('type','var')
    type = 0; %Only set password
end

global awsSetCredentialsUsed;
if isempty(awsSetCredentialsUsed) ... %Run only once
    || type > awsSetCredentialsUsed  %or if we request the advanced type, but only the basic one was set
    
    if exist('awsSetCredentials_Private') == 2 || exist('awsSetCredentialsPrivate') == 5
        awsSetCredentials_Private();
    else
        error('Cannot find awsSetCredentials_Private which contains AWS private keys. Please ask Yonatan to send you the file or get it at \\171.65.17.174\MATLAB_Share\Jenkins');
    end
    
    if type == 1
       %Make sure AWS  CLI is installed, we will need it
       
       [status,~] = system('aws');
       if (status == 1)
           %AWS is not a recognized command
           error('Please install AWS Command Line Interface for advanced cloud commands. Can be downloaded from: https://aws.amazon.com/cli/');
       end
    end  
    
    awsSetCredentialsUsed = type;
end
