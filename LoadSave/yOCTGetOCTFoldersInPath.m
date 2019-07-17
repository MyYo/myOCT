function [OCTFolders,folderNames] = yOCTGetOCTFoldersInPath (p)
%This function returns all OCT folders that are found in path p.
%If p is an OCT folder it will return it, if p has sub-folders, this
%function will search in subfolders.
%INPUTS:
%   p - root path to search in. Can be on local disk or Amazon S3
%OUTPUTS:
%   OCTFolders - cell array containing path to each OCT folder that was found
%       OCTFolders will be empty in case no folders are found. 
%       These contain the full path of the foler
%   folderNames - just the name of the most inner folder

%% AWS
if (strcmpi(p(1:3),'s3:'))
    yOCTSetAWScredentials;
    p = yOCTModifyPathForAWSCompetability(p);
    isAWS = true;
else
    isAWS = false;
end

if (strcmpi(p(end+(-3:0)),'.oct'))
    %% p is a path to .OCT file
    %This is a .OCT file, success, single file case
	OCTFolders = {p}; 
else
    %% Look insite folder p
    %Get All compressed OCT Files
    try
        ds=fileDatastore(p,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions','.oct');
        OCTFolders = ds.Files(:);
    catch
        %No .oct
        OCTFolders = {};
    end

    %Get all oct folders in that subfolder
    try
        ds=fileDatastore(p,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions',...
            {...
            '.xml' ... Thorlabs, search for header.xml
            '.dat' ... Thorlabs SRR, searches for chirp.dat
            '.bin','.tif' ... Wasatch
            });
        xmls = ds.Files(:);
        tmp = cellfun(@(x)(fileparts(x)),xmls,'UniformOutput',false);
        OCTFolders = [OCTFolders ; tmp];
        OCTFolders = unique(OCTFolders);
    catch
        %No other files
    end
end

%% Folder names
folderNames = cell(size(OCTFolders));
for i=1:length(folderNames)
    folderName = split(OCTFolders{i},{'/','\'});
    folderName = folderName(~cellfun(@isempty,folderName)); %Remove empty slots
    folderName = folderName{end};
    
    folderNames{i} = folderName;
end

%% Add Last '/' At the end of the folder
for i=1:length(OCTFolders)
    if strcmp(OCTFolders{i}(end+(-3:0)),'.oct')
        %This is an .OCT file don't add '\' at the end
        continue;
    end
    
    if (isAWS)
        OCTFolders{i} = [OCTFolders{i} '/'];
    else
        OCTFolders{i} = [OCTFolders{i} '\'];
    end
end