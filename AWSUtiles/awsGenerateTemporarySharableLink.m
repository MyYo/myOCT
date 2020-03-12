function url = awsGenerateTemporarySharableLink(s3Path)
% This function generates a temprary sharable link to an object in s3

if ~awsExist(s3Path,'file')
    error('Need a valid s3 path to generate a link to, got %s',s3Path);
end
s3Path = awsModifyPathForCompetability(s3Path,true);

expiresInSec = 7*24*60*60; % one week
[errCode, txt] = awsCmd(sprintf('aws s3 presign "%s" --expires-in %d',s3Path,expiresInSec));
url = txt;