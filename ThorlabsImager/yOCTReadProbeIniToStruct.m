function st = yOCTReadProbeIniToStruct(probeIniPath)
% This function reads probe ini and returns a struct with all the
% information in it to be used later

probeIniPath = 'Y:\Work\_de la Zerda Lab Scripts\HashtagAlignmentRepo\01 OCT Scan and Pattern\Thorlabs\Probe - Olympus 10x.ini';

%% Get the text in the probe ini
txt = fileread(probeIniPath);
txtlines = strsplit(txt,'\n');
txtlines = cellfun(@strtrim,txtlines,'UniformOutput',false);
txtlines(cellfun(@isempty,txtlines)) = [];

%% Loop over every line, evalue the input
st = struct();
for i=1:length(txtlines)
    ln = txtlines{i};
    
    %Remove comments and make line pretty
    if (contains(ln,'#'))
        %There is a comment in this line, remove it
        ln = ln(1:(find(ln=='#',1,'first')-1));
    end
    ln = strtrim(ln);
    if (isempty(ln))
        continue; %Nothing to do here
    end
    
    evalc(['st.' ln]);
end
