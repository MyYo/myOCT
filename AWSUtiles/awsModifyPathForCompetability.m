function p = awsModifyPathForCompetability (p,isAWS_CLI)
%This function excepts path and output AWS compatible path.
%If p is not AWS path will modify it to be compatible with windows
%INPUTS: 
%   p - path
%   isAWS_CLI - will this path be used with AWS CLI? if so, spaces are
%   defined slightly differently. If used path for datastore, set to false
if (length(p)<2)
    return; %No modifications needed
end

isAWS = awsIsAWSPath(p);
%% Generic replacements
%Spatial cases
p = strrep(p,'s3://','~1/');   %Save important structures, s3 paths
p = [strrep(p(1:2),'\\','~2/') p(3:end)]; %Save important structures, \\127.0.0.1\ Paths

p = strrep(p,'\','/'); %Repalce slashes

%Duplicance
p = strrep(p,'//','/'); 
p = strrep(p,'//','/'); 

%Handle '../'
s = split(p,'/');
find2Dots = @(s)(cellfun(@(x)strcmp(x,'..'),s));
iterations = sum(find2Dots(s));
if (iterations > 0)
    for i=1:iterations
        j = find(find2Dots(s) == 1,1,'first');
        s(j+(-1:0)) = [];
    end
    p = strjoin(s,'/');
end

%Replace back spatial cases
p = strrep(p,'~1/','s3://');
p = strrep(p,'~2/','\\');
p = strrep(p,newline,'');

%% Platform specific paths
if (isAWS)
    if ~exist('isAWS_CLI','var')
		isAWS_CLI = false;
    end

	if isAWS_CLI
		p = strrep(p,'%20',' '); %Replace Spaces
	else
		p = strrep(p,' ','%20'); %Replace Spaces
	end
else
	%Regular path
    if ispc
        p = strrep(p,'/','\'); %Repalce slashes if running on windows
    else
        p = strrep(p,'\','/');
    end
end