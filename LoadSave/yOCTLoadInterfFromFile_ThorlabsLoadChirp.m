function chirp = yOCTLoadInterfFromFile_ThorlabsLoadChirp(fp)
%This function loads a chirp file for thorlabs system. 
%fp - can be the exact file path of the chirp file or a path to the
%thorlabs datafolder

%% Get file path pointing on to the chirp file
[~,fname] = fileparts(fp);
if isempty(fname)
    %We got a folder pointer, search the folder for the chirp
    inputFolder = fp;
    
    % Any fileDatastore request to AWS S3 is limited to 1000 files in 
    % MATLAB 2021a. Due to this bug, we have replaced all calls to 
    % fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
    % 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
    ds=fileDatastore(inputFolder,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions',...
            {...
            '.data' ... Binary version of chirp, usually saved in non SRR version
            '.dat'  ... Text version of chirp, saved in SRR version
            });
  
    %See files that needs to be discarded
    useFile = zeros(size(ds.Files));
    for i=1:length(useFile)
        [~,fname] = fileparts(ds.Files{i});
        
        %See if fname is part of the files that are not chirp
        if ( strcmpi(fname,'chirp') || strcmpi(fname,'chrp'))
            useFile(i) = true;
        end
    end
    ds.Files(~useFile) = [];
    
    if (length(ds.Files) > 1)
        error('Found too many chirp files in: %s, cant decide what to do',inputFolder);
    elseif isempty(ds.Files)
        error('Didnt find any chirp files in %s.',inputFolder);
    end
    
else
    % Any fileDatastore request to AWS S3 is limited to 1000 files in 
    % MATLAB 2021a. Due to this bug, we have replaced all calls to 
    % fileDatastore with imageDatastore since the bug does not affect imageDatastore. 
    % 'https://www.mathworks.com/matlabcentral/answers/502559-filedatastore-request-to-aws-s3-limited-to-1000-files'
    ds=fileDatastore(fp,'ReadFcn',@(x)(x));
end

%% Read the chirp
%First try as text
ds.ReadFcn = @readChirpTxt;
chirp = ds.read;

if (length(chirp) < 2)
    %Text reading faild, try binary
    reset(ds);
    ds.ReadFcn = @readChirpBin;
    chirp = ds.read;
end

end

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
