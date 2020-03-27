function [lsContent, lsContentFullName] = awsls (rootLSFolder)
%Lists the files and folders in the rootLSFolder path
%lsContent - returns just file/folder names
%lsContentFull - returns full path

%% Handel the case where pointing to local folder
if (~awsIsAWSPath(rootLSFolder))
    % Get content
    tmp = dir(rootLSFolder);
    
    lsContent = {tmp(3:end).name};
    lsContentFullName = cellfun(@(x)(...
        awsModifyPathForCompetability([rootLSFolder,x],false)), ...
        lsContent,'UniformOutput',false);
    
    return;
end

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
        if ~isempty(m{end})
            lsContent{i} = m{end}{1};
        else
            lsContent{i} = '';
        end
    end
end

% Remove all empty content
lsContent(cellfun(@isempty,lsContent)) = [];

lsContentFullName = cellfun(@(x)(...
    awsModifyPathForCompetability([rootLSFolder,x],false)), ...
    lsContent,'UniformOutput',false);