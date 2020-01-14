% This function tests reslicing of a volume

%% Generate volume to reslice

% Generate OCT Volume
topView = diag(ones(1,20));
topView(:,[1:2 (end+(-1:0))]) = []; % We would like the image to be a-symetric
volume = repmat(topView,[1 1 5]);
volume = single(shiftdim(volume,2)); % Dimensions are (z,x,y)
volumeRand = single(rand(size(volume)));

% Generate dimensions
x = 1:size(volume,2); x = x-mean(x);
y = 1:size(volume,3); y = y-mean(y);
z = 0:size(volume,1)-1; 

% Generate dimensions structure
dimensions.z.order = 1;
dimensions.z.values = z;
dimensions.z.index = 1:length(z);
dimensions.z.units = 'mm';
dimensions.x.order = 2;
dimensions.x.values = x;
dimensions.x.index = 1:length(x);
dimensions.x.units = 'mm';
dimensions.y.order = 3;
dimensions.y.values = y;
dimensions.y.index = 1:length(y);
dimensions.y.units = 'mm';

%% Test extraction of data
slice = yOCTReslice_Slice(volumeRand, dimensions, x(1), y(1), z(1));
assert(abs(slice-volumeRand(1,1,1))<1e-3, ...
    'Extraction Test #1 Failed, single point');

slice = yOCTReslice_Slice(volumeRand, dimensions, x(1:3), y(1)*ones(1,3), z(1)*ones(1,3));
assert(all(abs(slice-volumeRand(1,1:3,1))<1e-3), ...
    'Extraction Test #2 Failed, x vector');

slice = yOCTReslice_Slice(volumeRand, dimensions, x(1:3), y(1:3), z(1:3));
assert(all(all(all(abs(slice - [...
    volumeRand(1,1,1), volumeRand(2,2,2), volumeRand(3,3,3)...
    ])<1e-3))), 'Extraction Test #3 Failed, 3D line');

[xx,zz] = meshgrid(x(1:3),z(1:2));
slice = yOCTReslice_Slice(volumeRand, dimensions, xx, y(1)*ones(size(xx)), zz);
assert(all(all(all(abs(slice - volumeRand(1:2,1:3,1))<1e-3))), ...
    'Extraction Test #4 Failed, y plane');

slice = yOCTReslice_Slice(volumeRand, dimensions, ...
    linspace(x(1),x(2)), linspace(y(1),y(1)), linspace(z(1),z(1)));
assert(abs(mean(slice)-mean(volumeRand(1,1:2,1)))<1e-3, ...
    'Extraction Test #5 Failed, interpolation');

slice = yOCTReslice_Slice(volumeRand, dimensions, 1e4, 1e4, 1e4);
assert(isnan(slice),'Extraction Test #6 Failed, expected nan outside of slide');

slice = yOCTReslice_Slice(volumeRand, dimensions, [0 0], [0 0], [-1 0]);
assert(isnan(slice(1)) & ~isnan(slice(2)),'Extraction Test #7 Failed, partial coverage');

slice = yOCTReslice_Slice(volumeRand, dimensions, [0 0], [1e4 0], [0 0]);
assert(isnan(slice(1)) & ~isnan(slice(2)),'Extraction Test #8 Failed, partial coverage');

%% Test coordinate system conversion
n = [0;1;0];
[~, xyzN2O] = yOCTReslice(volume,n,-1:1,-1:1,0:1,'dimensions',dimensions);
assert(all(xyzN2O(1,0,0)==[1;0;0]),'Test #1 Failed, Rectified Dim');
assert(all(xyzN2O(0,1,0)==[0;1;0]),'Test #2 Failed, Rectified Dim');
assert(all(xyzN2O(0,0,1)==[0;0;1]),'Test #3 Failed, Rectified Dim');

n = [1;0;0];
[~, xyzN2O] = yOCTReslice(volume,n,-1:1,-1:1,0:1,'dimensions',dimensions);
assert(all(xyzN2O(1,0,0)==[0;-1;0]),'Test #4 Failed, Rectified Dim');
assert(all(xyzN2O(0,1,0)==[1; 0;0]),'Test #5 Failed, Rectified Dim');
assert(all(xyzN2O(0,0,1)==[0; 0;1]),'Test #6 Failed, Rectified Dim');

n = [1;1;0]; n = n/norm(n);
[~, xyzN2O] = yOCTReslice(volume,n,-1:1,0:1,0:1,'dimensions',dimensions);
dz = xyzN2O(1,0,0)-xyzN2O(0,0,0);
dx = xyzN2O(0,1,0)-xyzN2O(0,0,0);
dy = xyzN2O(0,0,1)-xyzN2O(0,0,0);

assert(abs(dot(n,dz))<1e-3,'Direction incorrect 1');
assert(norm(dz)==1,'Size incorrect 1');
assert(norm(dx)==1,'Size incorrect 2');
assert(norm(dy)==1,'Size incorrect 3');
assert(norm(xyzN2O(0,0,0))<1e-3,'Center wrong');

%% End to end test
n = [-1;1;0]; n = n/norm(n);
m = min([max(x) max(y)]);
slice = yOCTReslice(volume,n,-m:m,[0 2],z,'dimensions',dimensions);
assert(all(all((slice(:,:,1) > 0.5))),'Midway should have values');
assert(all(all((slice(:,:,2) < 0.01))),'Outside midway shouldn''t have any values');

%% End to end test with input and output folders
yOCT2Tif(volume,'TMP_Reslice\Input\', 'metadata', dimensions);
yOCTReslice('TMP_Reslice\Input\',n,-m:m,[0 2],z,'outputFileOrFolder','TMP_Reslice\Output.tif');
yOCTReslice('TMP_Reslice\Input\',n,-m:m,[0 2],z,'outputFileOrFolder',{'TMP_Reslice\Output\','TMP_Reslice\Output.tif'});

slice =  yOCTFromTif('TMP_Reslice\Output\');
assert(all(all((slice(:,:,1) > 0.5))),'Midway should have values 2');
assert(all(all((slice(:,:,2) < 0.01))),'Outside midway shouldn''t have any values 2');

awsRmDir('TMP_Reslice');
