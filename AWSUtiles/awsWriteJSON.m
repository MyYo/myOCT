function awsWriteJSON(json,fp)
%This function write a JSON file from AWS or locally
%json - configuration file
%fp - file path, can be local or in AWS

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

%Encode and save
txt = jsonencode(json);
txt = strrep(txt,'"',[newline '"']);
txt = strrep(txt,[newline '":'],'":');
txt = strrep(txt,[':' newline '"'],':"');
txt = strrep(txt,[newline '",'],'",');
txt = strrep(txt,[newline '"}'],['"' newline '}']);
txt = strrep(txt,[newline '"]'],['"' newline ']']);
fid = fopen(fpToSave,'w');
fprintf(fid,'%s',txt);
fclose(fid);

if (isAWS)
    %Upload if required
    awsCopyFileFolder(fpToSave,fp); 
    delete(fpToSave);
end