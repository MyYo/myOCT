function tf = isRunningOnJenkins ()
%This function identifies if we are running on Jenkins environment
global isRunningOnJenkins; %Golbal varible stating execution status

if isempty(isRunningOnJenkins) || isRunningOnJenkins == false
    tf = false;
else
    tf = true;
end