function awsWriteJSON(json,fp)
%This function write a JSON file from AWS or locally
%json - configuration file
%fp - file path, can be local or in AWS

%% Checks
if (awsIsAWSPath(fp))
    %Load Data from AWS
    isAWS = true;
    fpToSave = 'tmp.json';
else
    isAWS = false;
    fpToSave = awsModifyPathForCompetability(fp);
end

%If folder to write JSON doesn't exist, make it
fldr = fileparts(fpToSave);
if ~isempty(fldr) && ~exist(fldr,'dir')
    mkdir(fldr);
end
%% Replace function handles with appropriate annotations
fn = fieldnames(json);
for i=1:length(fn)
    if (isa(json.(fn{i}),'function_handle'))
        json.(fn{i}) = 'function_handle';
    end
end 

%% Encode and save
txt = jsonencode(json);
txt = strrep(txt,'"',[newline '"']);
txt = strrep(txt,[newline '":'],'":');
txt = strrep(txt,[':' newline '"'],':"');
txt = strrep(txt,[newline '",'],'",');
txt = strrep(txt,[newline '"}'],['"' newline '}']);
txt = strrep(txt,[newline '"]'],['"' newline ']']);
fid = fopen(fpToSave,'w');
if (fid == -1)
    error('Couldn''t open file %s for write mode',fpToSave);
end
fprintf(fid,'%s',txt);
fclose(fid);

if (isAWS)
    %Upload if required
    awsCopyFileFolder(fpToSave,fp); 
    delete(fpToSave);
end