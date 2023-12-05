function yOCTTurnOpticalSwitch(newState)
% This script controls Agiltron Optical Switch. To select between
% photodiode path and OCT
% https://docs.google.com/document/d/110r7mnKVDM_gtPJmkoLcAxplEn8t0JWKQvdcDOx5qJA/edit
% INPUTS:
%   newState 
%       - set to 'photodiode' to connect photo diode fiber
%       - set to 'OCT' to connect OCT 
%       - set to 'init' to initialize com without turning anything

tic
%% Input Processing
% Nothing to do here

%% Comm
persistent sp
if isempty(sp)
    % This is the first time we turn switch, connect to Port
    sp=serialport('COM3',9600,'Timeout',5); %If that COM doesn't exist use serialportlist to list all options.
end
%delete(sp) % Disconnect from port

%% Set Data
switch(lower(newState))
    case 'photodiode'
        data = [1 18 0 1];
    case 'oct'
        data = [1 18 0 2];
    case 'init'
        return
    otherwise 
        error('Don''t know state %s',newState)
end

%% Execute command
write(sp,data,'uint8'); % Selects photobleach diode

%% Timing analysis
tt_ms=toc()*1e3;
if (tt_ms > 2)
    %warning("yOCTTurnOpticalSwitch('%s') took %.1fms, which is longer than expected",newState,tt_ms);
end
