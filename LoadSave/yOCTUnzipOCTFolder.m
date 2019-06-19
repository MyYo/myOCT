function yOCTUnzipOCTFolder(OCTFolderZipFileIn,OCTFolderOut,isDeleteOCTZippedFile)
%This function unzips .oct file (OCTFolderZipFileIn) into OCTFolderOut, and
%deletes OCTFolderZipFileIn (default)

%% Make sure we have AWS Cridentials
if (strcmpi(OCTFolderZipFileIn(1:3),'s3:'))
    yOCTSetAWScredentials (1); %Write cridentials are required  
    OCTFolderZipFileIn = myOCTModifyPathForAWSCompetability(OCTFolderZipFileIn);
    isAWSIn = true;
else
    isAWSIn = false;
end
if (strcmpi(OCTFolderOut(1:3),'s3:'))
    yOCTSetAWScredentials (1); %Write cridentials are required
    OCTFolderOut = myOCTModifyPathForAWSCompetability(OCTFolderOut);
    isAWSOut = true;
else
    isAWSOut = false;
end

if ~exist('isDeleteOCTFolderIn','var')
    isDeleteOCTZippedFile = true;
end

%% Setup input directory
if (isAWSIn)
    %We will need to unzip file locally

    %Download file from AWS
    system(['aws s3 cp "' OCTFolderZipFileIn '" tmp.oct']);

    if ~exist('tmp.oct','file')
        error('File did not download from AWS');
    end
    
    OCTFolderZipFileIn = 'tmp.oct';
end

%% Setup output directory
OCTUnzipToDirectory = OCTFolderOut;
if (isAWSOut)
    %Destination is at the cloud, need to unzip locally
    OCTUnzipToDirectory = 'tmp';
end
if strcmp(OCTFolderOut(1:2),'\\')
    %We are trying to unzip to a netowrk dirve, this is not a good idea
    %so we shall unzip to a local drive, than move to network drive
    OCTUnzipToDirectory = 'tmp';
end

%Check Unziped to directory is empty
if exist(OCTUnzipToDirectory,'dir')
    %Delete directory first
    rmdir(OCTUnzipToDirectory,'s');
end

%% Preform Unzip

%Unzip using 7-zip
if exist('C:\Program Files\7-Zip\','dir')
    system(['"C:\Program Files\7-Zip\7z.exe" x "' OCTFolderZipFileIn '" -o"' OCTUnzipToDirectory '"']);
elseif exist('C:\Program Files (x86)\7-Zip\','dir')
    system(['"C:\Program Files (x86)\7-Zip\7z.exe" x "' OCTFolderZipFileIn '" -o"' OCTUnzipToDirectory '"']);
else
    error('Please Install 7-Zip');
end

%Check unzip was successfull
if ~exist(OCTUnzipToDirectory,'dir')
    error('Failed to Unzip');
end

%% Upload if necessary
if ~strcmp(OCTUnzipToDirectory,OCTFolderOut)
    %Unzipped directory different from output folder, it means we need to
    %upload
    if(isAWSOut)
        %Upload to bucket
        system(['aws s3 sync "' OCTUnzipToDirectory '" "' OCTFolderOut '"']);
        %system(['aws s3 cp tmp\data "' OCTFolders{i} '/data" --recursive']);
        
        %Cleanup, delete temporary directory
        rmdir(OCTUnzipToDirectory,'s'); 
    else
        %File system copy
        [a,b,c] = movefile(OCTUnzipToDirectory,OCTFolderOut,'f');
        a
        b
        c
    end
    
end

%% Remove zipped archive if required (.OCT file)
if isDeleteOCTZippedFile  
    if (isAWSIn)
        system(['aws s3 rm "' OCTFolderZipFileIn '"']);
    else
        delete(OCTFolderZipFileIn);
    end
end