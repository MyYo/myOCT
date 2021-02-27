function tf = awsIsAWSPath(filePath)
% This function returns true if filePath contains an AWS stream
% filePath can be a string (for a single file), or cell array for multiple
% files

%% Input Checks
if ~iscell(filePath)
    filePath = {filePath};
end

%% Compute
tf = zeros(size(filePath));

for i=1:length(tf)
    if (strncmpi(filePath{i},'s3:',3))
        tf(i) = true;
    else
        tf(i) = false;
    end
end