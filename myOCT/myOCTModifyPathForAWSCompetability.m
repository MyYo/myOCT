function p = myOCTModifyPathForAWSCompetability (p)
%This function excepts path and output AWS compatible path

p = strrep(p,' ','%20'); %Replace Spaces
p = strrep(p,'\','/'); %Replace Spaces

