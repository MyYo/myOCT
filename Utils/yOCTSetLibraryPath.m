function yOCTSetLibraryPath(mode)
% Sets up library's path correctly.
% mode can be 
% 'simple' (default) where path head is just the folder above this file; or
% 'full' where path head is one folder above that and we include myOCT as
% well as hashtag alignment

%% Add to path
% Get this file's path
p = mfilename('fullpath');
pathparts = strsplit(p,filesep);
pathparts(end) = [];

% Generate myOCT/ folder path
myOCTFolder = assemblepath(pathparts(1:(end-1)));

% Generate hashtag alginment path
hashtagAlignmentFolder = assemblepath([pathparts(1:(end-2)) {'HashtagAlignmentRepo'}]);

% Add to path as needed by user input
addpath(genpath(myOCTFolder));
if exist('mode','var') && strcmp(mode,'full')
    addpath(genpath(hashtagAlignmentFolder));
end

function s = assemblepath(pathparts)
s = pathparts{1};
for i=2:length(pathparts)
    s = [s filesep pathparts{i}];
end
s = [s filesep];
