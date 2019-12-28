function yOCTTestReconstruction(baseFolder, isLookForTestVectorsInBaseFolder)
% This tester loads OCT files and check preformances and imagery.
% Will try to reconstruct OCT in basefolder according to information in
% parameters.json.
% INPUTS:
%   baseFolder - contains OCT data and may contain parameters.json
%       parameters.json can contain:
%           .reconstructionFunction - can be 'yOCTProcessScan' or
%              'yOCTProcessTiledScan'
%           .reconstructionParameters - cell array of parameters to send to
%              reconstructionFunction
%   isLookForTestVectorsInBaseFolder - when set to true (default is false)
%       will look for OCT folders inside the baseFoldera and run function
%       on all of them

%% Setup & Input checks
% To generate a parameters file
%parameters = {'OCTSystem','Wasatch','dispersionParameterA',100,'BScanAvgFramesToProcess',1:2};

%clear;
close all;

if ~exist('isLookForTestVectorsInBaseFolder','var')
    isLookForTestVectorsInBaseFolder = false;
end

%% List files
if isLookForTestVectorsInBaseFolder
    [folders,testNames] = yOCTGetOCTFoldersInPath(baseFolder);
else
    folders = awsModifyPathForCompetability([baseFolder '/']);
    [~,testNames] = fileparts([folders '.a']);
    folders = {folders};
    testNames = {testNames};
end

%% Loop for each folder, and test
for i=1:length(folders)
    %Determine if we are running on AWS
    isAWS = awsIsAWSPath(folders{i});
    
    if isAWS
        testNames{i} = [testNames{i} 'AWS'];
    end
    
    fprintf ('Runnig Test: %s ...\n',testNames{i});
    fprintf ('Test Folder: %s\n',folders{i});
    
    % Check if .mat parameters file exist, if it does convert to json
    paramFile = [folders{i} 'parameters.mat'];
    if ~isAWS && exist(paramFile,'file')
        parameters = load(paramFile);
        json.reconstructionFunction = 'yOCTProcessScan';
        json.reconstructionParameters = parameters.parameters;
        json.version = 1;
        
        awsWriteJSON(json,[folders{i} 'parameters.json']);
        awsRmFile(paramFile);
    end
    
    % Load parmeters file, if exist
    paramFile = [folders{i} 'parameters.json'];
    if awsExist(paramFile,'file')
        json = awsReadJSON(paramFile);
    else
        json.reconstructionFunction = 'yOCTProcessScan';
    end
    if (isempty(json.reconstructionParameters))
        json.reconstructionParameters = {};
    end
    
    %Run the test
    tic;
    switch(json.reconstructionFunction)
        case 'yOCTProcessScan'
            [meanAbs,speckleVariance,dimensions] = yOCTProcessScan({ ...
                folders{i}, ...
                {'meanAbs','speckleVariance'}, ... Which functions would you like to process. Option exist for function hendel
                json.reconstructionParameters{:}, ...
                'showStats',true});
    end
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



