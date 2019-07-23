//This file contains interface to ThorlabsImagerDll
#pragma once

#ifdef THORLABSIMAGERDLL_EXPORTS
#define THORLABSIMAGERDLL_API __declspec(dllexport)
#else
#define THORLABSIMAGERDLL_API __declspec(dllimport)
#endif

#ifndef MY_EXTERN_C
#ifdef __cplusplus
	#define MY_EXTERN_C extern "C"
#else
	//Not using C++ so no need to define as extern C
	#define MY_EXTERN_C 
#endif
#endif
 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OCT SCANNING CAPABILITIES
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Initialize OCT Scanner
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTScannerInit(
	const char probeFilePath[]  // Probe ini path. Can be usually found at: C:\\Program Files\\Thorlabs\\SpectralRadar\\Config\\ 
);

//Close OCT Scanner, Cleanup
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTScannerClose();

// Scan a 3D Volume
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTScan3DVolume(
	const double xCenter, //Scan center position [mm] 
	const double yCenter, //Scan center position [mm]
	const double rangeX, //fast direction length [mm]
	const double rangeY, //slow direction length [mm]
	const double rotationAngle, //Scan angle [deg]
	const int    sizeX,  //Number of pixels on the fast direction
	const int    sizeY,  //Number of pixels on the slow direction
	const int    nBScanAvg, //Number of B scan averages (set to 1 if non)
	const char   outputDirectory[] //Output folder, make sure it doesn't exists, otherwise will return an error
);

// Scan a Processed 3D Volume 
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTScan3DVolumeProcessed(
	const double xCenter, //Scan center position [mm] 
	const double yCenter, //Scan center position [mm]
	const double rangeX, //fast direction length [mm]
	const double rangeY, //slow direction length [mm]
	const double rotationAngle, //Scan angle [deg]
	const int    sizeX,  //Number of pixels on the fast direction
	const int    sizeY,  //Number of pixels on the slow direction
	const int    nBScanAvg, //Number of B scan averages (set to 1 if non)
	const char   outputDirectory[], //Output folder, make sure it doesn't exists, otherwise will return an error
	const double dispA // Dispersion parameter from ThorImage Software, units unkown
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PHOTOBLEACHING
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Control laser diode on / off
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTTurnLaser(const bool onoff); //set to true to turn laser on

// Photobleach a Line, turn laser diode on before photobleaching
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTPhotobleachLine(
	const double xStart,	//Start position [mm]
	const double yStart,	//Start position [mm]
	const double xEnd,		//End position [mm]
	const double yEnd,		//End position [mm]
	const double duration,	//How many seconds to photobleach
	const double repetition //How many times galvoes should go over the line to photobleach. slower is better. recomendation: 1
);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//STAGE CONTROL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Initialize Stage
MY_EXTERN_C THORLABSIMAGERDLL_API double yOCTStageInit(char axes);

//Get / Set Stage Position [mm]
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTStageSetPosition(char axes, double position);

//Close Stage
MY_EXTERN_C THORLABSIMAGERDLL_API void yOCTStageClose(char axes);
