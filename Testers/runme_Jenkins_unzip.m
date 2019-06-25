function runme_Jenkins_unzip(OCTFolders)
 %% Preprocess: unzipping 
    %Get what OCT Folders are in the path provided
    [OCTFolders,folderNames] = yOCTGetOCTFoldersInPath (OCTFolders);
    if (isempty(OCTFolders))
        error([ OCTFolders ' Does not have any OCT files or folders']);
    end

 %See if folder is an .OCT file. if so, unzip it first
    for i=1:length(OCTFolders)   
        if (strcmpi(OCTFolders{i}(end+(-3:0)),'.oct'))
            disp(['Unzipping ' OCTFolders{i}]);
            yOCTUnzipOCTFolder(OCTFolders{i},OCTFolders{i}(1:(end-4)));
            OCTFolders{i} = OCTFolders{i}(1:(end-4));
            folderNames{i} = folderNames{i}(1:(end-4));
        end
    end
end

