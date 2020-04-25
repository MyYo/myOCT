function [status,txt] = scp(cmd, isRunAsync)
% USAGE:
%   [status,txt] = scp([cmd, isRunAsync])
% INPUTS:
%   - If no inputs provided will return true or false indicating, is scp
%     installed on this computer.
%   - cmd - scp command to run.
%   - isRunAsync - when set to true will execute scp command and return to
%     Matlab without waiting for the copy command to complete. Default:
%     false.

if exist('isRunAsync','var') && isRunAsync
    cmd = [cmd '&'];
end

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