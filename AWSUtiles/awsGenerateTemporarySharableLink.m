function url = awsGenerateTemporarySharableLink(s3Path ,isUseCash)
% This function generates a temprary sharable link to an object in s3
% INPUTS:
%   - s3Path - s3 object path: 's3://bucketname/...'
%   - isUseCash, if set to true, and in case this object was already
%       generated recently, pull it from cash to save time. Set to false to
%       clear cash. Default is true.

%% Input check
if ~awsExist(s3Path,'file')
    warning('Need a valid s3 path to generate a link to, got %s',s3Path);
    url = '';
    return;
end
if ~exist('isUseCash','var')
    isUseCash = true;
end

%% Convert to s3 path, check cash
s3Path = awsModifyPathForCompetability(s3Path,true);

persistent cash 
if ~isUseCash || isempty(cash)
    cash = cell(0,2);
end
if isUseCash
    i=cellfun(@(x)(strcmp(s3Path,x)),cash(:,1));
    if any(i)
        url = cash{find(i==1,1,'first'),2}; % Read from cash
        return;
    end
end
    
%% Generate code
expiresInSec = 7*24*60*60; % one week
[errCode, txt] = awsCmd(sprintf('aws s3 presign "%s" --expires-in %d',s3Path,expiresInSec));
url = txt;

if isUseCash
    % Save to cash
    newC = size(cash,1)+1;
    cash{newC,1} = s3Path;
    cash{newC,2} = url;
end