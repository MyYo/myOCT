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
for i=1:length(files)
    from = files{i};
    
    %To get the real file name, go up a few folders
    f1 = fileparts(from);
    to = fileparts([f1 '.']);
    
    if (awsIsAWSPath(dest))
        cmd = sprintf('aws s3 mv "%s" "%s"',from,to);
        [status,txt] = system(cmd);
        if status~=0
            error('Could not move files, message: %s',txt);
        end
    else
        tmpfn = tempname;
        movefile(from,tmpfn,'f');
        rmdir(to,'s');
        movefile(tmpfn,to,'f');
    end
end