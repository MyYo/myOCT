function awsSetRegion(newRegion, level)
%This function sets the active region used by AWS
%For the meaning of levels see awsSetCredentials

if ~exist('level','var')
    level = 0;
end

%Set environment varible
setenv('AWS_REGION', newRegion);

if (level > 0)
    %Set for CLI
    
    %Create a temporary file with the instructions we need
    fid = fopen('tmp.txt','w');
    fprintf(fid,'\n\n%s\n\n',newRegion);
    fclose(fid);
    
    %Run
    [err,txt] = system('aws configure < tmp.txt');
    
    %Cleanup
    delete('tmp.txt');
    
    %Error check
    if (err ~= 0)
        error('Error happend while setting aws region: %s', txt);
    end
end
    
