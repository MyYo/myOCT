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
outCode = 0; %Output code

try 
	fprintf('Starting up environment... ');
    if ~exist('isConnectToCluster','var')
        isConnectToCluster = false;
    end
    
    %% Cleanup Path & Add some files we will need
    
    %Cleanup path. Allow only Matlab original toolbox to prevent multiple
    %paths from different Jenkins & users colliding
    p=path;
    ps = split(p,';');
    %Find which parts of the path are in the original folder of Matlab
    isOrig = cellfun(@(x)(contains(x,matlabroot)),ps);
    psNew = ps(isOrig);%Remove paths which are not matlab toolbox
    psNew = cellfun(@(x)([x ';']),psNew,'UniformOutput',false);%Add ';' at the end of every path
    pNew =[psNew{:}];
    path(pNew); %Set New Path
    
    %Add path
    currentFileFolder = fileparts(mfilename('fullpath'));
	yOCTMainFolder = [currentFileFolder '\..\'];
    d = dir(yOCTMainFolder);yOCTMainFolder = d(1).folder;
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
            case 'starting'
                wait(myCluster); %Wait for the cluster to be ready to accept job submissions
            case 'stopping'
				wait(myCluster,'offline');%Wait for the cluster to stop
                start(myCluster);
                wait(myCluster);
            case 'online'
				%Do nothing
			otherwise
				error('Unknown clustter state: %s, what should I do about it?', myCluster.State);
        end
		fprintf('Cluster.JobStorageLocation = "%s". You can find error logs in case of loss communication with worker here\n',...
			myCluster.JobStorageLocation);
		
		%Set default cluster, but don't start it yet
		parallel.defaultClusterProfile('delaZerdaParallel');
	else
		%Run locally if no cluster
		parallel.defaultClusterProfile('local'); 
    end
	%ps = parallel.Settings;
	%ps.Pool.AutoCreate = false; %Cancel autocreate of parallel pool
	disp(['Adjusted default cluster to: ' parallel.defaultClusterProfile ', cluster is up an running']);
	
	
	%% Setup environment
	opengl('save', 'software'); %Increase stubility in OPEN GL
	
	fprintf('Environment up and running, runing functionHendle...!\n'); %Indicate environment is up and running
    
    %% Run
    
    %Are we running a script?
    if ischar(functionHendle)
        %Convert script name to a function handel
        functionHendle = strrep(functionHendle,'.m',''); %Remove .m
        functionHendle=str2func(functionHendle);
    end
    
    functionHendle();
    
catch ME 
    %% Error Hendle
	disp(' '); 
	disp('Error Happened'); 
	disp(ME.message); 
	for i=1:length(ME.stack) 
		ME.stack(i) 
	end 
	
	outCode = 1;
end 

try
%% If Cluster is on shut it down
fprintf('Winding down ...');

%If parallel pool exists, close it
myPool = gcp('nocreate');
if ~isempty(myPool)
	%My pool is running, shut it down
	delete(myPool);
end

if isConnectToCluster && false %Leave cluster open, it will shut itself down after a while
	switch myCluster.State %All the scenarios the cluster can be in
		case 'starting'
			wait(myCluster); %Wait for cluster to turn on
			shutdown(myCluster);
		case 'stopping'
			%Do nothing, let the cluster close
		case 'online'
			shutdown(myCluster);
		case 'offline' 
			%do nothing
	end
end
fprintf('Done! Goodbye\n');

catch ME 
    %% Error Hendle
	disp(' '); 
	disp('Error Happened while winding down'); 
	disp(ME.message); 
	for i=1:length(ME.stack) 
		ME.stack(i) 
	end 
	
	outCode = 1;
end

exit(outCode); 
