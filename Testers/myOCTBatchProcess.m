function myOCTBatchProcess(OCTFolders,config)
%OCTFolders path to folder containing a single or multiple OCT folders
%config - key - value cell array for configuration of the excecution 
% Any parameters of yOCTLoadInterfFromFile, yOCTInterfToScanCpx can be used. 
% outputFilePrefix can be set.

%OCTFolders = 's3://delazerda/Yonatan/DiffSpeckleContrast/2018-11-08 In Vivo Mouse with Particles/Scan/Scan_0005_ModeSpeckle';
%OCTFolders = 's3://delazerda/Jenkins/Wasatch_3D';
%OCTFolders = '\\171.65.17.174\MATLAB_Share\Brooke\08NOV_INVIVO\Scan\Test5\';
%OCTFolders = '\\171.65.17.174\MATLAB_Share\Brooke\08NOV_INVIVO\Scan\Scan_0003_ModeSpeckle.oct';

try
%% Setup environment
currentFileFolder = fileparts(mfilename('fullpath'));
yOCTMainFolder = [currentFileFolder '\..\'];
addpath(genpath(yOCTMainFolder)); %Add current files to path
gcp; %Start Parallel Processs
opengl('save', 'software'); %Increase stubility in OPEN GL

%% Make sure we have AWS Cridentials
if (strcmpi(OCTFolders(1:3),'s3:'))
    %Load Data from AWS
    yOCTSetAWScredentials (1); %Write cridentials are required
    isAWS = true;
else
    isAWS = false;
end


%% Get configuration out of config
outputFilePrefix = '';
if exist('config','var')
    for i=1:2:length(config)
       eval([config{i} ' = config{i+1};']); %<-TBD - there should be a safer way
    end
else
    config = {};
end
config = config(:)';

%% Try to figure out what is the path provided by OCTFolders
try
   
    if (strcmpi(OCTFolders(end+(-3:0)),'.oct'))
        %This is a .OCT file
        %Success, single file case
    else
        %Try reading this folder directly, if it is not red then maybe it's
        %a folder of folders
        yOCTLoadInterfFromFile(OCTFolders,'PeakOnly',true);
    end
    
    %Success, single file case
	OCTFolders = {OCTFolders}; 
catch
    %We failed, meaning this folder is not an OCT Folder, maybe it is a folder of folders
    %TBD in future versions
    error('Not yet supported');
end

%Make sure we correct for AWS file names
if isAWS
    for i=1:length(OCTFolders)
        OCTFolders{i} = myOCTModifyPathForAWSCompetability(OCTFolders{i});
    end
end

%% Process
for i=1:length(OCTFolders)
    %See if folder is an .OCT file. if so, unzip it first
    if (strcmpi(OCTFolders{i}(end+(-3:0)),'.oct'))
        disp(['Unzipping ' OCTFolders{i}]);
        OCTFolders{i} = OCTFolders{i}(1:(end-4));
        if (isAWS)
            %We will need to unzip file locally then send back to the
            %cloud. 
                       
            %Download file from AWS
            system(['aws s3 cp "' OCTFolders{i} '.oct" tmp.oct']);
            
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
            system(['aws s3 sync tmp "' OCTFolders{i} '"']);
            %system(['aws s3 cp tmp\data "' OCTFolders{i} '/data" --recursive']);
            
            %Remove '.oct' file
            system(['aws s3 rm "' OCTFolders{i} '.oct"']);
        else
            %Unzip to the same folder it came from
            
            %Unzip using 7-zip
            if exist('C:\Program Files\7-Zip\','dir')
                system(['"C:\Program Files\7-Zip\7z.exe" x "' OCTFolders{i} '.oct" -o"' OCTFolders{i} '"']);
            elseif exist('C:\Program Files (x86)\7-Zip\','dir')
                system(['"C:\Program Files (x86)\7-Zip\7z.exe" x "' OCTFolders{i} '.oct" -o"' OCTFolders{i} '"']);
            else
                error('Please Install 7-Zip');
            end
            
            %Delete .oct file, we don't need it
            delete([OCTFolders{i} '.oct']);
        end
    end
    
    %Find folder's name
    folderName = split(OCTFolders{i},{'/','\'});
    folderName = folderName(~cellfun(@isempty,folderName)); %Remove empty slots
    folderName = folderName{end};
    
    fprintf('Processing File: %s (%d of %d) ...\n',folderName,i,length(OCTFolders));
        
    %Load OCT Data, parllel computing style
    [meanAbs,speckleVariance] = yOCTProcessScan([{OCTFolders{i}, ...
        {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
        'nYPerIteration', 1, ...
        'showStats',true} config]);
    
    %Modify Speckle Variance
    speckleVariance(meanAbs<3) = 0; %Threshlod
    speckleVariance(1:50,:,:) = 0; %Top of the image is usually noise
    speckleVariance = speckleVariance./meanAbs;

    %Save data
    yOCT2Tif(mag2db(meanAbs),[OCTFolders{i} '/' outputFilePrefix 'scanAbs.tif']); %Save to File
    yOCT2Tif(speckleVariance,[OCTFolders{i} '/' outputFilePrefix 'speckleVariance.tif']); %Save to File
    
    %Save Overview files
    yOCT2Tif(mag2db(squeeze(meanAbs(:,:,1))),[folderName outputFilePrefix 'BScan_1.tif']);
    yOCT2Tif(mag2db(squeeze(meanAbs(:,:,ceil(end/2)))),[folderName outputFilePrefix sprintf('BScan_%d.tif',ceil(size(meanAbs,3)/2))]);
    yOCT2Tif(mag2db(squeeze(meanAbs(:,:,end))),[folderName outputFilePrefix sprintf('BScan_%d.tif',size(meanAbs,3))]);
    
    %Max Projection
    mp = squeeze(max(speckleVariance,[],1)); %Project along z
    yOCT2Tif(mp,[folderName outputFilePrefix 'speckleVarMaxProjection.tif']);
end

%% Upload data to the colud
%In later versions
%Regreplace \ with /

%Where files are located in the cloud
fid = fopen('WhereAreMyFiles.txt','w'); 
fprintf(fid,'here');
fclose(fid);
%TBD

%% Safe exist
catch ME
    for i=1:length(ME.stack)
        ME.stack(i)
    end
    disp(ME.message); %Write
    exit(1); %Problem Happend
end
exit(0); 