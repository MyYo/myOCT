function j = awsReadJSON(fp)
%This function reads a JSON file from AWS or locally

if (awsIsAWSPath(fp))
    %Load Data from AWS
    isAWS = true;
    awsSetCredentials;
    fp = awsModifyPathForCompetability(fp,false);
else
    isAWS = false;
end

%Load JSON to get scan configuration and different depths
ds = fileDatastore(fp,'ReadFcn',@readJSON);
j = ds.read();

function o = readJSON(filename)
fid = fopen(filename);
txt=fscanf(fid,'%s');
fclose(fid);
o = jsondecode(txt);