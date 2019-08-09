function [bv,dim] = yOCTReadBigVolume(bigVolumeFolder,fileExtensions)
%This datastructure uses parallel toolbox (tall) to load a volume 
%(scan abs or speckle variance for example) which is larger then RAM, 
%or in cases where big volume is on the cloud, and processing can be done
%on the cloud as well.
%INPUTS:
%   bigVolumeFolder - Folder where the volume is saved
%   fileExtensions - can be .tif or .mat
%OUTPUT:
%   bv - data, dimensions are (y,x,z)
%   dim - dimensions structure 
%To create a BV from scatch, save data in a folder, each BScan has its own
%file (use yOCT2Tif or yOCT2Mat). 
%For example
%   if we have a folder 'MyMat' that contains 
%       'y0001.mat','y0002.mat', ...
%       contains 'dim.json'
%   We can use yOCTReadBigVolume('MyMat','mat') to load a big volume

%% Path
if awsIsAWSPath(bigVolumeFolder)
    awsSetCredentials;
    bigVolumeFolder = awsModifyPathForCompetability(bigVolumeFolder,false);
end

%% Read big Volume
switch(lower(fileExtensions))
    case 'tif'
        ds = fileDatastore(bigVolumeFolder,'ReadFcn',@yOCTFromTif,'FileExtensions','.tif','IncludeSubfolders',true);   
    case 'mat'
        ds = fileDatastore(bigVolumeFolder,'ReadFcn',@yOCTFromMat,'FileExtensions','.mat','IncludeSubfolders',true);   
    otherwise
        error('Unknown file extension %s',fileExtensions);
end

T = tall(ds);

%Modify dimensions of T such that at the end it will be (y,z,x);
T = cellfun(@(t) permute(t, [3, 1, 2]), T, 'UniformOutput', false);
bv = cell2mat(T);

%% Read dim if it exist
try
    ds2 = fileDatastore(bigVolumeFolder,'ReadFcn',@awsReadJSON,'FileExtensions','.json','IncludeSubfolders',true);
    dim = ds2.read();
catch
    dim = [];
end

