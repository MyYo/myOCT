function yOCTSetAWScredentials (type)
%This function sets user name and password for AWS and S3 storage

if ~exist('type','var')
    type = 0; %Only set password
end

global yOCTSetAWScredentialsUsed;
if isempty(yOCTSetAWScredentialsUsed) ... %Run only once
    || type > yOCTSetAWScredentialsUsed  %or if we request the advanced type, but only the basic one was set
    
    if exist('yOCTSetAWScredentialsPrivate') == 2 || exist('yOCTSetAWScredentialsPrivate') == 5
        yOCTSetAWScredentialsPrivate();
    else
        error('Cannot find yOCTSetAWScredentialsPrivate which contains AWS private keys. Please ask Yonatan to send you the file');
    end
    
    if type == 1
       %Make sure AWS  CLI is installed, we will need it
       
       [status,~] = system('aws');
       if (status == 1)
           %AWS is not a recognized command
           error('Please install AWS Command Line Interface for advanced cloud commands. Can be downloaded from: https://aws.amazon.com/cli/');
       end
    end  
    
    yOCTSetAWScredentialsUsed = type;
end
