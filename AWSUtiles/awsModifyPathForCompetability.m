function p = awsModifyPathForCompetability (p,isAWS_CLI)
%This function excepts path and output AWS compatible path.
%If p is not AWS path will modify it to be compatible with windows
%INPUTS: 
%   p - path
%   isAWS_CLI - will this path be used with AWS CLI? if so, spaces are
%   defined slightly differently. If used path for datastore, set to false

if (awsIsAWSPath(p))
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
else
	%Regular path
	p = strrep(p,'/','\'); %Repalce slashes
	p = [p(1:2) strrep(p(3:end),'\\','\')]; %Replace double slashed, starting 3rd note so if we start with '\\127.0.0.1\' will not replace the begning
end