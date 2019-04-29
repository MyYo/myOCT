function runme_TestMyOCTLibrary ()
%Jenkins will run this to test the library

try 
	yOCTTestAll; 
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
