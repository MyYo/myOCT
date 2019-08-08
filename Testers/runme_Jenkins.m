function runme_Jenkins(functionHendle,isConnectToCluster)
%This function does all the environment settings to run a function or a
%script (add to path, cluster, error handeling etc) when executed by Jenkins. 
%INPUTS:
%   - functionHendle - Can be
%       1) function to run. No inputs: functionHendle=@()(myfun()); or
%       2) Name of a script to run like 'runme' (no .m required). If running a script, it will run as an isolated environment
%   - isConnectToCluster - connect to Matlab / AWS cluster prior to
%       execution. Default: false.
%EXAMPLES:
%   - Running myOCTBatchProcess:
%       echo runme_Jenkins(@()(myOCTBatchProcess('%OCT_FOLDER_PATH%',{%EXECUTION_CONFIGURATION%})),%IS_CLUSTER_RUN%);> runme.m
%   - Running yOCTTestMyOCTLibrary
%       echo runme_Jenkins('yOCTTestMyOCTLibrary',false);> runme.m

global isRunningOnJenkins; %Golbal varible stating execution status
isRunningOnJenkins = true; 

try 
	fprintf('Starting up environment... ');
    if ~exist('isConnectToCluster','var')
        isConnectToCluster = false;
    end
    
    %% Cleanup Path & Add some files we will need
    
    %Cleanup
    p=path;
    ps = split(p,';');
    %Find which parts of the path are in the original folder of Matlab
    isOrig = cellfun(@(x)(contains(x,matlabroot)),ps);
    %Remove paths which are not matlab toolbox
    for i=find(~isOrig)
        rmpath(ps{i});
    end
    
    %Add path
    currentFileFolder = fileparts(mfilename('fullpath'));
	yOCTMainFolder = [currentFileFolder '\..\'];
	addpath(genpath(yOCTMainFolder)); %Add current files to path
    
    %% Connect to cluster if needed. Matlab Parallel Server
    %To Start a cluster, we need to have a Matlab user logged in to Matlab.
    %However, Jenkins runs as a "SYSTEM" user, which we are unable to open
    %a Matlab instance. Therefore we 'hack' and copy the matlab user files
    %from one of our users to System folder.
    %To do so, you need to copy (once) the files: matlab.prf, parallel.settings, connector.settings  -- after logging in to Matlab Cluster from my user --
    %Files are located at:
    %   C:\Users\<My User>\AppData\Roaming\MathWorks\MATLAB\<Which Matlab>\
    %Copy files to:
    %   C:\Windows\System32\config\systemprofile\AppData\Roaming\MathWorks\MATLAB\<Which Matlab>\
    
    if isConnectToCluster %Do we need to connect to cluster
		disp('Starting Cluster ... ');
        myCluster = parcluster('delaZerdaParallel')
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
			otherwise
				error('Unknown clustter state: %s, what should I do about it?', myCluster.State);
        end
    end
	
	%% Setup environment
	opengl('save', 'software'); %Increase stubility in OPEN GL
	
	fprintf('Done!\n'); %Indicate environment is up and running
    
    %% Run
    
    %Are we running a script?
    if ischar(functionHendle)
        %Convert script name to a function handel
        functionHendle = strrep(functionHendle,'.m',''); %Remove .m
        functionHendle=str2func(functionHendle);
    end
    
    functionHendle();
    
	fprintf('Winding down ...');
    %% If Cluster is on shut it down
    if isConnectToCluster 
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
	fprintf('Done! Goodbye\n');
    
catch ME 
    %% Error Hendle
    if isConnectToCluster
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
