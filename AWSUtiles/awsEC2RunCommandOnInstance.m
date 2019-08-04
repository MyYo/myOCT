function [status, txt] = awsEC2RunCommandOnInstance (ec2Instance,cmd)
%This function runs a command on the instance defined by DNS. Make sure to supply the PEM file path
%Both command and PEM file path are procided by awsEC2StartInstance
%INPUTS:
%   ec2Instance - instance created by awsEC2StartInstance
%   cmd - command to run (string). To run multiple commands one after the
%       cmd can be cell array

%% cmd
if (iscell(cmd))
    %Concatinate all commands
    cmd1 = '';
    for i=1:length(cmd)
        cmd1 = [cmd1 cmd{i} ' && '];
    end
    cmd1(end+(-3:0)) = [];
else
    cmd1 = cmd;
end
    
DNS = ec2Instance.dns;
pem = ec2Instance.pemFilePath;
[status,txt] = ssh(sprintf('-i "%s" ec2-user@%s "%s"',pem,DNS,cmd1));
    
