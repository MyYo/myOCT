function awsEC2TemporarilyDisconnectFromInstance(ec2Instance)
% This function temporarily disconnect from instance, clears pem file. to
% reconnect use awsEC2RunstructureToInstance. You will need to save
% instance id and dns from ec2Instance to reconnect. 
%
% INPUTS:
%   ec2Instance - see awsEC2RunstructureToInstance for more details.

%Delete the pem file
if exist(ec2Instance.pemFilePath,'file')
    rmdir(fileparts(ec2Instance.pemFilePath),'s');
end