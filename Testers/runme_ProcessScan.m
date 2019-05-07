function runme_ProcessScan(OCTFolderPath,executionConfiguration,isClusterConnect)
%This function process scans in a folder, is executed by Jenkins. 
%Its roal is to handle errors, exit
%after execution and handeling execution of parallel cluster.
%Jenkins will execute this command:
%   Jenkins executable: runme_ProcessScan('%OCT_FOLDER_PATH%',{%EXECUTION_CONFIGURATION%})
%Architecture:
%   runme_ProcessScan handels Jenkins and calls myOCTBatchProcess
%   myOCTBatchProcess loops for every OCT scan in the folder and runs yOCTProcessScan
%   yOCTProcessScan loads and process each slice of the OCT scan in a parallel way

try 
    %Connect to cluster if needed. Matlab Parallel Server
    %To Start a cluster, we need to have a Matlab user logged in to Matlab.
    %However, Jenkins runs as a "SYSTEM" user, which we are unable to open
    %a Matlab instance. Therefore we 'hack' and copy the matlab user files
    %from one of our users to System folder.
    %To do so, you need to copy (once) the files: matlab.prf, parallel.settings
    %Files are located at:
    %   C:\Users\<My User>\AppData\Roaming\MathWorks\MATLAB\<Which Matlab>\
    %Copy files to:
    %   C:\Windows\System32\config\systemprofile\AppData\Roaming\MathWorks\MATLAB\<Which Matlab>\
    if isClusterConnect %Do we need to connect to cluster
        myCluster = parcluster('delazerdatrial1');
        start(myCluster);
        wait(myCluster); % Wait for the cluster to be ready to accept job submissions
        myPool=parpool(myCluster); %Create parallel pool on cluster
    else
        %Start a local pool
        myPool=parpool('local'); 
    end
    
    %Process all OCT scans in the folder
	myOCTBatchProcess(OCTFolderPath,executionConfiguration); 
    
    %If Cluster is on, shut it down
    delete(myPool)
    if isClusterConnect
        shutdown(myCluster) 
        wait(myCluster)
    end
catch ME 
    
    %Error Hendle
	disp(' '); 
	disp('Error Happened'); 
	for i=1:length(ME.stack) 
		ME.stack(i) 
	end 
	disp(ME.message); 
	exit(1); 
end 
exit(0); 
