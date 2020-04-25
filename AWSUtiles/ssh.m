function [status,txt] = ssh(cmd, isRunAsync)
% USAGE:
%   [status,txt] = ssh([cmd, isRunAsync])
% INPUTS:
%   - If no inputs provided will return true or false indicating, is ssh
%     installed on this computer.
%   - cmd - ssh command to run.
%   - isRunAsync - when set to true will execute ssh command and return to
%     Matlab without waiting for the ssh command to complete. Default:
%     false.

if exist('isRunAsync','var') && isRunAsync
    cmd = [cmd '&'];
end

%% Figure out where ssh is installed
global sshInstalledPrefix
if isempty(sshInstalledPrefix)
   [stat,~] = system('ssh');
    if stat == 255 
        sshInstalledPrefix = 'ssh';
    elseif exist('C:\Program Files\OpenSSH','dir')
        sshInstalledPrefix = '"C:\Program Files\OpenSSH\ssh.exe"';
    elseif exist('C:\Program Files (x86)\OpenSSH','dir')
        sshInstalledPrefix = '"C:\Program Files (x86)s\OpenSSH\ssh.exe"';
    else
        %SSH Not Installed 
        sshInstalledPrefix = false;
    end
end

%% Are we looking to run a command or just know if SSH is installed

isSSHInstalled = ischar(sshInstalledPrefix);
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
[status,txt] = system([sshInstalledPrefix ' ' cmd]);