function yOCTUnzipOCTFolder(OCTFolderIn,OCTFolderOut,isDeleteOCTFolderIn)
%This function unzips .oct file (OCTFolderIn) into OCTFolderOut, and
%deletes OCTFolderIn (default)
%Notice that both OCTFolderIn and OCTFolderOut should be either both local
%or both in AWS

%% Make sure we have AWS Cridentials
if (strcmpi(OCTFolderIn(1:3),'s3:') || strcmpi(OCTFolderOut(1:3),'s3:'))
    yOCTSetAWScredentials (1); %Write cridentials are required
    
    OCTFolderIn = myOCTModifyPathForAWSCompetability(OCTFolderIn);
    OCTFolderOut = myOCTModifyPathForAWSCompetability(OCTFolderOut);
    isAWS = true;
else
    isAWS = false;
end

if ~exist('isDeleteOCTFolderIn','var')
    isDeleteOCTFolderIn = true;
end

if (isAWS)
    %% Unzip from cloud
    %We will need to unzip file locally then send back to the
    %cloud. 

    %Download file from AWS
    system(['aws s3 cp "' OCTFolderIn '" tmp.oct']);

    if ~exist('tmp.oct','file')
        error('File did not download from AWS');
    end

    %Unzip using 7-zip
    if exist('C:\Program Files\7-Zip\','dir')
        system('"C:\Program Files\7-Zip\7z.exe" x "tmp.oct" -o"tmp"');
    elseif exist('C:\Program Files (x86)\7-Zip\','dir')
        system('"C:\Program Files (x86)\7-Zip\7z.exe" x "tmp.oct" -o"tmp"');
    else
        error('Please Install 7-Zip');
    end

    if ~exist('tmp','dir')
        error('Faild to unzip');
    end

    %Upload to bucket
    system(['aws s3 sync tmp "' OCTFolderOut '"']);
    %system(['aws s3 cp tmp\data "' OCTFolders{i} '/data" --recursive']);

    if isDeleteOCTFolderIn
        %Remove '.oct' file
        system(['aws s3 rm "' OCTFolderIn '"']);
    end
    
    rmdir('tmp','s'); %Cleanup
else
    %% Unzip from localy 
    %Unzip to the same folder it came from

    %Unzip using 7-zip
    if exist('C:\Program Files\7-Zip\','dir')
        system(['"C:\Program Files\7-Zip\7z.exe" x "' OCTFolderIn '" -o"' OCTFolderOut '"']);
    elseif exist('C:\Program Files (x86)\7-Zip\','dir')
        system(['"C:\Program Files (x86)\7-Zip\7z.exe" x "' OCTFolderIn '" -o"' OCTFolderOut '"']);
    else
        error('Please Install 7-Zip');
    end

    if isDeleteOCTFolderIn
        %Delete .oct file, we don't need it
        delete(OCTFolderIn);
    end
end