// ThorlabsImagerOCT.cpp : Defines the exported functions for the DLL application - OCT Part

#include "stdafx.h"
#include "ThorlabsImager.h"
#include <SpectralRadar.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <vector>
//#include "lasercontrol.h"
#include <fstream>

using namespace std;

//DLL Internal state varibles:
static OCTDeviceHandle Dev_;
static ProbeHandle Probe_;

//OCT Probe specific info
static string octProbeName;
static double octScanSpeed; //A scans per sec

//Initialize OCT Scanner
void yOCTScannerInit(
	const char probeFilePath[]  // Probe ini path. Can be usually found at: C:\\Program Files\\Thorlabs\\SpectralRadar\\Config\\ 
)
{

	// Turn verbose off 
	setLog(LogOutputType::None, nullptr);

	Dev_ = initDevice();
	Probe_ = initProbe(Dev_, probeFilePath);

	//Device chack
	if (Dev_ == 0)
	{
		cerr << "Device handle is invalid. " << endl;
		char message[512];
		getError(message, 512);
		cerr << message << endl;
		return;
	}

	//Get Device Name
	const char* DevName = getDevicePropertyString(Dev_, Device_Type);

	//Device specific configuration
	if (strcmp(DevName, "Ganymede") == 0)
	{
		octProbeName = "Ganymede";
		octScanSpeed = 28000; //Hz (samples/sec)
	}
	else if (strcmp(DevName, "Telesto") == 0)
	{
		octProbeName = "Telesto";
		octScanSpeed = 28000; //Hz (samples/sec). This code only use the default scan rate.
	}
	else
		cerr << "Unknown device: " << DevName << endl;
}

//Close OCT Scanner, Cleanup
void yOCTScannerClose()
{
	closeProbe(Probe_);
	closeDevice(Dev_);
}

