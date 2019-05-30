function [interferogram, apodization,prof] = yOCTLoadInterfFromFile_ThorlabsSRRData(varargin)
%Interface implementation of yOCTLoadInterfFromFile. See help yOCTLoadInterfFromFile
%Reads SRR files
% OUTPUTS:
%   - interferogram - interferogram data, apodization corrected. 
%       Dimensions order (lambda,x,y,AScanAvg,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - apodization - OCT baseline intensity, without the tissue scatterers.
%       Dimensions order (lambda,podization #,y,BScanAvg). 
%       If dimension size is 1 it does not appear at the final matrix
%   - prof - profiling data - for debug purposes 

%% Input Checks
if (iscell(varargin{1}))
    %the first varible contains a cell with the rest of the varibles, open it
    varargin = varargin{1};
end 

inputDataFolder = varargin{1};
if (strcmpi(inputDataFolder(1:3),'s3:'))
    %Load Data from AWS
    yOCTSetAWScredentials;
    inputDataFolder = myOCTModifyPathForAWSCompetability(inputDataFolder);
end

%Optional Parameters
for i=2:2:length(varargin)
    switch(lower(varargin{i}))
        case 'dimensions'
            dimensions = varargin{i+1};
        otherwise
            %error('Unknown parameter');
    end
end

%% Determine dimensions
[sizeLambda, sizeX, sizeY, AScanAvgN, BScanAvgN] = yOCTLoadInterfFromFile_DataSizing(dimensions);   

apodSize = dimensions.aux.apodend - dimensions.aux.apodstart + 1;

%% Loop over all frames and extract data
%Define output structure
interferogram = zeros(sizeLambda,sizeX,sizeY, AScanAvgN, BScanAvgN);
apodization   = zeros(sizeLambda,apodSize,sizeY,1,BScanAvgN); 
prof.numberOfFramesLoaded = sizeY*dimensions.BScanAvg.indexMax; %We have to load all B-Scans, no partial load is possible as they are written in the same file
prof.totalFrameLoadTimeSec = 0;
for yI=1:sizeY
    td=tic;
    spectralFilePath = sprintf('%s/Data_Y%04d_YTotal%d_BTotal%d_%s.srr',...
        inputDataFolder,...
        dimensions.y.index(yI),dimensions.y.indexMax,...
        dimensions.BScanAvg.indexMax,...
        dimensions.aux.OCTSystem);
    
    %Load Data
    ds=fileDatastore(spectralFilePath,'ReadFcn',@(a)(DSRead(a,dimensions.aux.headerTotalBytes)));
    temp=ds.read;

    if (isempty(temp))
        error(['Missing file / file size wrong' spectralFilePath]);
    end
    prof.totalFrameLoadTimeSec = prof.totalFrameLoadTimeSec + toc(td);
    
    %Reshape, every scan end is a different frame concatinated
    temp = reshape(temp,sizeLambda,dimensions.aux.scanend,[]); %Dimensions are (lambda,scan,frame)

    %Read apodization
    apod = double(temp(:,dimensions.aux.apodstart:dimensions.aux.apodend,:));
    
    %Read interferogram
    interf = double(temp(:,dimensions.aux.scanstart:dimensions.aux.scanend,:)); 
    
    %Save
    apodization(:,:,yI,1,:) = apod(:,:,dimensions.BScanAvg.index);
    interferogram(:,:,yI,:,:) = interf(:,:,dimensions.BScanAvg.index); %Hand pick which B-Scan Avg to get
end

function temp = DSRead(fileName, headerTotalBytes) 
fid = fopen(fileName);

%Skip header
fseek(fid, headerTotalBytes, 'cof'); 

%Read
data = fread(fid,'uint16');

% data is 2 bytes (2^16) but really only ranges from 0-4096 (2^12)
temp = uint16(rem(data, 4096));

%Cleanup
fclose(fid);
