function [OCTSystem, OCTSystemManufacturer] = yOCTLoadInterfFromFile_WhatOCTSystemIsIt(inputDataFolder)
%This function goes to inputDataFolder and figures out which OCT system is
%it (from the OCT systems that are supported by this code package)

%% Input checks
%Modify for compatability, this function works even if the file is local
inputDataFolder = awsModifyPathForCompetability(inputDataFolder,false); %No CLI required

if awsIsAWSPath(inputDataFolder)
    awsSetCredentials;
end
    
%% Select manufacturer by looking at file types
try
ds=fileDatastore(inputDataFolder,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions',...
            {...
            '.xml' ... Thorlabs, search for header.xml
            '.dat' ... Thorlabs SRR, searches for chirp.dat
            '.bin','.tif' ... Wasatch
            });
catch ME
    disp('Couldn''t figure out whath is the manufactuer of the OCT system');
    disp(ME.message);
    
    error('Error in yOCTLoadInterfFromFile_WhatOCTSystemIsIt');
end

%Remove from the file captures general tifs that are not representating
%wasatch data
files = ds.Files;
iToDelete = zeros(size(files));
for i=1:length(files)
    [~,fname] = fileparts(files{i});
    d = sscanf(fname,'%sraw_%d');
    if isempty(d)
        %Dosen't match wasatch format
        iToDelete(i) = 1;
    end
end
files(iToDelete) = [];

%Parse file options
isThorlabs = cellfun(@(x)(contains(x,'.xml')),files);
isThorlabs_SRR = cellfun(@(x)(contains(x,'.dat')),files);
isWasatch = cellfun(@(x)(contains(x,'.bin')),files) | cellfun(@(x)(contains(x,'.tif')),files);

if(max(isThorlabs) + max(isThorlabs_SRR) + max(isWasatch) > 1)
    error('Could''nt determine OCT system, there are multiple manufactuerrs in this folder');
end

if (max(isThorlabs)>0)
    OCTSystemManufacturer = 'Thorlabs';
end
if (max(isThorlabs_SRR)>0)
    OCTSystemManufacturer = 'Thorlabs_SRR';
end
if (max(isWasatch)>0)
    OCTSystemManufacturer = 'Wasatch';
end

%% Refine to get OCTSystem
switch(OCTSystemManufacturer)
    case 'Wasatch'
        OCTSystem = 'Wasatch'; %We have only one
    case 'Thorlabs'
        
        %Read the content of the html file as a text
        ds=fileDatastore(awsModifyPathForCompetability([inputDataFolder '/Header.xml']),'ReadFcn',@fileread);
        txt = ds.read;
        
        isGanymede = contains(txt,'Ganymed');
        isTelesto = contains(txt,'Telesto');
        
        isOne = isGanymede + isTelesto;
        if (isOne ~= 1)
            error('Couldn''t figure out what OCT system that is')
        end
        
        if (isGanymede == 1)
            OCTSystem = 'Ganymede';
        else
            OCTSystem = 'Telesto';
        end
            
    case 'Thorlabs_SRR'
        
        %Read the name of the first file
        ds=fileDatastore(awsModifyPathForCompetability([inputDataFolder '/Data_Y0001_*B0001*']),'ReadFcn',@(x)(x),'fileExtensions','.srr');
        
        if (length(ds.Files) > 1)
            error('Expected only one first file in dataset, does this folder contain more than one OCT scan?');
        end
        
        [~,fName] = fileparts(ds.Files{1});
        %Scan file name, it should be there
        isGanymede = contains(fName,'Ganymede');
        isTelesto = contains(fName,'Telesto');
    
        if     (  isGanymede && ~isTelesto )
            OCTSystem = 'Ganymede_SRR';
        elseif ( ~isGanymede &&  isTelesto )
            OCTSystem = 'Telesto_SRR';
        else
            error('Cannot determine OCT system SRR');
        end
end

