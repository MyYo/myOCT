function dimensions = yOCTLoadInterfFromFile_ThorlabsHeaderLambda(inputDataFolder,OCTSystem)
%This script figures out what is lambda of the data folder

%% Figure out basic Parameters

%To get lambda min & max we contacted thorlabs
switch(OCTSystem)
    case 'Ganymede'
        chirpFileName = 'ChirpGanymede.dat';
        lambdaMin = 824.16;%[nm]
        lambdaMax = 1038.03;%[nm]
    case 'Telesto' 
        chirpFileName = 'ChirpTelesto.dat';
        lambdaMin = 1208.69;%[nm] Manufacturer number: 1200.56, See 2018-12-04 Filter Calibration Report for more details
        lambdaMax = 1372.50;%[nm] Manufacturer number: 1367.75, See 2018-12-04 Filter Calibration Report for more details
    
    otherwise
        error('Unknown OCT System, please specify as input ''OCTSystem''')
end

%% Load Chirp File

try
    %try
        chirp = getChirpFile();
    %catch
    %    chirp = getChirpFile(); %Try a second time, cloud sometimes doesnt work the first time around
    %end
catch
    %Couldn't load chirp file, try loading a local file instead
    warning('Could not find chirp file in OCT folder, loading local file instead');
    currentFileFolder = [fileparts(mfilename('fullpath')) '/'];
    if exist([currentFileFolder chirpFileName]','file')
        ds=fileDatastore([currentFileFolder chirpFileName],'ReadFcn',@readChirpTxt);
        chirp = ds.read;
    else
        error('Did not find chirp file');
    end
end

%% Compute and save lambda

%We belive that chirp describes the corrections for k. meaning
%k(i) = chirp(i)*A+B. Since lambda = 2*pi/k = 2*pi/(chirp*A+B) then
lambda = 1./( ...
            chirp .* ( 1/(length(chirp)-1)*(1/lambdaMax-1/lambdaMin) )+... chirp*A/2pi
            1/lambdaMin ... B/2pi
        );

dimensions.lambda.order  = 1;
dimensions.lambda.values = lambda;
dimensions.lambda.values = dimensions.lambda.values(:)';
dimensions.lambda.units = 'nm [in air]';
end

%% Sub functions
function chirp = readChirpTxt(fp)
    fid = fopen(fp);
    chirp = textscan(fid,'%f');
    chirp = chirp{1};
    fclose(fid);
end
function chirp = readChirpBin(fp)
    fid = fopen(fp);
    chirp  = fread(fid, 'float32');
    fclose(fid);
end

function chirp = getChirpFile()

try
    ds=fileDatastore([inputDataFolder '/data/Chirp.data'],'ReadFcn',@readChirpBin);
catch
    try
        %Try another name
        ds=fileDatastore([inputDataFolder 'Chirp.dat'],'ReadFcn',@readChirpTxt);
    catch
        %Try with lower case
        ds=fileDatastore([inputDataFolder '/data/chirp.data'],'ReadFcn',@readChirpBin);
    end
end
chirp = ds.read;
end