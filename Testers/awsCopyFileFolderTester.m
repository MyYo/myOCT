%This function tests aws copy file folder functionality

baseFolder = 'tmp';

%% Generate temporary files to copy
foldersName = { ...
    [baseFolder '\fold1\a'], ...
    [baseFolder '\fold1\b'], ...
    [baseFolder '\fold2\a'], ...
    [baseFolder '\fold2\b'], ...
    };

for j=1:length(foldersName)
    mkdir(foldersName{j});
end

for i=1:30
    a  = uint8(rand(500*500,1)*255);
    %Save a copy in each folder
    for j=1:length(foldersName)
        fid = fopen (sprintf('%s\\%02d.dat',foldersName{j},i),'w');
        fwrite(fid,a);
        fclose(fid);
    end
end

%% Concatinate
fc = [baseFolder '\fold1'];
bigFilesNames = awsCopyFileFolder_ConcatFiles (fc,fc,[],[],10000);

%% Verify files
fc = [baseFolder '\fold2'];

%Get all the files in fold2, to make sure all of them exist in big files
d = dir([fc '\**\*.*']);
d([d.isdir]) = []; %Delete all directories
nFilesLeftToObserve = length(d);

%Loop over all big files and verify file by file
fprintf('Checking %d files\n',nFilesLeftToObserve);
for i=1:length(bigFilesNames)
    bigName = bigFilesNames{i};
    
    %Process Text file 
    fid = fopen([bigName '.txt'],'r');
    c = textscan(fid,'%f %f %[^\n]\n','headerlines',1);
    fclose (fid);
    
    %Loop over all lines and compare big file with regular file
    for fileI=1:length(c{1})
        fStart = c{1}(fileI);
        fEnd = c{2}(fileI);
        fp = [fc c{3}{fileI}]; %Filepath of the original file

        %Read concatinatead version
        fid = fopen([bigName '.bin'],'r');
        fseek(fid,fStart-1,'bof');
        dat1 = fread(fid,fEnd-fStart+1,'ubit8');
        fclose(fid);

        %Read original version
        fid = fopen(fp,'r');
        dat2 = fread(fid,'ubit8');
        fclose(fid);

        if (sum(dat1 ~= dat2)>1)
            error('Big: %d, File %d:  %s Not the same',i,fileI,c{3}{fileI});
        end
        nFilesLeftToObserve = nFilesLeftToObserve-1;
        %disp('same');
    end
end
if (nFilesLeftToObserve~=0)
    error('Not all are present in both real folder and big');
end
disp('Check done, all the same');

%% Cleanup
rmdir('tmp','s');