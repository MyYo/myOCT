function [errCode, txt] = awsCmd(cmd, knownErrorText, v)
% This function executes system command: system(cmd). It will parse the
% error codes and decide if to throw an error, or retry command.
% INPUTS:
%   cmd - command to be executed
%   knownErrorText - cell array containin known error texts to ignore, they
%       are fine. Default: {}
%   v - verbose mode, default: false

%% Input checks

if ~exist('knownErrorText','var') || isempty(knownErrorText)
    knownErrorText = {};
end

if ~exist('v','var') || isempty(v)
    v = false;
end

%% Run command
nTimesToRetry = 3;
for retryI=(nTimesToRetry-1):-1:0 % Retry x times
    [errCode,txt] = system(cmd);
    
     %% Error handling
    %Known errors to ignore
    etxt = txt;
    for i=1:length(knownErrorText)
        etxt = strrep(etxt,knownErrorText{i},'');
    end
    etxt = strtrim(etxt);
    
    if (errCode ~= 0)  && ~isempty(etxt)
        emessage = ...
            sprintf('%s\nResulted in an error code %d: %s', cmd, errCode, txt);
        
        if (retryI > 0)
            warning('%s. retry in 1 sec.',emessage);
            pause(1);
        else
            error('%s',emessage);
        end
    else
        %Good code, no need to retry
        break;
    end
end

%% Finish
if(v)
    disp(txt);
end