function awsEC2UploadDataToInstance (ec2Instance,source,destination)
%This function moves data / files to and from the instance
%INPUTS:
%   - ec2Instance - instance created by awsEC2StartInstance
%   TempPEMFilePath - PEM file path
%   source - file path of files / directory at source. local!
%   destination - file path on desitanation. on instance. Leave empty to
%       save files on root. 
%   An example of directory (compared to root: ~/OCT

if ~exist('destination','var') || isempty(destination)
    destination = '';
end

%% Make the copy
DNS = ec2Instance.dns;
pem = ec2Instance.pemFilePath;
[stat,txt] = scp(sprintf('-i "%s" -r "%s" %s@%s:%s',...
    pem,source,ec2Instance.userName,DNS,destination));

if (stat~=0)
    error('Faild to copy files: %s',txt);
end