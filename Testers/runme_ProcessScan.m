function runme_ProcessScan(OCTFolderPath,executionConfiguration)
%Jenkins will run this to process scans:
%Jenkins executable: runme_ProcessScan('%OCT_FOLDER_PATH%',{%EXECUTION_CONFIGURATION%})

try 
	myOCTBatchProcess(OCTFolderPath,executionConfiguration); 
catch ME 
	disp(' '); 
	disp('Error Happened'); 
	for i=1:length(ME.stack) 
		ME.stack(i) 
	end 
	disp(ME.message); 
	exit(1); 
end 
exit(0); 
