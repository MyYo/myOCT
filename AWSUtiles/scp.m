function [status,txt] = scp(cmd)
%USAGE:
%   [status,txt] = scp(cmd)
%   [isInstalled] = scp()
%This function runs ssh command. If no command is provided will return if
%ssh is installed, false otherwise

%% Figure out where ssh is installed
isSSHInstalled = ssh();

%% Are we looking to run a command or just know if SSH is installed

if ~exist('cmd','var')
    %User is only interested to know if SSH is installed
    status = isSSHInstalled;
    txt = isSSHInstalled;
    return;
elseif ~isSSHInstalled
    %User wants to run a command but SSH is not installed
    error ('Please Install SSH, for example: OpenSSH');
end

%% Run the command
global sshInstalledPrefix;
prefix = strrep(sshInstalledPrefix,'ssh','scp');
[status,txt] = system([prefix ' ' cmd]);