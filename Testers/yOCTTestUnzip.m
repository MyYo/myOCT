function yOCTTestUnzip(TestVectorFolder)
%This tester tests unzipping capabilities

tic
%Determine if we are running on AWS
if (strcmpi(TestVectorFolder(1:3),'s3:'))
    isAWS = true;
else
    isAWS = false;
end

%Get list of tests
folders = yOCTGetOCTFoldersInPath (TestVectorFolder);

didFoundZippedFile = false;
for i=1:length(folders)
    if (strcmpi(folders{i}(end+(-3:0)),'.oct'))
        didFoundZippedFile = true;
        
        inFolder = folders{i};
        if isAWS
            outFolder = [folders{i}(1:(end-4)) 'Unzipped'];
        else
            outFolder = 'myLocalFolder';
        end
        disp(['Unzipping: ' inFolder]);
        disp(['To: ' outFolder]);
        
        %Make sure the output folder doesn't exist prior to unzip
        if isAWS
            e=(system(['aws s3 ls "' outFolder '"'])==0); %Does this folder exist?
            if(e)
                disp('Unzipped folder exists, cleanup before running test')
                system(['aws s3 rm "' outFolder '"']);
            end
        else 
            if(exist(outFolder,'dir'))
                disp('Unzipped folder exists, cleanup before running test')
                rmdir(outFolder,'s');
            end
        end
        
        %Unzipp
        yOCTUnzipOCTFolder(inFolder,outFolder,false); %Don't delete unzipped folder, we will use it again later
        
        %Check that a folder was created, meaning unzipping was scucessful
        %& cleanup
        if isAWS
            e=(system(['aws s3 ls "' outFolder '"'])==0); %Does this folder exist?
            if(~e)
                error('Unzipping was unccessfull');
            else
                system(['aws s3 rm "' outFolder '"']); %Ok Cleanup
            end
        else 
            if(~exist(outFolder,'dir'))
                error('Unzipping was unccessfull');
            else
                rmdir(outFolder,'s'); %Ok Cleanup
            end
        end
        
        disp('... Ok');
    end
end

if ~didFoundZippedFile
    error(['Did not find any zipped files in: ' TestVectorFolder]);
end

disp('Unzipping Test Completed Succsfully');
toc