// Scan a 3D Volume
void yOCTScan3DVolumePrivate(
	const double xCenter,
	const double yCenter,
	const double rangeX,
	const double rangeY,
	const double rotationAngle,
	const int    sizeX,
	const int    sizeY,
	const int    nBScanAvg,
	const char   outputDirectory[],
	const double dispA,
	const bool isSaveProcessed
)
{
	string outputDirectoryStr = outputDirectory;
	string chirpfilelocation = "C:\\Program Files\\Thorlabs\\SpectralRadar\\Config\\Chirp.dat";

	//If folder doesn't exist, make it. If it does exist, do nothnig - error
	std::wstring stemp = std::wstring(outputDirectoryStr.begin(), outputDirectoryStr.end());
	int isFailedToCreate = CreateDirectory(stemp.c_str(), NULL);
	int isAlreadyExist = ERROR_ALREADY_EXISTS == GetLastError();
	if (isFailedToCreate != 1 || isAlreadyExist)
	{
		cerr << "\nFolder exists already or failed to create directory, will not scan " << outputDirectoryStr << endl <<
			"isFailedToCreate: " << isFailedToCreate << " isAlreadyExist: " << isAlreadyExist << endl;
		return;
	}

	// Create Data Handles
	RawDataHandle Raw = createRawData();
	RawDataHandle RawVolume = createRawData();
	DataHandle BScan = createData();
	DataHandle Volume = createData();
	DataHandle Disp = createData();
	DataHandle Chirp = createData();
	ProcessingHandle Proc = createProcessingForDevice(Dev_);

	// Set Bscan Averages
	setProbeParameterInt(Probe_, Probe_Oversampling_SlowAxis, nBScanAvg);

	if (isSaveProcessed)
	{
		//Load Chirp file
		loadCalibration(Proc, Calibration_Chirp, chirpfilelocation.c_str());
		getCalibration(Proc, Calibration_Chirp, Chirp);

		// Disperion Parameters
		computeDispersionByCoeff(dispA, Chirp, Disp);
		setCalibration(Proc, Calibration_Dispersion, Disp);
		setProcessingFlag(Proc, Processing_UseDispersionCompensation, TRUE);
	}

	char message[1024];

	// Setup Scan and Start Scanning	
	ScanPatternHandle Pattern = createVolumePattern(Probe_, rangeX, sizeX, rangeY, sizeY, ScanPattern_ApoEachBScan, ScanPattern_AcqOrderFrameByFrame);
	rotateScanPattern(Pattern, rotationAngle);
	shiftScanPattern(Pattern, xCenter, yCenter);

	double quadraticCoefficient = 231;
	setProcessingFlag(Proc, Processing_UseDispersionCompensation, TRUE);
	setDispersionQuadraticCoeff(Proc, quadraticCoefficient);
	setDispersionCorrectionType(Proc, DispersionCorrectionType::Dispersion_QuadraticCoeff);

	startMeasurement(Dev_, Pattern, Acquisition_AsyncFinite);

	// get current time stamp to save it in the OCT file 
	time_t currentTime;
	time(&currentTime);

	// Creating the file in the correct data format for ThorImageOCT
	OCTFileHandle OCTFile = createOCTFile(FileFormat_OCITY);

	int bscanIndex = 0;

	const int numOfBScans = sizeY * nBScanAvg;
	std::vector<RawDataHandle> rawBScans(numOfBScans);

	// Looping over bscans
	// In this version of ThorlabsImager, the first B-scan to be saved is the one corresponding to the greatest Y, we therefore loop i backwards
	for (int i = sizeY - 1; i >= 0; --i)
	{
		for (int j = 0; j < nBScanAvg; ++j)
		{
			string fileExtention;
			//Get the raw data and process if required
			getRawData(Dev_, Raw);

			string title = string("data\\Spectral") + to_string(bscanIndex) + std::string(".data");

			rawBScans[bscanIndex] = Raw;

			addFileRawData(OCTFile, rawBScans[bscanIndex], title.c_str());

			bscanIndex += 1;
			//appendRawData(RawVolume, Raw, Direction_3);
			/*if (isSaveProcessed)
			{
				//fileExtention = "raw";
				appendRawData(RawVolume, Raw, Direction_3);
			}
			else
				setProcessedDataOutput(Proc, BScan);
				executeProcessing(Proc, Raw);
				appendData(Volume, BScan, Direction_3);
				//fileExtention = "srr";
			*/


			//Generate file name to export data to
			//char filename[1024];
			//sprintf_s(filename, (outputDirectoryStr + "\\Data_Y%04d_YTotal%d_B%04d_BTotal%d_" + octProbeName + "." + fileExtention).c_str(), i + 1, sizeY, j + 1, nBScanAvg);

			//Export data
			//if (isSaveProcessed)
			//	exportData(BScan, DataExport_RAW, filename);
			//else
			//	exportRawData(Raw, RawDataExport_SRR, filename);
			//if (getError(message, 512))
			//{
			//	cerr << "\n\nAn error occurred: " << message << endl;
			//}
		}
	}

	if (!isSaveProcessed)
	{
		//Copy Chirp File
		ifstream source(chirpfilelocation, ios::binary);
		ofstream dest(outputDirectoryStr + "\\Chirp.dat", ios::binary);
		dest << source.rdbuf();
	}

	//Cleanup
	stopMeasurement(Dev_);

	saveCalibrationToFile(OCTFile, Proc);
	/*if (isSaveProcessed)
	{
		// Optional: adding the raw (unprocessed) data to the file.
		addFileRawData(OCTFile, RawVolume, "data\\Spectral0.data");
		saveCalibrationToFile(OCTFile, Proc);
	}
	else
	{
		// Adding the processed data to the file
		// To see what data types are used in standard .oct-files please see variables "DataObjectName_" in "SpectralRadar.h"
		addFileRealData(OCTFile, Volume, DataObjectName_OCTData);
	}*/

	// A suitable mode need to be selected to view the data in ThorImageOCT. 
	// All acquisition modes from ThorImageOCT are listed in "SpectralRadar.h" starting with "AcquisitionMode_".
	// Please note that the available acquisitions modes may depend on your hardware.
	setFileMetadataString(OCTFile, FileMetadata_AcquisitionMode, AcquisitionMode_3D);

	// Specify that we provided both raw and processed data.
	setFileMetadataInt(OCTFile, FileMetadataInt::FileMetadata_ProcessState, FileMetadata_ProcessingState::RawSpectra);

	// The data from the device, probe, processing and scan pattern will be saved as well
	saveFileMetadata(OCTFile, Dev_, Proc, Probe_, Pattern);

	// save timestamp
	setFileMetadataTimestamp(OCTFile, currentTime);

	string octFilePath = outputDirectoryStr + "\\VolumeFile.oct";
	saveFile(OCTFile, octFilePath.c_str());

	setDeviceFlag(Dev_, Device_LaserDiodeStatus, FALSE);
	clearOCTFile(OCTFile);
	clearRawData(Raw);
	//clearRawData(RawVolume);
	clearScanPattern(Pattern);
}

