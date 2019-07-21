function p = awsModifyPathForCompetability (p,isAWS_CLI)
%This function excepts path and output AWS compatible path.
%INPUTS: 
%   p - path
%   isAWS_CLI - will this path be used with AWS CLI? if so, spaces are
%   defined slightly differently. If used path for datastore, set to false

if ~exist('isAWS_CLI','var')
    isAWS_CLI = false;
end

p = strrep(p,'\','/'); %Repalce slashes

%Replace double slashed
p = strrep(p,'//','/'); 
p = strrep(p,'//','/'); 
p = strrep(p,'s3:/','s3://'); 

if isAWS_CLI
    p = strrep(p,'%20',' '); %Replace Spaces
else
    p = strrep(p,' ','%20'); %Replace Spaces
end
