function awsEC2UploadDataToInstance (DNS,TempPEMFilePath,source,destination)
%This function moves data / files to and from the instance
%INPUTS:
%   DNS - instance path
%   TempPEMFilePath - PEM file path
%   source - file path of files / directory at source. local!
%   destination - file path on desitanation. on instance. Leave empty to
%       save files on root. 
%   An example of directory (compared to root: ~/OCT

if ~exist('destination','var') || isempty(destination)
    destination = '';
end

if (isdir(source))
    %Set reqursive flag
    flag = '-r';
else
    flag = '';
end
%% Make the copy
[stat,txt] = system(sprintf('scp -i "%s" %s "%s" ec2-user@%s:%s',TempPEMFilePath,flag,source,DNS,destination));

if (stat~=0)
    error('Faild to copy files: %s',txt);
end