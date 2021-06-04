function yOCTTurnLaser(newState)
% Instructions for this code follow thorlab's example below.
% For more information visit https://www.thorlabs.com/software_pages/ViewSoftwarePage.cfm?Code=4000_Series&viewtab=3
% This function will turn laser diode on or off
% INPUTS:
%   newState - set to true to turn on, false to turn off


%% Input processing

if newState
    newStateText = 'ON';
else
    newStateText = 'OFF';
end

%% Connect to the diode driver

% Device name, can be accessed (TBD) & How to connect to it
diodeDriverID = 'M00511660'; % This is also refered to as S/N when you boot up the driver
cldAddress = ['USB0::0x1313::0x804F::' diodeDriverID '::0::INSTR'];

global cld
cld=visa('NI',cldAddress);
try
    fopen(cld);
catch e
    fprintf('Error connecting to %s, make sure its connected. You can also try running fclose all to release device.\n',diodeDriverID);
    rethrow(e);
end
set (cld, 'Timeout', 10);
set (cld, 'EOSMode', 'read');

%% Set power (this number is configured for OCT histology)
if newState
    % Only before turning diode on
    fprintf(cld, 'SOURCE 0.14'); % LD curent setpoint in Amps
end

%% Change diode state

% TEC On/Off
fprintf(cld, ['OUTPUT2:STATE ' newStateText]);
if str2double(query(cld, 'OUTPUT2:STATE?')) ~= newState
    % Clean up before throwing an error
    fclose(cld);
    delete(cld);

    error('Couldn''t change TEC state to %s',newStateText);
end

% Diode On/Off
fprintf(cld, ['OUTPUT1:STATE ' newStateText]);
if str2double(query(cld, 'OUTPUT1:STATE?')) ~= newState
    warning('Couldn''t change diode state to %s, is the keylock engaged?',newStateText);
end

%% Clean up
fclose(cld);
delete(cld);
pause(0.2);

end


function samplecode
%% SAMPLE Code
%   Integrating a CLD10xx Laser Diode Controller in MATLAB using SCPI commands
%   Specify resource name and vendor of driver
%
%Resource name is: USB0::0x1313::<type id>::<serial number>::0::INSTR
%Please change the type ID and the serial number of your device
%  
%   Type ID: 0x8047when firmware update is enabled
%   0x804Fwhen firmware update is disabled%   (see Menu > System Settings)
cld_addr = 'USB0::0x1313::0x804F::M00278794::0::INSTR';
cld_vendor = 'NI';

%   Open VISA connection and set parametersglobalcld;
cld = visa (cld_vendor, cld_addr);
fopen (cld);
set (cld, 'Timeout', 10);
set (cld, 'EOSMode', 'read');

%   Set TEC temperature and enable 
TECtemp = 25;
fprintf (cld, ['SOURCE2:TEMPERATURE:SPOINT ', num2str(temp)]);
fprintf (cld, ':OUTPUT2:STATE ON');

%   Check if temperature is stabilized
value = str2double (query (cld, 'SENSE2:TEMPERATURE:DATA?'));
disp('Waiting until temperature is stabilized')
while(abs(temp-value)>0.5)
	value = str2double (query (cld, 'SENSE2:TEMPERATURE:DATA?'));
	disp(['Current TEC temperature: ', num2str(value)]);
	pause(1);
end;

%   Set LD current limit in A
fprintf(cld, 'SOURCE:LIMIT 0.060');
%   Set LD current setpoint in A
fprintf(cld, 'SOURCE:LEVEL 0.050');

%   Switch on LD current
fprintf(cld, 'OUTPUT1:STATE ON');
pause(5);

%   Switch off LD current
fprintf(cld, 'OUTPUT1:STATE OFF');

%   Close VISA connection
disp ('Close VISA connection.');
fclose (cld);
delete (cld);
disp ('Connection closed successfully.');

end