function yOCTTestReconstruction(TestVectorFolder)
%This tester loads OCT files and check preformances and imagery 
%
%To generate a parameters file
%parameters = {'OCTSystem','Wasatch','dispersionParameterA',100,'BScanAvgFramesToProcess',1:2};
%save('parameters.mat','parameters');

%clear;
close all;

%Determine if we are running on AWS
if (strcmpi(TestVectorFolder(1:3),'s3:'))
    isAWS = true;
else
    isAWS = false;
end

%% Get list of tests
[folders,testNames] = yOCTGetOCTFoldersInPath (TestVectorFolder);

%% Loop for each folder, and test
for i=1:length(folders)
    if isAWS
        testNames{i} = [testNames{i} 'AWS'];
    end
    
    fprintf ('Runnig Test: %s ...\n',testNames{i});
    %Check for parmeters file
    paramFile = [folders{i} 'parameters.mat'];
    
    parameters = {};
    if ~isAWS
        %This part doesn't work as is on AWS
        if exist(paramFile,'file')
            parameters = load(paramFile);
            parameters = parameters.parameters;
        end
    end    
    
    %Run the test
    tic;
    [meanAbs,speckleVariance,dimensions] = yOCTProcessScan({ ...
        folders{i}, ...
        {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
        parameters{:}, ...
        'showStats',true});
    totalRunTime = toc;
    testDate = datenum(datetime);
    
    %Load results from prev run (if they exist), and compare
    testResultFile = [testNames{i} 'TestResult.mat'];
    if exist(testResultFile,'file')
        testResult = load(testResultFile);
        
        %Check bit exactiness
        if (max(abs(speckleVariance(:) - testResult.speckleVariance(:))) < mean(speckleVariance(:))*0.05)
            %Pritty much bit exact
            disp('speckleVariance is Bit Exact');
        end
        if (max(abs(meanAbs(:) - testResult.meanAbs(:))) < mean(meanAbs(:))*0.05)
            %Pritty much bit exact
            BA = 'Bit Exact to Current Version';
            disp('meanAbs is Bit Exact');
        else
            BA = 'Different Then Current Version';
        end
        
        %Plot meanAbs
        subplot(2,1,2);
        imagesc(log(squeeze(testResult.meanAbs(:,:,round(end/2)))));
        xlabel('x direction');
        ylabel('z direction');
        title(sprintf('Prev Result, runtime of %.1f[sec]\n%s',testResult.totalRunTime(end),BA));
        
        totalRunTime = [testResult.totalRunTime totalRunTime]; %Append runtime
        testDate =[testResult.testDate testDate]; %Append test date
    end
            
    %Plot meanAbs
    subplot(2,1,1);
    imagesc(log(squeeze(meanAbs(:,:,round(end/2)))));
    xlabel('x direction');
    ylabel('z direction');
    title(sprintf('%s\nCurent Result, runtime of %.1f[sec]',testNames{i},totalRunTime(end)));
    colormap gray;
    saveas(gcf,['Test_' testNames{i} '.png']);
    
    %Plot runtime
    subplot(1,1,1);
    [~, sizeX, sizeY, AScanAvgN, BScanAvgN] = yOCTLoadInterfFromFile_DataSizing(dimensions);
    plot(testDate-testDate(end),totalRunTime,'o-',0,totalRunTime(end),'or');
    title(sprintf('{%s} Runtime Evolution\nScan Size in Pixels (X-Y-#Avg): %d-%d-%d',...
        testNames{i},sizeX,sizeY,AScanAvgN*BScanAvgN));
    ylabel('Runtime [sec]');
    xlabel('How Many Days Ago Was It Tested?');
    legend('History','Current Test');
    grid on;
    saveas(gcf,['Test_' testNames{i} 'Runtime.png']);
    
    %Save this test result for next excecution
    save(testResultFile,'totalRunTime','testDate','meanAbs','speckleVariance');
    disp('Done');
end



