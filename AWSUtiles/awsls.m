function [lsContent, lsContentFullName] = awsls (rootLSFolder)
%Lists the files and folders in the rootLSFolder
%lsContent - returns just file/folder names
%lsContentFull - returns full path

%% Credentials check
awsSetCredentials(1);
rootLSFolder = awsModifyPathForCompetability([rootLSFolder '/'],true);

%% Get the conent
[status,text] = system(['aws s3 ls "' rootLSFolder '"']);

%Error check
if status~=0
    error('Error happend while listing %s, Error: %s',rootLSFolder,text);
end

%% Parse content in to files & folders
c=textscan(text,'%s','Delimiter','\n');
c=c{1};
lsContent = cell(size(c,1),1);
for i=1:length(lsContent)
    cs = c{i};
    if (strcmpi(cs(1:4),'pre '))
        %Folder
        lsContent{i} = cs(5:end);
    else
        %File
        cs = strtrim(cs(20:end)); %Remove date
        m = textscan(cs,'%d %[^\n]');
        lsContent{i} = m{end}{1};
    end
end

lsContentFullName = cellfun(@(x)(...
    awsModifyPathForCompetability([rootLSFolder,x],false)), ...
    lsContent,'UniformOutput',false);