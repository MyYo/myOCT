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

%TBD in the future, find the comonality between the two functions (AWS and
%no AWS and make them one!)
if (awsIsAWSPath(p))
    [OCTFolders,folderNames] = yOCTGetOCTFoldersInPath_AWS(p);
else
    [OCTFolders,folderNames] = yOCTGetOCTFoldersInPath_NoAWS(p);
end


%% If this is not an AWS Case
function [OCTFolders,folderNames] = yOCTGetOCTFoldersInPath_NoAWS(p)
isAWS = false;

if (strcmpi(p(end+(-3:0)),'.oct'))
    %% p is a path to .OCT file
    %This is a .OCT file, success, single file case
	OCTFolders = {p}; 
else
    %% Look insite folder p
    %Get All compressed OCT Files
    try
        % Any fileDatastore request to AWS S3 is limited to 1000 files in 
        % MATLAB 2021a. Due to this bug, we have replaced all calls to 
        % fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
        % 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
        ds=fileDatastore(p,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions','.oct');
        OCTFolders = ds.Files(:);
    catch
        %No .oct
        OCTFolders = {};
    end

    %Get all oct folders in that subfolder
    try
        % Any fileDatastore request to AWS S3 is limited to 1000 files in 
        % MATLAB 2021a. Due to this bug, we have replaced all calls to 
        % fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
        % 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
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

%% If this is not an AWS Case
function [OCTFolders,folderNames] = yOCTGetOCTFoldersInPath_AWS(p)

%Set cridentials
awsSetCredentials(1); %This will be done with CLI
p = awsModifyPathForCompetability(p,true);

%Get bucket's name and path of initial folder
i = find(p=='/',3,'first');
bucket = p((i(2)+1):(i(3)-1)); %'delazerdamatlab'
awsStartPath = [p((i(3)+1):end)]; %'Users/Jenkins/'
fullPathPrefix = p(1:(i(3)));

%Make sure awsStartPath ends with '/' and only one and no duplicates inside
awsStartPath = [awsStartPath '/'];
awsStartPath = strrep(awsStartPath,'//','/');
awsStartPath = strrep(awsStartPath,'//','/');

%% Run CLI Command
[status,output] = ...
    system([...
    'aws s3api list-objects-v2 --bucket ' bucket ' ' ...
    '--prefix "' awsStartPath '" ' ...
    '--query "Contents[?' ... Define a quarry to filter out only paths where OCT data can be found
    'contains(Key, ''.oct'') || '      ... Thorlabs unzipped OCT folder
    'contains(Key, ''Header.xml'') || '... Thorlabs
    '(contains(Key, ''Y0001'') && contains(Key, ''B0001'')) || '... Thorlabs SRR, search for the first scan
    'contains(Key, ''raw_00001.tif'') || '... Wasatch 2D
    '(contains(Key, ''00000_raw_us'')&& contains(Key, ''.bin'')) '... Wasatch 3D
    '].Key']); %.Key specifies we want only the paths not the full spec
%Documentation for the operation above:
%   list-objects-vs - see https://docs.aws.amazon.com/cli/latest/reference/s3api/list-objects-v2.html
%   --querry - see http://jmespath.org/specification.html#or-expressions

%Checkoutput
if (status ~= 0)
    error(['AWS CLI Failed: ' output]);
end

%% Extract Information
%Loop over all this potential to extract folder path
potentialDirs = jsondecode(output);
OCTFolders = cell(size(potentialDirs));
folderNames = OCTFolders;
for i=1:length(potentialDirs)
    
    if(lower(potentialDirs{i}(end+(-3:0))) == '.oct')
        %This is an OCT zipped folder, add as is
        OCTFolders{i} = [fullPathPrefix potentialDirs{i}];
        
        [~,tmp] = fileparts(potentialDirs{i});
        folderNames{i} = [tmp];
    else
        %potentialDirs contains a pointer to a file inside the folder
        tmp = fileparts(potentialDirs{i});
        OCTFolders{i} = [fullPathPrefix tmp '/'];
        
        tmp = split(tmp,'/');
        folderNames{i} = tmp{end};
    end
    
end