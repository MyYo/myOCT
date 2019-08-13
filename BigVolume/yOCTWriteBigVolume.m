function yOCTWriteBigVolume(bv,dim, bigVolumeFolder,fileExtensions,c)
%Write the datastructure of big volume to drive (locally or remotly)
%INPUTS:
%   bv - tall array with the data, same dimensions as yOCTReadBigVolume
%   dim - dimensions structure - set to [] if doesn't exist
%   bigVolumeFolder - where to save it. If the folder is not empty prior to
%       saving, will delete folder
%   fileExtensions - what file extensions would you like. Default: Tif
%   c - optional for writing a tiff

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
    case 'mat'
        if ~exist('c','var')
            f = @yOCT2Mat;
        else
            f = @(data,fp)yOCT2Mat(data,fp,c);
        end
    otherwise
        error('Unknown file extension %s',fileExtensions);
end

%% Size
sz = length(dim.z.values);
sx = length(dim.x.values);
s = [sz sx];

%% Save
write([bigVolumeFolder '\y*.' lower(fileExtensions)],bv,'WriteFcn',@(info, data)writeFunctionBV(info,data,f,s));

%JSON
awsWriteJSON(dim,[bigVolumeFolder '\config.json']);


function writeFunctionBV(info,data,f,sz)
fn = strrep(info.RequiredFilePattern,'*',sprintf('%04d',info.PartitionIndex));
filename = [info.RequiredLocation '/' fn];%Remove required pattern, its easier that way
info
filename
f(reshape(data,sz),filename);
