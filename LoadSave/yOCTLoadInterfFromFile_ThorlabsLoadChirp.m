function chirp = yOCTLoadInterfFromFile_ThorlabsLoadChirp(fp)
%This function loads a chirp file for thorlabs system. 
%fp - can be the exact file path of the chirp file or a path to the
%thorlabs datafolder

%% Get file path pointing on to the chirp file
[~,fname] = fileparts(fp);
if isempty(fname)
    %We got a folder pointer, search the folder for the chirp
    inputFolder = fp;
    
    ds=fileDatastore(inputFolder,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions',...
            {...
            '.data' ... Binary version of chirp, usually saved in non SRR version
            '.dat'  ... Text version of chirp, saved in SRR version
            });
        
    if (length(ds.Files) > 1)
        %Go over the files, in 1D mode system will save SpectralFloat.data,
        %its not a chirp file and can be removed
        is1FFile = cellfun(@(x)(contains(x,'SpectralFloat.data')), ds.Files);
        ds.Files(is1FFile) = [];
    end
    
    if (length(ds.Files) > 1)
        error('Found too many chirp files in: %s, cant decide what to do',inputFolder);
    elseif isempty(ds.Files)
        error('Didnt find any chirp files in %s.',inputFolder);
    end
    
else
    ds=fileDatastore(fp,'ReadFcn',@(x)(x));
end

%% Figure out if we are working with binary or text version of the dataset
if (contains(fp,'data'))
    ds.ReadFcn = @readChirpBin;
else
    ds.ReadFcn = @readChirpTxt;
end

%% Read the chirp
chirp = ds.read;

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
