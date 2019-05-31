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
    %To do so, you need to copy (once) the files: matlab.prf, parallel.settings, connector.settings  -- after logging in to Matlab Cluster from my user --
    %Files are located at:
    %   C:\Users\<My User>\AppData\Roaming\MathWorks\MATLAB\<Which Matlab>\
    %Copy files to:
    %   C:\Windows\System32\config\systemprofile\AppData\Roaming\MathWorks\MATLAB\<Which Matlab>\
    
    if isClusterConnect %Do we need to connect to cluster
        myCluster = parcluster('delaZerdaParallel');
        switch myCluster.State %All Cluster's scenarios
            case 'offline'
                start(myCluster);
                wait(myCluster); %Wait for the cluster to be ready to accept job submissions
                myPool=parpool(myCluster); %Create parallel pool on cluster
            case 'starting'
                wait(myCluster); %Wait for the cluster to be ready to accept job submissions
                myPool=parpool(myCluster); %Create parallel pool on cluster
            case 'stopping'
                wait(myCluster); %Wait for the cluster to stop
                start(myCluster);
                wait(myCluster);
                myPool=parpool(myCluster); %Create parallel pool on cluster
            case 'online'
                mypool=gcp;
                if (mypool.Connected ~= 1)
                    myPool=parpool(myCluster);
                end
        end
    end
	
	%% Setup environment
	currentFileFolder = fileparts(mfilename('fullpath'));
	yOCTMainFolder = [currentFileFolder '\..\'];
	addpath(genpath(yOCTMainFolder)); %Add current files to path
	opengl('save', 'software'); %Increase stubility in OPEN GL
    
    %Process all OCT scans in the folder
	myOCTBatchProcess(OCTFolderPath,executionConfiguration); 
    
    %If Cluster is on shut it down
     if isClusterConnect 
        switch myCluster.State %All the scenarios the cluster can be in
            case 'starting'
                wait(myCluster);
                shutdown(myCluster);
            case 'stopping'
                wait(myCluster); %Wait for the cluster to stop
            case 'online'
                shutdown(myCluster);
                wait(myCluster);
            case 'offline' 
                %do nothing
        end
    end
    
catch ME 
    %Error Hendle
    if isClusterConnect
        if ((strcmp(myCluster.State,'offline')==0) && strcmp(myCluster.State,'stopping')==0) %If Cluster is on/starting
            delete(myPool);
            wait(myCluster);
            shutdown(myCluster);
            wait(myCluster);
        end
    end
       
	disp(' '); 
	disp('Error Happened'); 
	for i=1:length(ME.stack) 
		ME.stack(i) 
	end 
	disp(ME.message); 
	exit(1); 
end 
exit(0); 