//Public versions
void yOCTScan3DVolumeProcessed(
	const double xCenter, const double yCenter, const double rangeX, const double rangeY, const double rotationAngle,
	const int    sizeX, const int    sizeY, const int    nBScanAvg,
	const char   outputDirectory[], const double dispA
)
{
	yOCTScan3DVolumePrivate(xCenter, yCenter, yCenter, rangeY, rotationAngle, sizeX, sizeY, nBScanAvg, outputDirectory,
		dispA, true);
}

void yOCTScan3DVolume(
	const double xCenter, const double yCenter, const double rangeX, const double rangeY, const double rotationAngle,
	const int    sizeX, const int    sizeY, const int    nBScanAvg,
	const char   outputDirectory[]
)
{
	yOCTScan3DVolumePrivate(xCenter, yCenter, rangeX, rangeY, rotationAngle, sizeX, sizeY, nBScanAvg, outputDirectory,
		0, false);
}

//Turn laser on/off
/*void yOCTTurnLaser(const bool onoff) //set to true to turn laser on
{
	if (onoff)
		controllaser(TRUE);
	else
		controllaser(FALSE);
}*/

// Photobleach a Line
void yOCTPhotobleachLine(
	const double xStart,	//Start position [mm]
	const double yStart,	//Start position [mm]
	const double xEnd,		//End position [mm]
	const double yEnd,		//End position [mm]
	const double duration,	//How many seconds to photobleach
	const double repetition //How many times galvoes should go over the line to photobleach. slower is better. recomendation: 1
)
{
	double dwellPerPass = duration / repetition; //How many seconds to dwell per one pass of the laser beam on the B scan
	int howManyAScansPerBScan = (int)(octScanSpeed * dwellPerPass); //Total number of A Scans asqquired

	//Setup
	ScanPatternHandle Pattern = createBScanPatternManual(Probe_, xStart, yStart, xEnd, yEnd, howManyAScansPerBScan);

	//Photobleach
	startMeasurement(Dev_, Pattern, AcquisitionType::Acquisition_AsyncContinuous);
	Sleep((long)(1000 * duration)); //Sleep in msec
	stopMeasurement(Dev_);

	//Cleanup
	clearScanPattern(Pattern);

}

//Take a picture with camera that is on OCT head
void yOCTCaptureCameraImage(
	const char filePath[] //Where to save
)
{
	ColoredDataHandle Image = createColoredData();
	for (int i = 1; i <= 2; i++) // first image is always blank, so run twice
	{
		//cout << "Acquired image. " << endl;
		getCameraImage(Dev_, Image); // can change image size here to crop image
		exportColoredData(Image, ColoredDataExport_JPG, Direction_1, filePath, ExportOption_None);
	}
	clearColoredData(Image);

}

// takes intensity values 0-100, 0 is off, 100 is max
void yOCTSetCameraRingLightIntensity(
	const int newIntensityPercent // 0 to 100
)
{
	//cout << "Changing Ring Light Intensity to " << IntensityValue << endl;
	setOutputDeviceValueByName(Dev_, "ring light", newIntensityPercent);
}