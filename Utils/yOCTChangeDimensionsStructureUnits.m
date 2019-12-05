function dimOut = yOCTChangeDimensionsStructureUnits(dim,newUnits)
% This function converts dimensions structure to specified units
% INPUTS:
%   dim - dimensions structure to be converted
%   newUnits - string specifing what are the new units, can be
%       'mm' or 'millimeters' - for milimeters
%       'um' or 'microns' - for micrometers
%       'm'  or 'meters' - for meters
% OUTPUTS:
%   dimOut - dimensions corrected structure

% How to convert to new units
fToNewUnits = howToConvertFromMetersTo(newUnits);

%% Loop over all fields, correct them individually
dimOut = dim;
if isfield(dim,'x')
    dimOut.x = correctField(dim.x,fToNewUnits,newUnits);
end
if isfield(dim,'y')
    dimOut.y = correctField(dim.y,fToNewUnits,newUnits);
end
if isfield(dim,'z')
    dimOut.z = correctField(dim.z,fToNewUnits,newUnits);
end
if isfield(dim,'lambda')
    dimOut.lambda = correctField(dim.lambda,fToNewUnits,newUnits);
end



function fOut = correctField(f, fToNewUnits, newUnitsName)
fOut = f;

fFromOldUnits = 1/howToConvertFromMetersTo(f.units);
fOut.values = fOut.values*fFromOldUnits*fToNewUnits;

fOut.units = newUnitsName;

function factor = howToConvertFromMetersTo (unitsName)

% Rectify units name
unitsName = lower(unitsName);
unitsName(unitsName == '[') = '';
unitsName(unitsName == ']') = '';
unitsName(unitsName == '(') = '';
unitsName(unitsName == ')') = '';
unitsName = strtrim(unitsName);

%Find what units is this
if (contains(unitsName,'mm') || contains(unitsName,'millimeter'))
    factor = 1e3;
elseif (contains(unitsName,'um') || contains(unitsName,'micron') || contains(unitsName,'micrometer'))
    factor = 1e6;
elseif (contains(unitsName,'nm') || contains(unitsName,'nanometer'))
    factor = 1e9;
elseif (contains(unitsName,'meter') || strcmp(strtrim(unitsName),'m'))
    factor = 1;
elseif strcmp(strtrim(unitsName),'na')
    %Unit nane is NA, conversion factor doesn't exist
    factor = NaN;
else
    error('Don''t know unit type: %s',unitsName);
end