function yOCTWriteBigVolume(bv,dim, bigVolumeFolder,fileExtensions)
%Write the datastructure of big volume to drive (locally or remotly)
%INPUTS:
%   bv - tall array with the data, same dimensions as yOCTReadBigVolume
%   dim - dimensions structure - set to [] if doesn't exist
%   bigVolumeFolder - where to save it. If the folder is not empty prior to
%       saving, will delete folder
%   fileExtensions - what file extensions would you like. Default: Tif

if ~exist('fileExtensions','var') || isempty(fileExtensions)
    fileExtensions = 'tif';
end

%% Path
if awsIsAWSPath(bigVolumeFolder)
    awsSetCredentials;
    bigVolumeFolder = awsModifyPathForCompetability(bigVolumeFolder,false);
end

%Is folder empty?
awsRmDir(bigVolumeFolder);

%% Which function to use to save
switch(lower(fileExtensions))
    case 'tif'
        f = @yOCT2Tif;
        %addAttachedFiles(gcp,'yOCTToTif.m');
    case 'mat'
        f = @yOCT2Mat;
        %addAttachedFiles(gcp,'yOCTToMat.m');
    otherwise
        error('Unknown file extension %s',fileExtensions);
end

%% Save
write([bigVolumeFolder '\y*.' lower(fileExtensions)],bv,'WriteFcn',@(info, data)writeFunctionBV(info,data,f));

%JSON
if ~isempty(dim)
    awsWriteJSON(dim,[bigVolumeFolder '\config.json']);
end

function writeFunctionBV(info,data,f)
fn = strrep(info.RequiredFilePattern,'*',sprintf('%04d',info.PartitionIndex));
filename = [info.RequiredLocation '\' fn];%Remove required pattern, its easier that way
f(data,filename);
