function yOCTTurnOpticalSwitch(newState, com)
% This script controls Agiltron Optical Switch. To select between
% photodiode path and OCT
% https://docs.google.com/document/d/110r7mnKVDM_gtPJmkoLcAxplEn8t0JWKQvdcDOx5qJA/edit
% INPUTS:
%   newState - set to 'photodiode' to connect photo diode fiber
%            - set to 'OCT' to connect OCT 
%   com - which com port is the switch on. If com is empty, will skip this
%   function. Default is 'COM3'. To see a list of all ports: serialportlist


%% Input processing
if ~exist('com','var')
    com='COM3';
end

if isempty(com)
    return;
end

switch(lower(newState))
    case 'photodiode'
        data = [1 18 0 1];
    case 'oct'
        data = [1 18 0 2];
    otherwise 
        error('Don''t know state %s',newState)
end

%% Execute command
s=serialport('COM3',9600,'Timeout',5);
write(s,data,'uint8'); % Selects photobleach diode
delete(s)
