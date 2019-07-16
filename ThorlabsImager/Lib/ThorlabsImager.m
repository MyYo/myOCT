function [methodinfo,structs,enuminfo,ThunkLibName]=ThorlabsImager
%THORLABSIMAGER Create structures to define interfaces found in 'ThorlabsImager'.

%This function was generated by loadlibrary.m parser version  on Mon Jul 15 22:55:00 2019
%perl options:'ThorlabsImager.i -outfile=ThorlabsImager.m -thunkfile=ThorlabsImager_thunk_pcwin64.c -header=ThorlabsImager.h'
ival={cell(1,0)}; % change 0 to the actual number of functions to preallocate the data.
structs=[];enuminfo=[];fcnNum=1;
fcns=struct('name',ival,'calltype',ival,'LHS',ival,'RHS',ival,'alias',ival,'thunkname', ival);
MfilePath=fileparts(mfilename('fullpath'));
ThunkLibName=fullfile(MfilePath,'ThorlabsImager_thunk_pcwin64');
%  void yOCTScannerInit ( const char probeFilePath [] ); 
fcns.thunkname{fcnNum}='voidcstringThunk';fcns.name{fcnNum}='yOCTScannerInit'; fcns.calltype{fcnNum}='Thunk'; fcns.LHS{fcnNum}=[]; fcns.RHS{fcnNum}={'cstring'};fcnNum=fcnNum+1;
%  void yOCTScannerClose (); 
fcns.thunkname{fcnNum}='voidThunk';fcns.name{fcnNum}='yOCTScannerClose'; fcns.calltype{fcnNum}='Thunk'; fcns.LHS{fcnNum}=[]; fcns.RHS{fcnNum}=[];fcnNum=fcnNum+1;
%  void yOCTScan3DVolume ( const double xStart , const double yStart , const double rangeX , const double rangeY , const double rotationAngle , const int sizeX , const int sizeY , const int nBScanAvg , const char outputDirectory [] ); 
fcns.thunkname{fcnNum}='voiddoubledoubledoubledoubledoubleint32int32int32cstringThunk';fcns.name{fcnNum}='yOCTScan3DVolume'; fcns.calltype{fcnNum}='Thunk'; fcns.LHS{fcnNum}=[]; fcns.RHS{fcnNum}={'double', 'double', 'double', 'double', 'double', 'int32', 'int32', 'int32', 'cstring'};fcnNum=fcnNum+1;
%  void yOCTPhotobleachLine ( const double xStart , const double yStart , const double xEnd , const double yEnd , const double duration , const double repetition ); 
fcns.thunkname{fcnNum}='voiddoubledoubledoubledoubledoubledoubleThunk';fcns.name{fcnNum}='yOCTPhotobleachLine'; fcns.calltype{fcnNum}='Thunk'; fcns.LHS{fcnNum}=[]; fcns.RHS{fcnNum}={'double', 'double', 'double', 'double', 'double', 'double'};fcnNum=fcnNum+1;
%  void yOCTStageInit (); 
fcns.thunkname{fcnNum}='voidThunk';fcns.name{fcnNum}='yOCTStageInit'; fcns.calltype{fcnNum}='Thunk'; fcns.LHS{fcnNum}=[]; fcns.RHS{fcnNum}=[];fcnNum=fcnNum+1;
%  void yOCTStageSetZPosition ( const double newZ ); 
fcns.thunkname{fcnNum}='voiddoubleThunk';fcns.name{fcnNum}='yOCTStageSetZPosition'; fcns.calltype{fcnNum}='Thunk'; fcns.LHS{fcnNum}=[]; fcns.RHS{fcnNum}={'double'};fcnNum=fcnNum+1;
methodinfo=fcns;