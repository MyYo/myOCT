function awsCopyFile_MW2(dest)
%See awsCopyFile_MW1 for documentation
%Will go over all folders in dest to get files out!

%% Input check
if awsIsAWSPath(dest)
    awsSetCredentials(1); %We need CLI
end
dest = awsModifyPathForCompetability(dest);

%% Get all the files that need loving

ds=fileDatastore(dest,'ReadFcn',@(x)(x),'IncludeSubfolders',true,'FileExtensions',{'.getmeout'});
files = ds.Files;
froms = cell(size(files));
tos   = cell(size(files));
for i=1:length(files)
    froms{i} = files{i};
    
    %To get the real file name, go up a few folders
    f1 = fileparts(from);
    to = fileparts([f1 '.']);
    
    tos{i} = to;
end

%% Preform the move
if (awsIsAWSPath(dest))
    
    if true
        %Parallel Version
        awsCopyFile_MW2_AWSParallel(froms,tos);
    else
        %Sync version
        for i=1:length(forms)
            cmd = sprintf('aws s3 mv "%s" "%s"',froms{i},tos{i});
            [status,txt] = system(cmd);
            if status~=0
                error('Could not move files, message: %s',txt);
            end
        end
    end
else
    for i=1:length(froms)
        tmpfn = tempname;
        movefile(froms{i},tmpfn,'f');
        rmdir(to,'s');
        movefile(tmpfn,tos{i},'f');
    end
end

function awsCopyFile_MW2_AWSParallel(froms,tos)
%A good way to test the function below is with the following code:
%sourceDir = 's3://<mybucket>/...';
%destDir   = 's3://<mybucket2>/..';
%ds = fileDatastore(sourceDir,'FileExtensions','.mat','ReadFcn',@(x)(x));
%awsCopyFile_MW2_AWSParallel(ds.Files,destDir);


%% Input Checks
if (length(tos) == 1)
    to = tos;
    tos = cell(size(froms));
    for i=1:length(tos)
        tos(i) = to;
    end
end

%% Make the commands
cmd = cell(size(froms));
for i=1:length(cmd)
    cmd{i} = sprintf('aws s3 mv "%s" "%s"',froms{i},tos{i});
end

%% Devide commands into betches
maxCmdsPerBetch = min(maxNumCompThreads*4,20);
nBetches = round(length(cmd)/maxCmdsPerBetch);
cmdBatch = mod((1:length(cmd))-1,nBetches)+1; %This will tell each command in what batch it is

%% Run batches
for batchI = 1:nBetches

    %Create a file with all commands
    tmpBatFP = 'tmp.bat';
    fid = fopen(tmpBatFP,'w');
    fprintf(fid,'@echo off\n(\n');
    
    cmdsInThisBetch = find(cmdBatch == batchI);
    for j=1:length(cmdsInThisBetch)
        fprintf(fid,' start /b %s\n',cmd{cmdsInThisBetch(j)});
    end
    fprintf(fid,') | set /P "="');
    fclose(fid);
    [status,txt] = system(tmpBatFP);
    
    %Known errors to ignore
    txt = strrep(txt,'Exception ignored in: <_io.TextIOWrapper name=''<stdout>'' mode=''w'' encoding=''cp1252''>','');
    txt = strrep(txt,'OSError: [Errno 22] Invalid argument','');
    txt = strtrim(txt);
    
    %Error handling
    if status~=0 && ~isempty(txt)
        fprintf('problem in MW2:\n%s',txt);
    end
    
    %Cleanup
    delete(tmpBatFP);
end
