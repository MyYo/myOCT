function yOCTTurnOpticalSwitch(newState, com, delayStart_sec)
% This script controls Agiltron Optical Switch. To select between
% photodiode path and OCT
% https://docs.google.com/document/d/110r7mnKVDM_gtPJmkoLcAxplEn8t0JWKQvdcDOx5qJA/edit
% INPUTS:
%   newState 
%       - set to 'photodiode' to connect photo diode fiber
%       - set to 'OCT' to connect OCT 
%   com - which com port is the switch on. 
%       Default is 'COM3'. To see a list of all ports: serialportlist.
%   delayStart_sec - Utilize this option to delay the turn on operation in
%       order to sync with the OCT scan head. If value is 0 (default), bypass.

%% Input processing
if ~exist('com','var') || isempty(com)
    com = 'COM3';
end

if ~exist('delayStart_sec','var') || isempty(delayStart_sec)
    delayStart_sec = 0;
end

%% Set Data
switch(lower(newState))
    case 'photodiode'
        data = [1 18 0 1];
    case 'oct'
        data = [1 18 0 2];
    otherwise 
        error('Don''t know state %s',newState)
end

%% Timer management
persistent tmr;
if isempty(tmr)
    tmr = timer('ExecutionMode', 'singleShot');
end

%% Execute command
execute_command = @(tmp_1, tmp_2)(execute_command1(data));

if delayStart_sec == 0
    execute_command()
else   
    % Start with async delay
    set(tmr,'StartDelay',delayStart_sec)
    set(tmr,'TimerFcn',execute_command)
    start(tmr);
end

function execute_command1 (data)
s=serialport('COM3',9600,'Timeout',5);
write(s,data,'uint8'); % Selects photobleach diode
delete(s)
