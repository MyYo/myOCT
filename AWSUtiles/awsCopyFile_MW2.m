function awsCopyFile_MW2(dest)
%See awsCopyFile_MW1 for documentation
%Will go over all folders in dest to get files out!

%% Input check
if awsIsAWSPath(dest)
    awsSetCredentials(1); %We need CLI
end
dest = awsModifyPathForCompetability(dest);

%% Get all the files that need loving
try
    ds = fileDatastore(...
        dest,'ReadFcn',@(x)(x),'IncludeSubfolders',true,...
        'FileExtensions',{'.getmeout'});
catch ME
    fprintf('Processing "%s"...\n',dest);
    rethrow(ME)
end
files = ds.Files;
froms = cell(size(files));
tos   = cell(size(files));
for i=1:length(files)
    from = files{i};
    froms{i} = awsModifyPathForCompetability(from,true); %Modify for CLI compatablity
    
    %To get the real file name, go up a few folders
    f1 = fileparts(from);
    to = fileparts([f1 '.']);
    
    tos{i} = awsModifyPathForCompetability(to,true); %Modify for CLI compatablity
end

%% Preform the move
if (awsIsAWSPath(dest))
    
    if length(froms) > 5 %Many files are been copied
        %Parallel Version
        awsCopyFile_MW2_AWSParallel(froms,tos);
    else % Few files are beying copied
        %Sync version
        for i=1:length(froms)
            awsCmd(sprintf('aws s3 mv "%s" "%s"',froms{i},tos{i}));
        end
    end
else
    for i=1:length(froms)
        tmpfn = tempname;
        movefile(froms{i},tmpfn,'f');
        rmdir(tos{i},'s');
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
if (~iscell(tos) == 1)
    to = awsModifyPathForCompetability([tos '/']);
    tos = cell(size(froms));
    for i=1:length(tos)
        tos{i} = to;
    end
end

%% Make the commands
cmd = cell(size(froms));
for i=1:length(cmd)
    cmd{i} = sprintf('aws s3 mv "%s" "%s"',froms{i},tos{i});
end

%% Devide commands into betches
maxCmdsPerBetch = min(maxNumCompThreads*4,20);
nBetches = ceil(length(cmd)/maxCmdsPerBetch);
cmdBatch = mod((1:length(cmd))-1,nBetches)+1; %This will tell each command in what batch it is

%% Run batches
for batchI = 1:nBetches

    %Create a file with all commands
    tmpBatFP = 'tmp.bat';
    fid = fopen(tmpBatFP,'w');
    fprintf(fid,'@echo off\n(\n');
    
    cmdsInThisBetch = find(cmdBatch == batchI);
    for j=1:length(cmdsInThisBetch)
        c = cmd{cmdsInThisBetch(j)};
        c = strrep(c,'%','%%'); %If % sign appears in the command it needs to be '%%'
        fprintf(fid,' start "" /b cmd /c %s\n',c);
    end
    fprintf(fid,') | set /P "="');
    fclose(fid);
    
    awsCmd(tmpBatFP, {...
        'Exception ignored in: <_io.TextIOWrapper name=''<stdout>'' mode=''w'' encoding=''cp1252''>', ...
        'OSError: [Errno 22] Invalid argument' ...
        });
   
    %Cleanup
    delete(tmpBatFP);
end
