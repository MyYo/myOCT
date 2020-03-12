function awsSaveMatlabFigure(figureHandle, pathToSave, isMaximizeBeforeSave, isCloseFigureAfterSaving)
% This function saves a matlab figure handle as png to the cloud / local
% drive.
% INPUTS:
%   - figureHandle - use gcf for current matlab figure handle.
%   - pathToSave - s3 or local path to save figure to.
%   - isMaximizeBeforeSave - Default: true. maximize figure to full screen
%       before saving - to make it visually bigger.
%   - isCloseFigureAfterSaving - Default: true. Closes matlab figure after
%       done.

%% Input checks
if isempty(figureHandle)
    figureHandle = gcf;
end

if ~exist('isMaximizeBeforeSave','var') || isempty(isMaximizeBeforeSave)
    isMaximizeBeforeSave = true;
end
if ~exist('isCloseFigureAfterSaving','var') || isempty(isCloseFigureAfterSaving)
    isCloseFigureAfterSaving = true;
end

%% Maximize image if requred
if (isMaximizeBeforeSave)
    set(figureHandle,'units','normalized','outerposition',[0 0 1 1]);
    refresh(figureHandle);
    pause(0.1);
end

%% Make sure we can save to cloud before going further
if (awsIsAWSPath(pathToSave))
    awsSetCredentials(1); %Write cridentials are required  
end

%% Generate figure into temporary file
fp = [tempname '.png'];
saveas(figureHandle,fp);

%% Upload & Cleanup
awsCopyFileFolder(fp,pathToSave);
delete(fp);

if (isCloseFigureAfterSaving)
    close(figureHandle);
end
