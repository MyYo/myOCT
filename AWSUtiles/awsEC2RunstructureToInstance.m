function ec2Instance = awsEC2RunstructureToInstance(ec2RunStructure, id, dns)
% This function converts ec2Runstructure to ec2Instance, setting up pem and
% permissions.
% USAGE:
%   awsEC2RunstructureToInstance(ec2RunStructure, id, dns)
% INPUTS:
%   - ec2RunStructure - see awsEC2BuildRunStructure for details
%   - id - id of the instances loaded. See awsEC2StartInstance for more
%       information abouth this field
%   - dns - DNS to access each id using ssh. See awsEC2StartInstance for more
%       information abouth this field
% OUTPUTS:
%   ec2Instance - structure with the fields:
%       id - id of the instances loaded
%       dns - DNS to access each id using ssh
%       region - which region the instance is running on
%       pemFilePath - filepath of the PEM, temporary, delete after
%       running

% In case we have an error, how do we shut down an instance
howToShutDownInstance = sprintf(...
    ['To shutdown your instances, go to: \n' ...
    'https://us-west-1.console.aws.amazon.com/ec2/v2/home?region=%s#Instances:sort=instanceId\n' ... 
    'Look for %s'],...
    ec2RunStructure.region ,...
    id);

%% PEM
% Createa a local copy and set restrictions.

%Create a local pem file path that we can modify restrictions to
tempDir = tempname();
mkdir(tempDir); %Create directory
[~,tmp] = fileparts(ec2RunStructure.pemFilePath);
TempPEMFilePath = [tempDir '\' tmp '.pem'];

if ~exist(ec2RunStructure.pemFilePath,'file')
    error('Cannot find PEM file path at: %s. Or file not accesible',ec2RunStructure.pemFilePath);
end
	
copyfile(ec2RunStructure.pemFilePath,TempPEMFilePath);   
pemFP = TempPEMFilePath;

%Get user to modify restrictions
loggedInUser = getenv('USERNAME');
%loggedInUser = strrep(loggedInUser,'MATLAB-SERVER$','SYSTEM'); %Matlab Server is actually system.
if (contains(loggedInUser,'$')) %Change $ to system
    warning('getenv(''USERNAME'') returned a wired user name: "%s". Changing to "SYSTEM"',loggedInUser);
    loggedInUser = 'SYSTEM';
end

%Modify restrictions
s=['ICACLS "' pemFP '" /inheritance:r /grant "' loggedInUser '":(r)'];
[err,txt] = system(s); %grant read permission to user, remove all other permissions
if (err~=0)
    error('Error in moidifing pem restrictions %s\n%s',txt,...
        howToShutDownInstance);
end

%% Generate output structure
clear ec2Instance 
ec2Instance.id = id;
ec2Instance.dns = dns;
ec2Instance.region = ec2RunStructure.region;
ec2Instance.pemFilePath = pemFP;
ec2Instance.userName = ec2RunStructure.userName;