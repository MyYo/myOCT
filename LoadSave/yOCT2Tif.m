function yOCT2Tif (varargin)
% This function saves a grayscale version of data to a Tiff stack file.
% There are a few options to save:
%   1) Save to a single large tif file, good for ImageJ viewing, less good
%       for parallel processing. To use this option filepath should end
%       with .tif
%   2) Save to a tif directory, where each y plane is a single plane,
%       creating smaller but many tif files.
%   2.1) Partial data mode
% Dimensions are (z,x) and each frame is y
% USAGE:
%   yOCT2Tif(data, filePath, [paramName, paramValue])
% INPUTS:
%   - data - 2D or 3D matrix to be saved to file. Dimensions of data are
%       (Z by X by Y). Each tif file will be Z by X and tiff stack will
%       will contain Y planes.
%   - filePath - where to save tiff file, this path can be:
%       1) Tif file path (ends with .tif) if you would like to save data to
%           a single tif file (or tif stack if 3D volume is provided)
%       2) Folder path. In this case, each Y plane will be saved as a
%           seperate file in the folder
%       3) Cell array with one tif file path and one tif folder path if you
%           would like to save both.
%           For example {'myfile.tif','myFolder\'} will generate both.
%       path can be local or AWS s3 path
% OPTINAL INPUTS: (entered as parameter name, value)  
%   'clim' - [min, max] of the grayscale, default will be minimum and
%       maximum of the data
%   'metadata' - i.e. dimention structure, to be saved alongside with the data
%       metadata is ignoted if partialFileMode = 1
%   'partialFileMode' - can be 0 (not active), 1 and 2. If partialFileMode
%       is 1, please set partialFileModeIndex. See partial mode below
%   'partialFileModeIndex' - index along the y axis (each y is saved in a
%       different tif file) that data is assocated with
%
% PARTIAL FILE MODE - EXPLENATION:
% In case we would like to process an OCT file which is much larger than 
% can be stored in memory, or we would like to process OCT file in parfor, 
% use this function. The general schematic for processing will be:
%   parfor bScanY=1:n %Loop over all B-Scans
%       resultScan = <Process B Scan> %Result scan dimensions are (y,x)
%       yOCT2Tif('C:\myOCTVolume\',resultScan, ...
%       'partialFileMode',1,'partialFileModeIndex',bScanY); %Save a scan 
%   end
%   yOCT2Tif('C:\myOCTVolume\',[],'partialFileMode',2); % Finalize saving


%% Input Processing
p = inputParser;
addRequired(p,'data',@isnumeric);
addRequired(p,'filePath',@(x)(ischar(x) | iscell(x)));

addParameter(p,'clim',[])
addParameter(p,'metadata',[])
addParameter(p,'partialFileMode',0);
addParameter(p,'partialFileModeIndex',[]);

parse(p,varargin{:});
in = p.Results;
data = in.data;
filePath = in.filePath;
c = in.clim;
metadata = in.metadata;

% Partial Mode checks
mode = in.partialFileMode;
if (mode == 1)
    if (isempty(in.partialFileModeIndex))
        error('Please specify partialFileModeIndex when working in partialFileMode=1');
    elseif (length(in.partialFileModeIndex) ~= size(data,3))
        error('Please make sure partialFileModeIndex is the same as data 3 axis');
    end
end

%% Figure out what the output format is
outputFilePaths = cell(2,1); %(1) - file path, (2) - folder path if needed

% What to output
if ischar(filePath)
    filePath = {filePath};
end
if (length(filePath)>2)
    error('Can output one file and or one folder but no more than that');
end

for i=1:length(filePath)
    [~,~,f] = fileparts(filePath{i});
    
    if (~isempty(f))
        %File
        if ~isempty(outputFilePaths{1})
            error('Can only output one file!');
        end
        outputFilePaths(1) = filePath(i);
    else
        %Folder
        if ~isempty(outputFilePaths{2})
            error('Can only output one folder!');
        end
        outputFilePaths{2} = awsModifyPathForCompetability([filePath{i} '/']);
    end
end

isOutputFile = ~isempty(outputFilePaths{1});
isOutputFolder = ~isempty(outputFilePaths{2});

if ~isOutputFolder
    %Generate a folder path name from the file, just in case
    outputFilePaths{2} = awsModifyPathForCompetability([ ...
        outputFilePaths{1} '.fldr/']);
end

%% If upload to AWS, make arrengements
if (awsIsAWSPath(filePath))
    isAWS = true;
    
    if mode == 0 || mode == 2
        awsSetCredentials(1); %Use the advanced version as uploading is more challenging
    elseif mode == 1
        awsSetCredentials(0); %Use the advanced version as uploading is more challenging
    end
    
    %We will use this path for AWS CLI
    awsOutputFilePath{1} = awsModifyPathForCompetability(outputFilePaths{1},true); 
    awsOutputFilePath{2} = awsModifyPathForCompetability(outputFilePaths{2},true); 
    
    if (mode == 0) % Save locally
        outputFilePaths{1} = [tempname '.tif']; %Temporary local file path
        outputFilePaths{2} = [tempname '\']; %Temporary local file path
    else
        outputFilePaths = awsOutputFilePath;
    end
else
    isAWS = false;
end

%% Actuall writing of data, regular mode
if mode == 0
    % clim
    if isempty(c)
        c = [min(data(:)) max(data(:))];
    end
    
    % encode meta data
    metaJson = GenerateMetaData(metadata,c);
    
    for yI=1:size(data,3)
        bits = data2bits(data(:,:,yI),c);
        
        % Save file
        if isOutputFile
            if (yI==1)
                imwrite(bits,outputFilePaths{1},...
                    'Description',jsonencode(metaJson) ... Description contains min & max values
                    );
            else
                imwrite(bits,outputFilePaths{1},...
                    'writeMode','append');     
            end
        end
        if isOutputFolder
            if (yI==1)
                awsWriteJSON(metaJson, ...
                    [outputFilePaths{2} '/TifMetadata.json']);
            end
            p = yScanPath(outputFilePaths{2},yI);
            imwrite(bits,p);
        end
    end
    
    % At the end of the loop, upload to AWS if needed
    if (isAWS && isOutputFile)
        awsCopyFileFolder(outputFilePaths{1},awsOutputFilePath{1});
        delete(outputFilePaths{1}); %Cleanup
    end
    if (isAWS && isOutputFolder)
        awsCopyFileFolder(outputFilePaths{2},awsOutputFilePath{2});
        rmdir(outputFilePaths{2},'s'); %Cleanup
    end

%% Actual writing of data, partial file mode (loop part)
elseif mode == 1
    % clim
    if isempty(c)
        c = [min(data(:)) max(data(:))];
    end
    
    for yI=1:size(data,3)
        bits = data2bits(data(:,:,yI),c);
        
        p = yScanPath(outputFilePaths{2},in.partialFileModeIndex(yI));
        
        % Save Tif stack file
        tn1 = [tempname '.tif'];
        imwrite(bits,tn1);
        awsCopyFile_MW1(tn1,p); ...
        delete(tn1); % Cleanup   
    
        % Save C as a temp json
        tn2 = [tempname '.json'];
        a.c = c;
        awsWriteJSON(a,tn2);
        awsCopyFile_MW1(tn2,[p '.json']); ...
        delete(tn2); % Cleanup   
    end
    
%% Actual writing of data, partial file mode (finalization part)
else
    % Finish WM work
    awsCopyFile_MW2(outputFilePaths{2});
    
    %Get all the JSON files, so we can read c
    dsJsons = fileDatastore(outputFilePaths{2},'ReadFcn',@awsReadJSON, ...
        'FileExtensions','.json'); 
    cJsons = dsJsons.readall();
    cmin = cellfun(@(x)(min(x.c)),cJsons);
    cmax = cellfun(@(x)(max(x.c)),cJsons);
    
    if isempty(c)
        c = [min(cmin) max(cmax)];
    end
   
    % Clear Json files that contain c, they are no longer required
    for i=1:length(dsJsons.Files)
        awsRmFile(dsJsons.Files{i});
    end
    
    % Rewrite individual slides with the same c boundray for all
    parfor(parforI=1:1,1) %Run once but on a worker
        % Make sure worker has the right credentials
        awsSetCredentials;
        
        for frameI = 1:length(cmin)
            % Read frame
            fp = awsModifyPathForCompetability(sprintf('%s/y%04d.tif',...
                outputFilePaths{2},frameI)); 
            ds = fileDatastore(fp,'readFcn',@imread);
            dat = ds.read();
            
            % Delete existing frame
            awsRmFile(fp);
           
            % Write a new frame
            tn = [tempname '.tif'];
            imwrite( ...
                uint16(...
                (double(dat)*(cmax(frameI)-cmin(frameI)) + cmin(frameI) - c(1))/diff(c) ...
                ),tn);
            awsCopyFile_MW1(tn,fp); %Matlab worker version of copy files
            delete(tn);
        end 
    end %Run once but on a worker
    awsCopyFile_MW2(outputFilePaths{2});
    
    % Finish generating a folder by placing metadata
    metaJson = GenerateMetaData(metadata,c);
    awsWriteJSON(metaJson, ...
                [outputFilePaths{2} '/TifMetadata.json']);
    
    % Generate a single file if required
    if isOutputFile
        outputFileTmpFolder = awsModifyPathForCompetability([outputFilePaths{2} 'all\all.tif']);
        parfor(parforI=1:1,1) %Run once but on a worker
            % Load data
            dat = yOCTFromTif(outputFilePaths{2});
            
            % Save it as a single file
            tn = [tempname '.tif'];
            yOCT2Tif(dat,tn,'clim',c,'metadata',metadata);
            awsCopyFile_MW1(tn,outputFileTmpFolder); %Matlab worker version of copy files
            delete(tn);

        end
        awsCopyFile_MW2([outputFileTmpFolder '/../']);
        awsCopyFileFolder(outputFileTmpFolder,outputFilePaths{1});
        awsRmDir([outputFileTmpFolder '/../']);
    end
    
    % If output folder is not required, delete it
    if ~isOutputFolder
        awsRmDir(outputFilePaths{2});
    end

end

function bits = data2bits(data,c)
maxbit = 2^16-1;
bits = uint16((squeeze(data)-c(1))/(c(2)-c(1))*maxbit);
bits(bits>maxbit) = maxbit;
bits(bits<0) = 0;
bits(isnan(bits)) = 0;

function p = yScanPath(outputFilePaths,yIndex)
p = awsModifyPathForCompetability(... 
    sprintf('%s/y%04d.tif', outputFilePaths,yIndex));

function metaJson = GenerateMetaData(metadata,c)
meta.metadata = metadata;
meta.clim = c;
meta.version = 3;
metaJson = meta;