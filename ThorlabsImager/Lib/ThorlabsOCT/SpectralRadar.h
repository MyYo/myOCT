#ifndef _SPECTRALRADAR_H
#define _SPECTRALRADAR_H

#include <stddef.h>

/*! \mainpage Spectral Radar SDK

\section intro Introduction

This document gives an introduction into using the ANSI C Spectral Radar SDK and demonstrates the use of the most important functions.

\subsection overview Overview

The ANSI C Spectral Radar SDK follows an object-oriented approach. 
All objects are represented by pointers where appropriate typedefs are provided for convenience. The defined types are called Handles and are used as 
Return values when created and are passed as value when used. All functionality has been created with full LabVIEW compatibility in mind and it should 
be possible to use the SDK with most other programming languages as well. 
The most important handles are given in the following sections.

\subsection sec_datahandles Data Handle (DataHandle, ColoredDataHandle, ComplexDataHandle, RawDataHandle)
Data acquired and used by the SDK is provided via data objects. A data object can contain 
- floating point data (via #DataHandle)
- complex floating point data (via #ComplexDataHandle)
- ARGB32 colored data (via #ColoredDataHandle)
- unprocessed RAW data (via #RawDataHandle)
The data objects store all information belonging to them, such as pixel data, spacing between pixels, comments attached to their data, etc.
Data objects are automatically resized if necessary and can contain 1-, 2- or 3-dimensional data. The dimensionality can be read by #getDataPropertyInt(), etc.
Direct access to their memory is possible via #getDataPtr(), etc.
Data properties can be read out via #getDataPropertyInt(), #getDataPropertyFloat(), etc. These include sizes along their first, second and third axis, 
physical spacing between pixels, their total range, etc.

\subsection sec_OCTDeviceHandle OCTDeviceHandle 
A handle specifying the OCT device that is used. In most cases the #OCTDeviceHandle is obtained using the #initDevice() function and needs to be closed 
after using by #closeDevice(). The complete device will be initialized, the SLD will be switched on and all start-up dependent calibration will be performed.
All hardware and hardware dependend actions require the #OCTDeviceHandle to be passed. These include for example 
- starting and stopping a measurement (#startMeasurement() and #stopMeasurement())
- getting properties of the device (#getDevicePropertyInt() and #getDevicePropertyFloat())

\subsection sec_processinghandle ProcessingHandle
The numerics and processing routines required in order to create A-scans, B-scans and volumes out of directly measured spectra can be accessed via the #ProcessingHandle. 
When the #ProcessingHandle is created, all required temporary memory and routines are initialized and prepared and several threads are started.
In most cases the ideal way to create a processing handle is to use #createProcessingForDevice() which creates optimized processing algorithms for the #OCTDeviceHandle specified. 
If no device is available or the processing routines are to be tweaked manually #createProcessing() must be used. When all required processing is done, #closeProcessing 
must be used to stop all processing threads and free all temporary memory.
All functions whose output is dependent on the processing routines used have a #ProcessingHandle parameter. These include for example
- The #setProcessingParameterInt() and #setProcessingFlag() functions for setting parameters that are used for processing
- The #executeProcessing() function for triggering the processing of raw data

\subsection sec_probehandle ProbeHandle
The probe is the hardware used for scanning the sample, usually with help of galvanometric scanners. The object referenced by #ProbeHandle is 
responsible for creating scan patterns and holds all information and settings of the probe attached to the device. It needs to be calibrated to map suitable 
output voltage (for analog galvo drivers) or digital values (for digital galvo drivers) to scanning angles, inches or milimeters. In most cases this calibration data
is provided by *.ini files and the probe is initialized by #initProbe() where the probe configuration file name needs be specified as a string parameter.
Probes calibrated at Thorlabs will usually come with a factory-made probe configuration file which follows the nomenclature Probe + Objective Name.ini, e.g. "ProbeLSM03.ini"

If the probe is to be hardcoded into the software one can also provide an empty string as parameter and provide the configuration manually using the #setProbeParameterInt() 
and #setProbeParameterFloat() functions. When the Probe object is no longer needed, #closeProbe() must be called to free temporary memory.
All actions that depend on the probe configuration require a #ProbeHandle to be specified, such as:
- move galvo scanner to a specific position (#moveScanner()).
- create a scan pattern (#createBScanPattern()), see also \ref sec_scanpatternhandle. 
- set calibration parameters for a specific probe (#setProbeParameterFloat() and #setProbeParameterInt())

\subsection sec_scanpatternhandle ScanPatternHandle
A scan pattern is used to specifiy the points on the probe to scan during data acquisition, and its information is accessible via the #ScanPatternHandle. 
A dedicated function can be used to create a specific scan pattern, such as #createBScanPattern() for a simple B-scan or #createVolumePattern() for a simple volume scan. 
When the scan pattern is no longer needed its ressources can be freed using #clearScanPattern(). 
The #ScanPatternHandle needs to be specified to all functions that need information on the resulting scan. For example:
- creating a pattern (#createBScanPattern(), #createVolumePattern(), etc.)
- starting a measurement (#startMeasurement())

\subsection sec_otherhandles Other Handles
Other Handles that are used in the Spectral Radar SDK are
- #DopplerProcessingHandle: Handle to Doppler processing routines that can be used to transform complex data to Doppler phase and amplitude signals.
- #SettingsHandle: Handle to an INI file that can be read and written to without explicitly taking care of parsing the file.
- #Coloring32BitHandle: Handle to processing routines that can map floating point data to 32 bit color data. 

\section first_steps First Steps
The following section describes first steps that are needed to acquire data with the Spectral Radar SDK.
\subsection init Initializing The Device

The easiest way to initialize the device is to use the #initDevice() function. It returns an approprate #OCTDeviceHandle that can be used to identifiy the device:
\code
OCTDeviceHandle Dev = initDevice();
// Acquire data, processing, direct hardware access...
closeDevice(Dev);
\endcode

\subsection processing Creating Processing Routines
In most cases raw data acquired by the OCT device needs to be transformed usings a Fast Fourier transform and other pre- and postprocessing algorithms. 
To get a #ProcessingHandle on these algorithms the most convenient way is to use the #createProcessingForDevice() functionality which requires a valid #OCTDeviceHandle:
\code
// ...
ProcessingHandle Proc = createProcessingForDevice(Dev);
// acquire data and perform processing
closeProcessing(Proc);
// ...
\endcode

\subsection scanpattern Creating A Scan Pattern
In order to scan a sample and acquire B-scan OCT data one needs to specifiy a scan pattern that describes at which point to acquire data. To get the data of a 
simple B-Scan on can simply use #createBScanPattern():
\code 
// ...
ProbeHandle Probe = initProbe(Dev, "Probe"); 
ScanPatternHandle Pattern = createBScanPattern(Probe, 2.0, 512); // get B-scans with 2.0mm scanning angle and 512 A-scans per B-scan 
// acquire data, ...
clearScanPattern(Pattern);
closeProbe(Probe);
// ...
\endcode

\subsection Asynchronous Acquisition
The most convenient and fast way to acquire data is to acquire data asynchronously. For this one starts a measurement using #startMeasurement() and retrieves 
the latest available #getRawData(). The memory needed to store the data needs to be allocated first:
\code
int i;


RawDataHandle Raw = createRawData();
DataHandle BScan = createRealData();
startMeasurement(Dev, Pattern, Acquisition_ASyncContinuous);

for(i=0; i<1000; ++i) // get 1000 B-scans
{
    getRawData(Raw);
    setProcessedDataOutput(Proc, BScan);
    executeProcessing(Proc, Raw);
    // data is now in BScan...
    // do something with the data...
}

stopMeasurement(Dev);
clearData(BScan);
clearRawData(Raw);
\endcode

\section ErrorHandling Error Handling
Error handling is done by calling the function #getError(). The function will return an #ErrorCode and if the result is not NO_ERROR an error string 
will be provided giving details about the problem.
\code
#define ERROR_STRLEN 1024;
//...
char error[ERROR_STRLEN];
OCTDeviceHandle Dev = initDevice();
if(!getError(error, ERROR_STRLEN)) // check whether the previous calls to SDK functions caused an error
{
    printf("An error occurred: %s", error);
}
// ...
\endcode
*/

/// \file SpectralRadar.h
/// \brief Header containing all functions of the Spectral Radar SDK. This SDK can be used for Callisto, Ganymede, Hyperion and Telesto devices.
///
/// \def SPECTRALRADAR_API
/// \brief Export/Import of define of DLL members.

#ifdef SPECTRALRADAR_EXPORTS
#define SPECTRALRADAR_API __declspec(dllexport)
#else
#define SPECTRALRADAR_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
	/// \typedef BOOL
	/// \brief A standard boolean data type used in the API.
	typedef int BOOL;

	/// \def TRUE
	/// \brief TRUE for use with data type #BOOL.
#define TRUE 1

	/// \def FALSE
	/// \brief FALSE for use with data type #BOOL.
#define FALSE 0

	/// \struct ComplexFloat
	/// \ingroup Data
	/// \brief A standard complex data type that is used to access complex data.
	// needed for compatibility reasons if compiled with x byte aligned struct data.
	typedef struct {
		/// data[0] is the real part and data[1] is the imaginary part. 
		float data[2];
	} ComplexFloat;

	/// \typedef RawDataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding the unprocessed raw data. 
	struct C_RawData;
	typedef struct C_RawData* RawDataHandle;

	/// \typedef DataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding 1-, 2- or 3-dimensional floating point data.
	struct C_Data;
	typedef struct C_Data* DataHandle;

	/// \typedef ColoredDataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding 1-, 2- or 3-dimensional colored data.
	struct C_ColoredData;
	typedef struct C_ColoredData* ColoredDataHandle;

	/// \typedef ComplexDataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding complex 1-, 2- or 3-dimensional complex floating point data.
	struct C_ComplexData;
	typedef struct C_ComplexData* ComplexDataHandle;

	/// \typedef BufferHandle
	/// \ingroup Buffer
	/// \brief The BufferHandle identifies a data buffer.
	struct C_Buffer;
	typedef struct C_Buffer* BufferHandle;

	/// \typedef OCTDeviceHandle
	/// \ingroup Hardware
	/// \brief The OCTDeviceHandle type is used as Handle for using the SpectralRadar.
	struct C_OCTDevice;
	typedef struct C_OCTDevice* OCTDeviceHandle;

	/// \typedef ProbeHandle
	/// \ingroup Probe
	/// \brief Handle for controlling the galvo scanner.
	struct C_Probe;
	typedef struct C_Probe* ProbeHandle;

	/// \typedef ScanPatternHandle
	/// \ingroup ScanPattern
	/// \brief Handle for controlling the scan pattern.
	struct C_ScanPattern;
	typedef C_ScanPattern* ScanPatternHandle;

	/// \typedef ProcessingHandle
	/// \ingroup Processing
	/// \brief Handle for a processing routine.
	struct C_Processing;
	typedef struct C_Processing* ProcessingHandle; 

	/// \typedef DopplerProcessingHandle
	/// \ingroup Doppler
	/// \brief Handle used for Doppler processing.
	struct C_DopplerProcessing;
	typedef struct C_DopplerProcessing* DopplerProcessingHandle;

	/// \typedef PolarizationProcessingHandle
	/// \ingroup Polarization
	/// \brief Handle used for Polarization processing.
	struct C_PolarizationProcessing;
	typedef struct C_PolarizationProcessing* PolarizationProcessingHandle;

	/// \typedef Coloring32BitHandle
	/// \ingroup Coloring
	/// \brief Handle for routines that color avaible scans for displaying.
	struct C_Coloring32Bit;
	typedef struct C_Coloring32Bit* Coloring32BitHandle;    

	/// \typedef ImageFieldHandle
	/// \ingroup Data
	/// \brief Handle to the image field description
	struct C_ImageFieldCorrection;
	typedef struct C_ImageFieldCorrection* ImageFieldHandle;

	/// \typedef VisualCalibrationHandle
	/// \ingroup Helper
	/// \brief Handle to the visual galvo calibration class
	struct C_VisualCalibration;
	typedef struct C_VisualCalibration* VisualCalibrationHandle;

	/// \typedef MarkerListHandle
	/// \ingroup Marker
	/// \brief Handle to the marker list class
	struct C_MarkerList;
	typedef struct C_MarkerList* MarkerListHandle;

	/// \typedef OCTFileHandle
	/// \ingroup Data
	/// \brief Handle to the OCT file class
	struct C_OCTFileHandle;
	typedef struct C_FileHandling* OCTFileHandle;
	

	/// \fn unsigned long InterpretReferenceIntensity(float);
	/// \ingroup Helper
	/// \brief interprets the reference intensity and gives a color code that reflects its state. 
	/// 
	/// Possible colors include: 
	/// - red = 0x00FF0000 (bad intensity);
	/// - orange = 0x00FF7700 (okay intensity);
	/// - green = 0x0000FF00 (good intensity);
	/// \param intensity the current reference intensity as a value between 0.0 and 1.0
	/// \return the color code reflecting the state of the refernce intensity	
	SPECTRALRADAR_API unsigned long InterpretReferenceIntensity(float intensity);

	// ERROR HANDLING

	/// \defgroup ErrorHandling Error Handling
	/// \brief Error handling
    ///
	/// \enum ErrorCode
	/// \ingroup ErrorHandling
	/// \brief This enum is used to describe errors that occur when operating an OCT device.
	///
	/// \warning Error codes and error description texts are subject to change in future releases.
	typedef enum {
		/// No error occurred. This entry can be cast to FALSE.
		NoError = 0x00000000,
		/// Error occurred. This entry can be cast to TRUE.
		Error = 0xE0000000
	} ErrorCode;

	/// \fn ErrorCode getError(char* Message, int StringSize);
	/// \ingroup ErrorHandling
	/// \brief Returns an error code and a message if an error occurred. The error flag will be cleared.
	/// \param Message Error message describing the error.
	/// \param StringSize Size of the string that was given to Message. 
	SPECTRALRADAR_API ErrorCode getError(char* Message, int StringSize);

	// DATA ACCESS
	
	/// \defgroup Data Data Access
	/// \brief Functions for accessing the information stored in data objects. 
    ///
	/// \enum RawDataPropertyInt
	/// \ingroup Data
	/// \brief Specifies properties of RawData
	typedef enum {
		/// Size of the first dimension. This will be the spectral dimension, i. e. z-dimension prior to Fourier transformation. 
		RawData_Size1,
		/// Size of the second dimension. This is a transversal axis (x). 
		RawData_Size2,
		/// Size of the third dimension. This is a transversal axis (y). 
		RawData_Size3,
		/// The number of elements in the raw data object.
		RawData_NumberOfElements, 
		/// The size of the data object in bytes.
		RawData_SizeInBytes,
		/// The number of bytes of a single element, i. e. the data type of the raw data
		RawData_BytesPerElement,
		/// The number of lost frames during data acqusition.
		RawData_LostFrames
	} RawDataPropertyInt;

		/// \enum DataPropertyInt
	/// \ingroup Data
	/// \brief Selects integer point data property.
	typedef enum {
		/// Dimension of the data object. Usually 1, 2 or 3. 0 indicates empty data.
		Data_Dimensions,
		/// Size of the first dimension. For OCT data this is usually the longitudinal axis (z)
		Data_Size1,
		/// Size of the first dimension. For OCT data this is usually a transversal axis (x)
		Data_Size2,
		/// Size of the first dimension. For OCT data this is usually a transversal axis (y)
		Data_Size3,
		/// The number of elements in the data object.
		Data_NumberOfElements,
		/// The size of the data object in bytes.
		Data_SizeInBytes,
		/// The number of bytes of a single element.
		Data_BytesPerElement
	} DataPropertyInt;

	/// \enum DataPropertyFloat
	/// \ingroup Data
	/// \brief Selects floating point data property.
	typedef enum {
		/// Spacing between two subsequent data elements in direction of the first axis in physical units.
		Data_Spacing1,
		/// Spacing between two subsequent data elements in direction of the second axis in physical units.
		Data_Spacing2,
		/// Spacing between two subsequent data elements in direction of the third axis in physical units.
		Data_Spacing3,
		/// Total range of the data in direction of the first axis in physical units.
		Data_Range1,
		/// Total range of the data in direction of the second axis in physical units.
		Data_Range2,
		/// Total range of the data in direction of the third axis in physical units.
		Data_Range3
	} DataPropertyFloat;

	/// \enum DataAnalyzation
	/// \ingroup Data
	/// \brief Selects data property to analyze.
	typedef enum {
		/// Minimum of the values in the data.
		Data_Min,
		/// Arithmetic mean of all values in the data.
		Data_Mean,
		/// Maximum of the values in the data.
		Data_Max,
		/// The depth of the maximum of the values in the data.
		Data_MaxDepth
	} DataAnalyzation;

	/// \enum AScanAnalyzation
	/// \ingroup Data
	/// \brief Selects an appropriate A-Scan analyzation.
	typedef enum {
		/// Noise of the A-scan in dB. This assumes that no signal is present in the A-scan.
		/// The noise is computed by averaging all fourier channels larger than 50.
		Data_Noise_dB,
		/// Noise of the A-scan in electrons. This assumes that no signal is present in the A-scan.
		/// The noise is computed by averaging all fourier channels larger than 50.
		Data_Noise_electrons,
		/// Peak position of the highest peak in pixels. 
		/// The peak position is determined by computing a parable going through the maximum value point and its surrounding pixels. 
		/// The position of the maximum is used.
		Data_PeakPos_Pixel,
		/// Peak position of the highest peak in physical units. 
		/// The peak position is determined by computing a parable going through the maximum value point and its surrounding pixels. 
		/// The position of the maximum is used. Physical coordinates are computed by using the calibrated zSpacing property of the device. 
		/// The concrete physical units of the return value depends on the calibration.
		Data_PeakPos_PhysUnits,
		/// Peak height of the highest peak in dB.
		/// The peak hieght is determined by computing a parable going through the maximum value point and its surrounding pixels. 
		/// The height of the resulting parable is returned. 
		Data_PeakHeight_dB,
		/// Signal width at -6dB. This is the FWHM.
		Data_PeakWidth_6dB,
		/// Signal width at -20dB.
		Data_PeakWidth_20dB,
		/// Signal width at -40dB.
		Data_PeakWidth_40dB
	} AScanAnalyzation;

	/// \enum DataOrientation
	/// \ingroup Data
	/// \brief Selects the orientation of the data.
	typedef enum {
		DataOrientation_ZXY,
		DataOrientation_ZYX,
		DataOrientation_XZY,
		DataOrientation_XYZ,
		DataOrientation_YXZ,		
		DataOrientation_YZX,
		DataOrientation_ZTX,
		DataOrientation_ZXT,
	} DataOrientation;



	/// \fn int getDataPropertyInt(DataHandle Data, DataPropertyInt Selection)
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified data.
    ///
	/// \fn double getDataPropertyFloat(DataHandle Data, DataPropertyFloat Selection)
	/// \ingroup Data
	/// \brief Returns the selected floating point property of the specified data.
    ///
	/// \fn void copyData(DataHandle DataSource, DataHandle DataDestination);
	/// \ingroup Data
	/// \brief Copies the content of the specified source to the specified destination.
    ///
	/// \fn void copyDataContent(DataHandle, float* )
	/// \ingroup Data
	/// \brief Copies the data in the specified data object (#DataHandle) into the specified pointer.
    ///
	/// \fn float* getDataPtr(DataHandle Data)
	/// \ingroup Data
	/// \brief Returns a pointer to the content of the specified data.
	///
	/// \fn void reserveData(DataHandle Data, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Reserves the amount of data specified. This might improve performance if appending data to the #DataHandle as no additional
	/// memory needs to be reserved then. 
    ///
	/// \fn void resizeData(DataHandle Data, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Resizes the respective data object. In general the data will be 1-dimensional if Size2 and Size3 are equal to 
	/// 1, 2-dimensional if Size3 is equal to 1 dn 3-dimensional if all, Size1, Size2, Size3, are unequal to 1.
    ///
	/// \fn void setDataRange(DataHandle Data, double range1, double range2, double range3)
	/// \ingroup Data
	/// \brief Sets the range in mm in the 3 axes represented in the RealData buffer.
	/// 
	/// \fn void setDataContent(DataHandle Data, float* NewContent)
	/// \ingroup Data
	/// \brief Sets the data content of the data object. The data chung pointed to by NewContent needs to be of the size expected by the data object, i. e. Size1*Size2*Size*sizeof(float).
    ///
	/// \fn void DataOrientation getDataOrientation(DataHandle Data)
	/// \ingroup Data
	/// \brief Returns the data orientation of the data object.
	///
	/// \fn void setDataOrientation(DataHandle Data, DataOrientation)
	/// \ingroup Data
	/// \brief Sets the data oritentation of the data object to the given orientation.

	// FLOATING POINT DATA
	SPECTRALRADAR_API int getDataPropertyInt(DataHandle Data, DataPropertyInt Selection);
	SPECTRALRADAR_API double getDataPropertyFloat(DataHandle Data, DataPropertyFloat Selection);
	SPECTRALRADAR_API void copyData(DataHandle DataSource, DataHandle DataDestination);
	SPECTRALRADAR_API void copyDataContent(DataHandle DataSource, float* Destination);
	SPECTRALRADAR_API float* getDataPtr(DataHandle Data);
	SPECTRALRADAR_API void reserveData(DataHandle Data, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void resizeData(DataHandle Data, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void setDataRange(DataHandle Data, double range1, double range2, double range3);
	SPECTRALRADAR_API void setDataContent(DataHandle Data, float* NewContent);
	SPECTRALRADAR_API DataOrientation getDataOrientation(DataHandle Data);
	SPECTRALRADAR_API void setDataOrientation(DataHandle Data, DataOrientation Orientation);


	// COMPLEX FLOATING POINT DATA
	/// \fn int getComplexDataPropertyInt(ComplexDataHandle Data, DataPropertyInt Selection)
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified data.
	///
	/// \fn void copyComplexDataContent(ComplexDataHandle DataSource, ComplexFloat* Destination);
	/// \ingroup Data
	/// \brief Copies the content of the complex data to the pointer specified as destination.
	///
	/// \fn ComplexFloat* getComplexDataPtr(ComplexDataHandle Data);
	/// \ingroup Data
	/// \brief Returns a pointer to the data represented by the #ComplexDataHandle. The data is still managed by the #ComplexDataHandle object.
	/// 
	/// \fn void setComplexDataContent(ComplexDataHandle Data, ComplexFloat* NewContent);
	/// \ingroup Data
	/// \brief Sets the data content of the #ComplexDataHandle to the content specified by the pointer.
	///
	/// \fn void reserveComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Reserves the amount of data specified. This might improve performance if appending data to the #ComplexDataHandle as no additional
	/// memory needs to be reserved then. 
	///
	/// \fn void resizeComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Resizes the respective data object. In general the data will be 1-dimensional if Size2 and Size3 are equal to 
	/// 1, 2-dimensional if Size3 is equal to 1 dn 3-dimensional if all, Size1, Size2, Size3, are unequal to 1.
	///
	/// \fn void setComplexDataRange(ComplexDataHandle Data, double range1, double range2, double range3)
	/// \ingroup Data
	/// \brief Sets the range in mm in the 3 axes represented in the RealData buffer.
	SPECTRALRADAR_API int getComplexDataPropertyInt(ComplexDataHandle Data, DataPropertyInt Selection);
	SPECTRALRADAR_API void copyComplexDataContent(ComplexDataHandle DataSource, ComplexFloat* Destination);
	SPECTRALRADAR_API ComplexFloat* getComplexDataPtr(ComplexDataHandle Data);
	SPECTRALRADAR_API void setComplexDataContent(ComplexDataHandle Data, ComplexFloat* NewContent);
	SPECTRALRADAR_API void reserveComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void resizeComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void setComplexDataRange(ComplexDataHandle Data, double range1, double range2, double range3);

	// COLORED (UNSIGNED 32BIT) DATA
	/// \fn int getColoredDataPropertyInt(ColoredDataHandle ImageData, DataPropertyInt Selection);
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified colored data.
	///
	/// \fn int getColoredDataPropertyFloat(ColoredDataHandle ImageData, DataPropertyFloat Selection);
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified colored data.
	///
	/// \fn void copyColoredData(ColoredDataHandle ImageSource, ColoredDataHandle ImageDestionation);
	/// \ingroup Data
	/// \brief Copies the contents of the specified #ColoredDataHandle to the specified destination #ColoredDataHandle.
	///
	/// \fn void copyColoredDataContent(ColoredDataHandle ImageSource, unsigned long* Destination);
	/// \ingroup Data
	/// \brief Copies the data in the specified colored data object (#ColoredDataHandle) into the specified pointer.
	/// 
	/// \fn void copyColoredDataContentAligned(ColoredDataHandle ImageSource, unsigned long* Destination, int Alignment1);
	/// \ingroup Data
	/// \brief Copies the data in the specified colored data object (#ColoredDataHandle) into the specified pointer. This function assues the data to be aligned accordingly.
	///
	/// \fn unsigned long* getColoredDataPtr(ColoredDataHandle ColData)
	/// \ingroup Data
	/// \brief Returns a pointer to the content of the specified #ColoredDataHandle.
	///
	/// \fn void resizeColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Resizes the respective colored data object. In general the data will be 1-dimensional if Size2 and Size3 are equal to 
	/// 1, 2-dimensional if Size3 is equal to 1 dn 3-dimensional if all, Size1, Size2, Size3, are unequal to 1.
	///
	/// \fn void reserveColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Reserves the amount of colored data specified. This might improve performance if appending data to the #ColoredDataHandle as no additional
	/// memory needs to be reserved then. 
	///
	/// \fn  void setColoredDataContent(ColoredDataHandle ColData, unsigned long* NewContent)
	/// \ingroup Data
	/// \brief Sets the data content of the colored data object. The data chung pointed to by NewContent needs to be of the size expected 
	/// by the data object, i. e. Size1*Size2*Size*sizeof(unsigned long).
	///
	/// \fn void setColoredDataRange(ColoredDataHandle Data, double range1, double range2, double range3)
	/// \ingroup Data
	/// \brief Sets the range in mm in the 3 axes represented in the data object buffer.
	/// 
	/// \fn DataOrientation getColoredDataOrientation(ColoredDataHandle Data)
	/// \ingroup Data
	/// \brief Returns the data orientation of the colored data object.
	///
	/// \fn void setColoredDataOrientation(ColoredDataHandle Data, DataOrientation)
	/// \ingroup Data
	/// \brief Sets the data oritentation of the colored data object to the given orientation.
	SPECTRALRADAR_API int getColoredDataPropertyInt(ColoredDataHandle ColData, DataPropertyInt Selection);
	SPECTRALRADAR_API double getColoredDataPropertyFloat(ColoredDataHandle ColData, DataPropertyFloat Selection);
	SPECTRALRADAR_API void copyColoredData(ColoredDataHandle ImageSource, ColoredDataHandle ImageDestionation);
	SPECTRALRADAR_API void copyColoredDataContent(ColoredDataHandle Source, unsigned long* Destination);
	SPECTRALRADAR_API void copyColoredDataContentAligned(ColoredDataHandle ImageSource, unsigned long* Destination, int Alignment1);
	SPECTRALRADAR_API unsigned long* getColoredDataPtr(ColoredDataHandle ColData);
	SPECTRALRADAR_API void resizeColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void reserveColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void setColoredDataContent(ColoredDataHandle ColData, unsigned long* NewContent);
	SPECTRALRADAR_API void setColoredDataRange(ColoredDataHandle Data, double range1, double range2, double range3);
	SPECTRALRADAR_API DataOrientation getColoredDataOrientation(ColoredDataHandle Data);
	SPECTRALRADAR_API void setColoredDataOrientation(ColoredDataHandle Data, DataOrientation);

	// RAW DATA
	/// \fn void getRawDataSize(RawDataHandle, int*, int*, int* );
	/// \ingroup Data
	/// \brief Returns the size of the specified raw data (#RawDataHandle).
	///
	/// \fn void copyRawDataContent(RawDataHandle RawDataSource, void* DataContent);
	/// \ingroup Data
	/// \brief Copies the content of the raw data into the specified buffer. The suer needs to assure that enough memory is allocated. 
    ///
	/// \fn void* getRawDataPtr(RawDataHandle RawDataSource);
	/// \ingroup Data
	/// \brief Returns the pointer to the raw data content. The pointer might no longer after additional actions using the RawDataHandle. 
    ///
	/// \fn int getRawDataPropertyInt(RawDataHandle RawData, RawDataPropertyInt Property);
	/// \ingroup Data
	/// \brief Returns a raw data property
    ///
	/// \fn void setRawDataBytesPerPixel(RawDataHandle Raw, int BytesPerPixel);
	/// \ingroup Data
	/// \brief Sets the bytes per pixel for raw data.
	///
	/// \fn void reserveRawData(DataHandle Data, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Reserves the amount of data specified. This might improve performance if appending data to the #RawDataHandle as no additional memory needs to be reserved then. 
    ///
	/// \fn void resizeRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Resizes the specified raw data buffer accordingly.
    ///
	/// \fn void setRawDataContent(RawDataHandle RawDataSource, void* NewContent);
	/// \ingroup Data
	/// \brief Sets the content of the raw data buffer. The size of the RawDataHandle needs to be adjusted first, as otherwise not all data might be copied. 
    ///
	/// \fn void setScanSpectra(RawDataHandle RawData, int NumberOfScanRegions, int* ScanRegions);
	/// \ingroup Data
	/// \brief Sets the number of the spectra in the raw data that are used for creating A-scan/B-scan data. 
    ///
	/// \fn void setApodizationSpectra(RawDataHandle RawData, int NumberOfScanRegions, int* ApodizationRegions)
	/// \ingroup Data
	/// \brief Sets the number of the spectra in the raw data that contain data useful as apodization spectra. 
    ///
	/// \fn  int getNumberOfScanRegions(RawDataHandle Raw)
	/// \ingroup Data
	/// \brief Returns the number of regions that have been acquired that contain scan data, i. e. spectra that are used to compute A-scans.
    ///
	/// \fn  int getNumberOfApodizationRegions(RawDataHandle Raw)
	/// \ingroup Data
	/// \brief Returns the number of regions in the raw data containing spectra that are supposed to be used for apodization.
    ///
	/// \fn  void getScanSpectra(RawDataHandle Raw, int* SpectraIndex)
	/// \ingroup Data
	/// \brief Returns the indices of spectra that contain scan data, i. e. spectra that are supposed to be used to compute A-scans. 
	/// 
	/// An array needs to be provided that has twice the sice of the number of scan regions which can be obtained by getNumberOfScanRegions()
    ///
	/// \fn  void getApodizationSpectra(RawDataHandle Raw, int* SpectraIndex)
	/// \ingroup Data
	/// \brief Returns the indices of spectra that contain apodization data, i. e. spectra that are supposed to be used as input for apodization. 
	/// 
	/// An array needs to be provided that has twice the sice of the number of apodization regions which can be obtained by getNumberOfApodizationRegions()

	SPECTRALRADAR_API void getRawDataSize(RawDataHandle Raw, int* SizeX, int* SizeY, int* SizeZ);
	SPECTRALRADAR_API void copyRawDataContent(RawDataHandle RawDataSource, void* DataContent);
	SPECTRALRADAR_API void* getRawDataPtr(RawDataHandle RawDataSource);
	SPECTRALRADAR_API int getRawDataPropertyInt(RawDataHandle RawData, RawDataPropertyInt Property);
	SPECTRALRADAR_API void setRawDataBytesPerPixel(RawDataHandle Raw, int BytesPerPixel);
	SPECTRALRADAR_API void resizeRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void reserveRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);
	SPECTRALRADAR_API void setRawDataContent(RawDataHandle RawDataSource, void* NewContent);
	SPECTRALRADAR_API void setScanSpectra(RawDataHandle RawData, int NumberOfScanRegions, int* ScanRegions);
	SPECTRALRADAR_API void setApodizationSpectra(RawDataHandle RawData, int NumberOfScanRegions, int* ApodizationRegions);
	SPECTRALRADAR_API int getNumberOfScanRegions(RawDataHandle Raw);
	SPECTRALRADAR_API int getNumberOfApodizationRegions(RawDataHandle Raw);
	SPECTRALRADAR_API void getScanSpectra(RawDataHandle Raw, int* SpectraIndex);
	SPECTRALRADAR_API void getApodizationSpectra(RawDataHandle Raw, int* SpectraIndex);

	// DATA CREATION/CLEARING
	/// \defgroup DataCreation Data Creation and Clearing
	/// \brief Functions to create and clear object containing data.
    ///
	/// \fn RawDataHandle createRawData()
	/// \ingroup DataCreation
	/// \brief Creates a raw data object (#RawDataHandle).
    ///
	/// \fn void clearRawData(RawDataHandle)
	/// \ingroup DataCreation
	/// \brief Clears a raw data object (#RawDataHandle)
    ///
	/// \fn DataHandle createData(void);
	/// \ingroup DataCreation
	/// \brief Creates a 1-dimensional data object, containing floating point data.
    ///
	/// \fn DataHandle createGradientData(int Size)
	/// \ingroup DataCreation
	/// \brief Creates a 1-dimensional data object, containing floating point data with equidistant arranged values between [0, size-1] with distance 1/(size-1).
	///
	/// \fn void clearData(DataHandle)
	/// \ingroup DataCreation
	/// \brief Clears the specified #DataHandle, #DataHandle, #DataHandle or #DataHandle objects.
    ///
	/// \fn  ColoredDataHandle createColoredData(void)
	/// \ingroup DataCreation
	/// \brief Creates a colored data object (#ColoredDataHandle).
    ///
	/// \fn void clearColoredData(ColoredDataHandle)
	/// \ingroup DataCreation
	/// \brief Clears a colored volume object. 
    ///
	/// \fn ComplexDataHandle createComplexData(void)
	/// \ingroup DataCreation
	/// \brief Creates a data object holding complex data.
    ///
	/// \fn void clearComplexData(ComplexDataHandle Data)
	/// \ingroup DataCreation
	/// \brief Clears a data object holding complex data.

	SPECTRALRADAR_API RawDataHandle createRawData(void);
	SPECTRALRADAR_API void clearRawData(RawDataHandle Raw);

	SPECTRALRADAR_API DataHandle createData(void);
	SPECTRALRADAR_API DataHandle createGradientData(int Size);
	SPECTRALRADAR_API void clearData(DataHandle Data);

	SPECTRALRADAR_API ColoredDataHandle createColoredData(void);
	SPECTRALRADAR_API void clearColoredData(ColoredDataHandle Volume);

	SPECTRALRADAR_API ComplexDataHandle createComplexData(void);
	SPECTRALRADAR_API void clearComplexData(ComplexDataHandle Data);

	// GROUP HARDWARE

	/// \defgroup Hardware Hardware 
	/// \brief Functions providing direct access to OCT Hardware functionality.
    ///
	/// \enum DevicePropertyFloat
	/// \ingroup Hardware
	/// \brief Properties of the device that can be read or measured.
	typedef enum {
		/// The full well capacity of the device.
		Device_FullWellCapacity,
		/// The spacing between two pixels in an A-scan.
		Device_zSpacing,
		/// The maximum measurement range for an A-scan.
		Device_zRange,
		/// The minimum expected dB value for final data.
		Device_SignalAmplitudeMin_dB,
		/// The typical low dB value for final data.
		Device_SignalAmplitudeLow_dB,
		/// The typical high dB value for final data.
		Device_SignalAmplitudeHigh_dB,
		/// The maximum expected dB value for final data.
		Device_SignalAmplitudeMax_dB,
		/// Scaling factor between binary raw data and electrons/photons
		Device_BinToElectronScaling,
		/// Internal device temperature in degrees C
		Device_Temperature,
		/// Absolute power-on time of the SLD since first start in seconds
		Device_SLD_OnTime_sec,
		/// The center wavelength of the device
		Device_CenterWavelength_nm,
		/// The spectral width of the spectrometer
		Device_SpectralWidth_nm
	} DevicePropertyFloat;

	/// \enum DevicePropertyInt
	/// \ingroup Hardware
	/// \brief Properties of the device that can be read or measured.
	typedef enum {
		/// The number of pixels provided by the spectrometer.
		Device_SpectrumElements,
		/// The number of bytes one element of the spectrum occupies.
		Device_BytesPerElement,
		/// The maximum number of scans per dimension in the live volume rendering mode.
		Device_MaxLiveVolumeRenderingScans,
		/// Bit depth of the DAQ.
		Device_BitDepth,
		/// Flag indicating if the data is signed 
		Device_DataIsSigned
	} DevicePropertyInt;

	/// \enum ScanAxis
	/// \ingroup Hardware
	/// \brief used to select the axis for manual galvo operations.
	typedef enum {
		/// X-Axis of the scanner
		ScanAxis_X = 0,
		/// Y-Axis of the scanner
		ScanAxis_Y = 1
	} ScanAxis;

	/// \enum Device_CameraPreset
	/// \ingroup Hardware
	/// \brief Enum identifying sensitivity and acquisition speed of the device
	typedef enum {
		/// Default device preset. Most common compromise of acquisition speed and sensitivity.
		Device_CameraPreset_Default,
		/// Device preset 1
		Device_CameraPreset_1,
		/// Device preset 2
		Device_CameraPreset_2,
		/// Device preset 3
		Device_CameraPreset_3		
	} Device_CameraPreset;

	/// \fn OCTDeviceHandle initDevice(void);
	/// \ingroup Hardware
	/// \brief Initializes the installed device.
	/// \return Handle to the initialized OCT device.
	/// 
	/// \fn void getDeviceType(OCTDeviceHandle Dev, char DevName[], int BufferSize)
	/// \ingroup Hardware
	/// \brief Gives the name of the device type that is given by the #OCTDeviceHandle.
	///
	/// \fn int getDeviceRevision(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Returns the revision of the device given by the #OCTDeviceHandle.
    ///
	/// \fn int void getDeviceSerialNumber(OCTDeviceHandle Dev, char DevName[], int BufferSize)
	/// \ingroup Hardware
	/// \brief Returns the serial number of the device given by the #OCTDeviceHandle.
	///
	/// \fn int getDevicePropertyInt(OCTDeviceHandle Dev, DevicePropertyInt Selection)
	/// \ingroup Hardware
	/// \brief Returns properties of the device belonging to the specfied #OCTDeviceHandle.
    ///
	/// \fn double getDevicePropertyFloat(OCTDeviceHandle Dev, DevicePropertyFloat)
	/// \ingroup Hardware
	/// \brief Returns properties of the device belonging to the specfied #OCTDeviceHandle.
    ///
	/// \fn void closeDevice(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Closes the device opened previously with initDevice.
	/// \param Dev The #OCTDeviceHandle that was initially provided by initDevice.
    ///
	/// \fn BOOL isDeviceOn(OCTDeviceHandle Handle)
	/// \ingroup Hardware
	/// \brief Returns if the device is switched on.
	/// \param Dev The #OCTDeviceHandle that was initially provided by initDevice.
	///
	/// \fn BOOL isVideoCameraAvailable(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Returns if the video camera is available.
	/// \param Dev The #OCTDeviceHandle that was initially provided by initDevice.
	///
	/// \fn BOOL isSLDAvailable(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Returns whethter the SLD is available.
	/// \param Dev The #OCTDeviceHandle that was initially provided by initDevice.
	///
	/// \fn void setSLD(OCTDeviceHandle Dev, BOOL);
	/// \ingroup Hardware
	/// \brief switches the SLD of the SpectralRadar device on and off.
	/// \param Dev handle to the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param OnOff TRUE switches the SLD on, FALSE swichted the SLD off.
    ///
	/// \fn void moveScanner(OCTDeviceHandle Dev, ProbeHandle, ScanAxis, double)
	/// \ingroup Hardware
	/// \brief manually moves the scanner to a given position
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe A handle to the probe (#ProbeHandle); whose galvo position is to be set.
	/// \param Axis the axis in which you want to set the position manually
	/// \param Position the actual position you want to move the galvo to.
    ///
	/// \fn void setLaserDiode(OCTDeviceHandle Dev, BOOL)
	/// \ingroup Hardware
	/// \brief switches the LaserDiode of the SpectralRadar device on and off.
	///
	/// \warning Not all devices are equiped 
	/// \param Dev handle to the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param OnOff TRUE switches the VisLD on, FALSE swichted the VisLD off.
	///
	/// \fn double getWavelengthAtPixel(OCTDeviceHandle Dev, int Pixel)
	/// \ingroup Hardware
	/// \brief Returns the wavelength at a speicified pixel of the spectrometer.
	/// \warning This function is still experimental and results might be incorrect. 
	///
	/// \fn void setCameraPreset(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc, Device_CameraPreset Preset);
	/// \ingroup Hardware
	/// \brief Sets the preset of the device. Using presets the sensitivity and acquisition speed of the device can be influenced.
	/// 
	/// \fn void Device_CameraPreset getCameraPreset(OCTDeviceHandle Dev);
	/// \ingroup Hardware
	/// \brief Gets the currently used device preset.
	///
	/// \fn void getCameraPresetDescription(OCTDeviceHandle Dev, Device_CameraPreset Preset, char* Description, int BufferSize);
	/// \ingroup Hardware
	/// \brief Returns a description of the selected device preset. Using the description more information about sensitivity and acquisition speed of the respective set can be found.
	///
	/// \fn void setTrigger(OCTDeviceHandle Dev, Device_TriggerType Trigger, double Timeout);
	/// \ingroup Hardware
	/// \brief Sets the trigger mode for the OCT device. Currently only implemented for NI framegrabber-based OCT systems. The timeout must be given in seconds.

	SPECTRALRADAR_API OCTDeviceHandle initDevice(void);
	SPECTRALRADAR_API void getDeviceType(OCTDeviceHandle Dev, char DevName[], int BufferSize);
	SPECTRALRADAR_API int getDeviceRevision(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void getDeviceSerialNumber(OCTDeviceHandle Dev, char DevName[], int BufferSize);
	SPECTRALRADAR_API int getDevicePropertyInt(OCTDeviceHandle Dev, DevicePropertyInt Selection);
	SPECTRALRADAR_API double getDevicePropertyFloat(OCTDeviceHandle Dev, DevicePropertyFloat Selection);
	SPECTRALRADAR_API void closeDevice(OCTDeviceHandle Dev);
	SPECTRALRADAR_API BOOL isDeviceOn(OCTDeviceHandle Handle);
	SPECTRALRADAR_API BOOL isVideoCameraAvailable(OCTDeviceHandle Dev);
	SPECTRALRADAR_API BOOL isSLDAvailable(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void setSLD(OCTDeviceHandle Dev, BOOL OnOff);
	SPECTRALRADAR_API void moveScanner(OCTDeviceHandle Dev, ProbeHandle Probe, ScanAxis Axis, double Position);
	SPECTRALRADAR_API void setLaserDiode(OCTDeviceHandle Dev, BOOL OnOff);
	SPECTRALRADAR_API double getWavelengthAtPixel(OCTDeviceHandle Dev, int Pixel);
	SPECTRALRADAR_API void setCameraPreset(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc, int Preset);
	SPECTRALRADAR_API int getCameraPreset(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void getCameraPresetDescription(OCTDeviceHandle Dev, int Preset, char* Description, int BufferSize);
	SPECTRALRADAR_API int getNumberOfCameraPresets(OCTDeviceHandle Dev);

	// GROUP Internal values
	
	/// \defgroup Internal Internal Values
	/// \brief Functions for access to all kinds of Digital-to-Analog and Analog-to-Digital on the device.
    ///
	/// \fn int getNumberOfInternalValues(OCTDeviceHandle);
	/// \ingroup Internal
	/// \brief Returns the number of Analog-to-Digital Converter present in the device.
    ///
	/// \fn void getInternalValueName(OCTDeviceHandle, int, char*, int NameStringSize, char*, int UnitStringSize);
	/// \ingroup Internal
	/// \brief Returns names and unit for the specified Analog-to-Digital Converter. 
	/// 
	/// The index is running number, starting with 0, smaller than the number specified by #getNumberOfInternalValues.
    ///
	/// \fn double getInternalValueByName(OCTDeviceHandle, const char*);
	/// \ingroup Internal 
	/// \brief Returns the value of the specified Analog-to-Digital Converter (ADC);. 
	///
	/// The ADC is specified by the name returned by #getInternalValueName.
    ///
	/// \fn double getInternalValueByIndex(OCTDeviceHandle, int);
	/// \ingroup Internal
	/// \brief Returns the value of the selected ADC. 
	/// 
	/// The index is running number, starting with 0, smaller than the number specified by #getNumberOfInternalValues.

	SPECTRALRADAR_API int getNumberOfInternalValues(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void getInternalValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);
	SPECTRALRADAR_API double getInternalValueByName(OCTDeviceHandle Dev, const char* Name);
	SPECTRALRADAR_API double getInternalValueByIndex(OCTDeviceHandle Dev, int Index);
	
	// PROBE
	
	/// \defgroup Probe Pattern Factory/Probe
	/// \brief Functions setting up a probe that can be used to create scan patterns.
    ///
	/// \enum ProbeParameterFloat
	/// \ingroup Probe
	/// \brief Parameters describing the behaviour of the Probe, such as calibration factors and scan parameters.
	///
	/// Computation of physical position and raw values for the scanner is done by
	/// PhyscialPosition = Factor * RawValue + Offset
	typedef enum {
		/// Factor for the x axis.
		Probe_FactorX, 
		/// Offset for the x axis.
		Probe_OffsetX,
		/// Factor for the y axis.
		Probe_FactorY,
		/// Offset for the y axis.
		Probe_OffsetY,
		/// Flyback time of the system. This time is usually needed to get from an apodization position to scan position and vice versa.
		Probe_FlybackTime_Sec,
		/// The scanning range is extended by a number of A-scans equivalent to the expansion time.
		Probe_ExpansionTime_Sec,
		/// The scan pattern is usually shifted by a number of A-scans equivalent to the rotation time.
		Probe_RotationTime_Sec,
		/// The expected scan rate. \warning In general the expected scan rate is set during initialization of the probe with respect to 
		/// the attached device. In most cases it should not be altered manually.
		Probe_ExpectedScanRate_Hz,
		/// The px/mm ratio in X direction for the BScan overlay on the video image.
		Probe_CameraScalingX,
		/// The BScan overlay X offset in pixels.
		Probe_CameraOffsetX,
		/// The px/mm ratio in Y direction for the BScan overlay on the video image.
		Probe_CameraScalingY,
		/// The BScan overlay Y offset in pixels.
		Probe_CameraOffsetY,
		/// Corrective rotation angle for the BScan overlay.
		Probe_CameraAngle,
		/// White balance settings will only take effect on initialization of the probe.
		/// White balance value for red channel of video camera (if -1, adjustment will be automatic).
		Probe_WhiteBalanceRed,
		/// White balance value for green channel of video camera (if -1, adjustment will be automatic).
		Probe_WhiteBalanceGreen,
		/// White balance value for blue channel of video camera (if -1, adjustment will be automatic).
		Probe_WhiteBalanceBlue,
		/// Maximum scan range in X direction
		Probe_RangeMaxX,
		/// Maximum scan range in Y direction
		Probe_RangeMaxY,
		/// Maximum galvo slope (accounting for the distortion capabilities of different galvo types)
		Probe_MaximumSlope_XY,
		/// Speckle size to be used for scan pattern computation if speckle reduction is switched on.
		Probe_SpeckleSize,
		/// X-position used to acquire the apodization spectrum
		Probe_ApoPosX,
		/// Y-position used to acquire the apodization spectrum
		Probe_ApoPosY
	} ProbeParameterFloat;

	/// \enum ProbeParameterInt
	/// \ingroup Probe
	/// \brief Parameters describing the behaviour of the Probe, such as calibration factors and scan parameters.
	typedef enum {
		/// The number of cycles used for apodization.
		Probe_ApodizationCycles,
		/// A factor used as oversampling.
		Probe_Oversampling,
		/// Automatic white balance for video camera, 0 == off, not 0 == on
		Probe_WhiteBalanceAutomatic,
		/// A factor used as oversampling of the slow scanner axis.
		Probe_Oversampling_SlowAxis,
		/// Number of speckles that are scanned over for averaging. Requires Oversampling >= SpeckleReduction
		Probe_SpeckleReduction,
		/// Shape of the maximum scan range: 0 is a rectangle, 1 is an ellipse
		Probe_MaxScanRangeShape
	} ProbeParameterInt;

	/// \enum ProbeFlag
	/// \ingroup Probe
	/// \brief Boolean parameters describing the behaviour of the Probe.
	typedef enum {
		/// Bool if the scan pattern in the video camera image is flipped around x-axis or not.
		Probe_CameraInverted_X,
		/// Bool if the scan pattern in the video camera image is flipped around y-axis or not.
		Probe_CameraInverted_Y,
		/// Boolean if the probe type uses a MEMS mirror or not, e.g. a handheld probe.
		Probe_HasMEMSScanner
	} ProbeFlag;

	/// \enum ProbeType 
	/// \ingroup Probe
	/// \brief Determines the kind of probe types
	typedef enum {
		/// Specifies the standard or general probe.
		ProbeType_Standard,
		/// Specfies the handheld probe.
		ProbeType_Handheld,
		/// Specifies the scientific probe.
		ProbeType_Scientific
	} ProbeType;

	/// \fn ProbeHandle initProbe(OCTDeviceHandle, const char*);
	/// \ingroup Probe
	/// \brief Initializes a probe specified by ProbeFile.
	///
	/// In older systems up until a manufacturing date of May 2011 either "Handheld" or "Microscope" are used. An according ini-file 
	/// (i. e. "Handheld.ini" or "Microscope.ini); will be loaded from the config path of the SpectralRadar installation containing all necessary information.
	/// With systems manufactured after May 2011 "Probe" should be used.
	///
	///	It is recommended to use #initStandardProbe for systems manufactured in or after May 2011.
	///
	/// \fn ProbeHandle initStandardProbe(OCTDeviceHandle Dev)
	/// \ingroup Probe
	/// \brief Creates a standard probe using the Probe.ini file. If this configuration file is not found, standard parameters without valid calibration will be used.
	///
	/// \fn ProbeHandle initProbeWithType(OCTDeviceHandle Dev, ProbeType Type)
	/// \ingroup Probe
	/// \brief Creates a standard probe for the given probe type but without valid calibration data .
	///
	/// \fn void saveProbe(ProbeHandle Probe, const char* ProbeFile);
	/// \ingroup Probe
	/// \brief Saves the current properties of the #ProbeHandle to a specified INI file to be reloaded using the #initProbe() function.
	///
	/// \fn void setProbeParameterInt(ProbeHandle, ProbeParameterInt, int)
	/// \ingroup Probe
	/// \brief Sets 
	///
	/// \fn void setProbeParameterFloat(ProbeHandle, ProbeParameterFloat, double)
	/// \ingroup Probe
	/// \brief Sets floating point parameters of the specified probe.
	///
	/// \fn int getProbeParameterInt(ProbeHandle, ProbeParameterInt)
	/// \ingroup Probe
	/// \brief Gets integer parameters of the specified probe.
	///
	/// \fn double getProbeParameterFloat(ProbeHandle, ProbeParameterFloat)
	/// \ingroup Probe
	/// \brief Gets floating point parameters of the specified probe.
	///
	/// \fn BOOL getProbeFlag(ProbeHandle Probe, ProbeFlag Selection)
	/// \ingroup Probe
	/// \brief Returns the selected boolean value of the specified probe.
	///
	/// \fn void getProbeName(ProbeHandle Probe, char ProbeName[], int BufferSize)
	/// \ingroup Probe
	/// \brief Returns the name of the specified probe.
	///
	/// \fn void setProbeName(ProbeHandle Probe, const char* ProbeName)
	/// \ingroup Probe
	/// \brief Sets the given name of the specified probe.
	///
	/// \fn void getProbeSerialNo(ProbeHandle Probe, char SerialNo[], int BufferSize)
	/// \ingroup Probe
	/// \brief Gets the serial number of the specified probe.
	///
	/// \fn void setProbeSerialNo(ProbeHandle Probe, const char* SerialNo)
	/// \ingroup Probe
	/// \brief Gets the serial number of the specified probe.
	///
	/// \fn void getProbeType(ProbeHandle Probe, char Type[], int BufferSize)
	/// \ingroup Probe
	/// \brief Gets the type of the specified probe.
	///
	/// \fn void setProbeType(ProbeHandle Probe, const char* Type);
	/// \ingroup Probe
	/// \brief Sets the type of the specified probe.
	///
	/// \fn void getProbeObjective(ProbeHandle Probe, char Objective[], int BufferSize)
	/// \ingroup Probe
	/// \brief Gets the objective of the specified probe.
	///
	/// \fn void setProbeObjective(ProbeHandle Probe, const char* Objective)
	/// \ingroup Probe
	/// \brief Sets the given objective of the specified probe.
	///
	/// \fn void closeProbe(ProbeHandle Probe);
	/// \ingroup Probe
	/// \brief Closes the probe and frees all memory associated with it.
	///
	/// \fn void blendEnFaceInCamera(ProbeHandle Probe, ScanPatternHandle Pattern, ColoredDataHandle EnFace2D, ColoredDataHandle Image, float Ratio, BOOL DenseView);
	/// \ingroup Probe
	/// \brief Blends the en-face image of a given volume acquisition on top of the video image. Can be used to calibrate the probe manually.
	///
	/// \fn void drawScanPatternOverlay(ProbeHandle Probe, ScanPatternHandle Pattern, ColoredDataHandle Image, float Opacity);
	/// \ingroup Probe
	/// \brief Draws the currently selected #ScanPatternHandle on the camera image given by the #ColoredDataHandle with a specified opacity. 
	/// It needs to be assured that the device is properly calibrated.
	/// 
	/// \fn void CameraPixelToPosition(ProbeHandle Probe, ColoredDataHandle Image, int PixelX, int PixelY, double* PosX, double* PosY);
	/// \ingroup Probe
	/// \brief Computes the physical position of a camera pixel of the video camera in the probe. It needs to be assured that the device is properly calibrated.
	/// 
	/// \fn void PositionToCameraPixel(ProbeHandle Probe, ColoredDataHandle Image, double PosX, double PosY, int* PixelX, int* PixelY);
	/// \ingroup Probe
	/// \brief Computes the pixel of the video camera corresponding to a physical position. It needs to be assured that the device is properly calibrated.
	///
	SPECTRALRADAR_API ProbeHandle initProbe(OCTDeviceHandle Dev, const char* ProbeFile);
	SPECTRALRADAR_API ProbeHandle initStandardProbe(OCTDeviceHandle Dev);
	SPECTRALRADAR_API ProbeHandle initProbeWithType(OCTDeviceHandle Dev, ProbeType Type);
	SPECTRALRADAR_API void saveProbe(ProbeHandle Probe, const char* ProbeFile);
	SPECTRALRADAR_API void setProbeParameterInt(ProbeHandle Probe, ProbeParameterInt Selection, int Value);
	SPECTRALRADAR_API void setProbeParameterFloat(ProbeHandle Probe, ProbeParameterFloat Selection, double Value);
	SPECTRALRADAR_API int getProbeParameterInt(ProbeHandle Probe, ProbeParameterInt Selection);
	SPECTRALRADAR_API double getProbeParameterFloat(ProbeHandle Probe, ProbeParameterFloat Selection);
	SPECTRALRADAR_API BOOL getProbeFlag(ProbeHandle Probe, ProbeFlag Selection);
	SPECTRALRADAR_API void getProbeName(ProbeHandle Probe, char ProbeName[], int BufferSize);
	SPECTRALRADAR_API void setProbeName(ProbeHandle Probe, const char* ProbeName);
	SPECTRALRADAR_API void getProbeSerialNo(ProbeHandle Probe, char SerialNo[], int BufferSize);
	SPECTRALRADAR_API void setProbeSerialNo(ProbeHandle Probe, const char* SerialNo);
	SPECTRALRADAR_API void getProbeType(ProbeHandle Probe, char Type[], int BufferSize);
	SPECTRALRADAR_API void setProbeType(ProbeHandle Probe, const char* Type);
	SPECTRALRADAR_API void getProbeObjective(ProbeHandle Probe, char Objective[], int BufferSize);
	SPECTRALRADAR_API void setProbeObjective(ProbeHandle Probe, const char* Objective);
	SPECTRALRADAR_API void closeProbe(ProbeHandle Probe);

	SPECTRALRADAR_API void blendEnFaceInCamera(ProbeHandle Probe, ScanPatternHandle Pattern, ColoredDataHandle EnFace2D, ColoredDataHandle Image, float Ratio, BOOL DenseView);
	// SPECTRALRADAR_API void drawScanPatternOverlay(ProbeHandle Probe, ScanPatternHandle Pattern, ColoredDataHandle Image, float Opacity);
	SPECTRALRADAR_API void CameraPixelToPosition(ProbeHandle Probe, ColoredDataHandle Image, int PixelX, int PixelY, double* PosX, double* PosY);
	SPECTRALRADAR_API void PositionToCameraPixel(ProbeHandle Probe, ColoredDataHandle Image, double PosX, double PosY, int* PixelX, int* PixelY);

	// SCAN PATTERN
	
	/// \defgroup ScanPattern Scan Pattern
	/// \brief Functions that describe the movement of the Scanner during measurement.
    ///
	/// \fn ScanPatternHandle createNoScanPattern(ProbeHandle Probe, int Scans, int NumberOfScans);
	/// \ingroup ScanPattern
	/// \brief Creates a simple scan pattern that does not move the galvo. Use this pattern for point scans and/or non-scanning probes.
	/// 
	/// \fn ScanPatternHandle createTriggerPattern(ProbeHandle Probe, int Scans);
	/// \ingroup ScanPattern
	/// \brief Creates a pattern only consisting of a specified amount of trigger signals.
    ///
	/// \fn ScanPatternHandle createBScanPattern(ProbeHandle Probe, double Range, int AScans, BOOL apodization);
	/// \ingroup ScanPattern
	/// \brief Creates a simple B-scan pattern that moves the galvo over a specified range.
	/// 
	/// \fn ScanPatternHandle createBilateralBScanPattern(ProbeHandle Probe, double Range, int AScans, double Shift);
	/// \ingroup ScanPattern
	/// \brief Creates a bilateral scan pattern. The contouring error can be influenced using the Shift parameter.
	///
	/// \fn ScanPatternHandle createBScanPatternManual(ProbeHandle Probe, double StartX, double StartY, double StopX, double StopY, int AScans, BOOL apodization);
	/// \ingroup ScanPattern
	/// \brief Creates a B-scan pattern specified by start and end points. 
	///
	/// \fn ScanPatternHandle createIdealBScanPattern(ProbeHandle Probe, double Range, int AScans);
	/// \ingroup ScanPattern
	/// \brief Creates an ideal B-scan pattern assuming scanners with infinite speed. No correction factors are taken into account. This is only used for internal
	/// purposes and not as a scan pattern designed to be output to the galvo drivers.
	///
	/// \fn ScanPatternHandle createCirclePattern(ProbeHandle Probe, double Radius, int AScans);
	/// \ingroup ScanPattern
	/// \brief Creates a circle scan pattern. 
	/// \warning Circle patterns cannot be rotated properly. 
    /// 
	/// \fn ScanPatternHandle createVolumePattern(ProbeHandle Probe, double RangeX, int SizeX, double RangeY, int SizeY);
	/// \ingroup ScanPattern
	/// \brief Creates a simple volume pattern.
	///
	/// The volume pattern consists of a single uninterrupted scan and all data is acquired in a single measurement. In contrast to a B-scan 
	/// stack pattern (see #createBScanStackPattern()) only one apodization is performed for the complete volume. 
	/// The complete volume will be returned in one raw data (#RawDataHandle) by calling #getRawData() if a continuous asynchroneous measurement 
	/// is performed (#Acquisition_AsyncContinuous).
	/// The volume will be returned slice-by-slice by calling #getRawData() if a single/finite asynchroneous measurement is performed (#Acquisition_AsyncFinite).
    ///
	/// \fn ScanPatternHandle createBScanStackPattern(ProbeHandle Probe, double RangeX, int SizeX, double RangeY, int SizeY);
	/// \ingroup ScanPattern
	/// \brief Creates a simple stack pattern.
	///
	/// The BScan stack pattern is a volume measurement which consists of several shifted B-Scan measurements. The resulting data will be 
	/// identical to a volume (see #createVolumePattern()) but an apodization is performed for each slice (B-scan).
	/// The complete volume will be returned in one raw data (#RawDataHandle) if a single asynchronous measurement is performed (#Acquisition_AsyncContinuous).
	/// The volume will be returned slice-by-slice by calling #getRawData() if a single/finite asynchronous measurement is performed (#Acquisition_AsyncFinite).
    ///
	/// \fn ScanPatternHandle createFreeformScanPattern(ProbeHandle Probe, float* positions, int size_x, int size_y, BOOL apodization);
	/// \ingroup ScanPattern
	/// \brief Creates a freeform scan pattern based on an array of positions.
	///
	///	The positions array must consist of pairs of x/y coordinates inside the valid scanning limits of the probe and contain all points of the scan pattern.
	/// size_x/size_y can be used analogue to the B-scan stack pattern to discern the length (size_x) and number (size_y) of single lines or sections inside the
	/// freeform scan pattern. The position array is taken as-is, so care must be taken to use sensible values with regard to the capabilities of the utilized scanner
	/// system and to the resolution of the system resp. the desired resolution of your scan pattern.
	///
	/// \fn ScanPatternHandle createFragmentedScanPattern(int ChunkSize, int NumberOfChunks);
	/// \ingroup ScanPattern
	/// \brief Creates a scan pattern which can be used to acquire a dataset of <NumberOfChunks> times <ChunkSize> A-scans at position 0/0.
	///	The Fragmented scan pattern can be compared in structure to a B-scan stack pattern with x and y ranges of 0; however the fragmented scan pattern behaves like a
	/// volume pattern in that it shows no delay between the respective chunks.
	///
	/// \fn void updateScanPattern(ScanPatternHandle Pattern)
	/// \ingroup ScanPattern
	/// \brief Updates the specfied pattern (#ScanPatternHandle);.
	///
	/// \fn void rotateScanPattern(ScanPatternHandle, double Angle);
	/// \ingroup ScanPattern
	/// \brief Rotates the specfied pattern (#ScanPatternHandle);.
    ///
	/// \fn void rotateScanPatternExt(ScanPatternHandle, double Angle, int index);
	/// \ingroup ScanPattern
	/// \brief Rotates the scan #index (0-based) of the specfied pattern (#ScanPatternHandle).
	///
	/// \fn void shiftScanPattern(ScanPatternHandle, double ShiftX, double ShiftY)
	/// \ingroup ScanPattern
	/// \brief Shifts the specified pattern (#ScanPatternHandle). 
	///
	/// \fn void shiftScanPatternExt(ScanPatternHandle, double ShiftX, double ShiftY, BOOL ShiftApo, int index)
	/// \ingroup ScanPattern
	/// \brief Shifts the scan #index (0-based) of the specified pattern (#ScanPatternHandle). 
	///
	/// \fn void zoomScanPattern(ScanPatternHandle Pattern, double Factor)
	/// \ingroup ScanPattern
	/// \brief Zooms the specified pattern (#ScanPatternHandle). 
	///
	/// \fn int getScanPatternLUTSize(ScanPatternHandle Pattern);
	/// \ingroup ScanPattern
	/// \brief Returns the number of data points the specified pattern (#ScanPatternHandle) used.
	///
	/// \fn void getScanPatternLUT(ScanPatternHandle Pattern, double* PosX, double* PosY);
	/// \ingroup ScanPattern
	/// \brief Returns the actual positions to be scanned with the specified pattern (#ScanPatternHandle).
    ///
	/// \fn void clearScanPattern(ScanPatternHandle Pattern);
	/// \ingroup ScanPattern
	/// \brief Clears the specified scan pattern (#ScanPatternHandle).



	SPECTRALRADAR_API ScanPatternHandle createNoScanPattern(ProbeHandle Probe, int Scans, int NumberOfScans);
	SPECTRALRADAR_API ScanPatternHandle createTriggerPattern(ProbeHandle Probe, int Scans);
	SPECTRALRADAR_API ScanPatternHandle createBScanPattern(ProbeHandle Probe, double Range, int AScans, BOOL apodization);
	SPECTRALRADAR_API ScanPatternHandle createBilateralBScanPattern(ProbeHandle Probe, double Range, int AScans, double Shift);
	SPECTRALRADAR_API ScanPatternHandle createBScanPatternManual(ProbeHandle Probe, double StartX, double StartY, double StopX, double StopY, int AScans, BOOL apodization);
	SPECTRALRADAR_API ScanPatternHandle createIdealBScanPattern(ProbeHandle Probe, double Range, int AScans);
	SPECTRALRADAR_API ScanPatternHandle createCirclePattern(ProbeHandle Probe, double Radius, int AScans);
	SPECTRALRADAR_API ScanPatternHandle createVolumePattern(ProbeHandle Probe, double RangeX, int SizeX, double RangeY, int SizeY);
	SPECTRALRADAR_API ScanPatternHandle createBScanStackPattern(ProbeHandle Probe, double RangeX, int SizeX, double RangeY, int SizeY);
	SPECTRALRADAR_API ScanPatternHandle createFreeformScanPattern(ProbeHandle Probe, float* positions, int size_x, int size_y, BOOL apodization);
	SPECTRALRADAR_API ScanPatternHandle createFragmentedScanPattern(int ChunkSize, int NumberOfChunks);
	SPECTRALRADAR_API void updateScanPattern(ScanPatternHandle Pattern);
	SPECTRALRADAR_API void rotateScanPattern(ScanPatternHandle Pattern, double Angle);
	SPECTRALRADAR_API void rotateScanPatternExt(ScanPatternHandle Pattern, double Angle, int index);
	SPECTRALRADAR_API void shiftScanPattern(ScanPatternHandle Pattern, double ShiftX, double ShiftY);
	SPECTRALRADAR_API void shiftScanPatternExt(ScanPatternHandle Pattern, double ShiftX, double ShiftY, BOOL ShiftApo, int Index);
	SPECTRALRADAR_API void zoomScanPattern(ScanPatternHandle Pattern, double Factor);
	SPECTRALRADAR_API int getScanPatternLUTSize(ScanPatternHandle Pattern);
	SPECTRALRADAR_API void getScanPatternLUT(ScanPatternHandle Pattern, double* PosX, double* PosY);
	SPECTRALRADAR_API void clearScanPattern(ScanPatternHandle Pattern);
	
	//  ACQUISITION

	/// \defgroup Acquisition Acquisition
	/// \brief Functions for acquisition.
    ///
	/// \enum AcquisitionType 
	/// \ingroup Acquisition
	/// \brief Determines the kind of acquisition process
	typedef enum {
		/// Specifies an asynchronous infinite/continuous measurement.
		Acquisition_AsyncContinuous,
		/// Specifies an asynchronous finite measurement.
		Acquisition_AsyncFinite,
		/// Specfies a synchronous measurement.
		Acquisition_Sync
	} AcquisitionType;

	/// \fn void startMeasurement(OCTDeviceHandle, ScanPatternHandle, AcquisitionType);
	/// \ingroup Acquisition
	/// \brief starts a continuous measurement BScans. 
	///
	/// Scanning takes place according to the specified scan pattern handle. 
	/// Data can be recorded using the #getRawData() function.  
	/// If you are done, call #stopMeasurement().
	/// \param Dev The #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Pattern the #ScanPatternHandle
    ///
	/// \fn void getRawData(OCTDeviceHandle, RawDataHandle);
	/// \ingroup Acquisition
	/// \brief Acquires data and stores the data unprocessed.
    ///
	/// \fn void void getRawDataEx(OCTDeviceHandle Dev, RawDataHandle RawData, int CameraIdx);
	/// \ingroup Acquisition
	/// \brief Acquires data with the specific camera given with camera index and stores the data unprocessed.
	///
	/// \fn void stopMeasurement(OCTDeviceHandle);
	/// \ingroup Acquisition
	/// \brief stops the current measurement.
	/// \param Dev The #OCTDeviceHandle that was initially provided by initDevice.
    ///
	/// \fn void measureSpectra(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw)
	/// \ingroup Acquisition
	/// \brief Acquires N spectra of raw data without moving galvo scanners.
	///
	/// \fn void measureSpectraEx(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw, int CameraIdx)
	/// \ingroup Acquisition
	/// \brief Acquires N spectra of raw data without moving galvo scanners. Supports multiple cameras (e.g. PS-OCT).

	SPECTRALRADAR_API size_t projectMemoryRequirement(OCTDeviceHandle Handle, ScanPatternHandle Pattern, AcquisitionType type);
	SPECTRALRADAR_API void startMeasurement(OCTDeviceHandle Dev, ScanPatternHandle Pattern, AcquisitionType type);
	SPECTRALRADAR_API void getRawData(OCTDeviceHandle Dev, RawDataHandle RawData);
	SPECTRALRADAR_API void getRawDataEx(OCTDeviceHandle Dev, RawDataHandle RawData, int CameraIdx);
	SPECTRALRADAR_API void stopMeasurement(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void measureSpectra(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw);
	SPECTRALRADAR_API void measureSpectraEx(OCTDeviceHandle Dev, int N, RawDataHandle Raw, int CameraIndex);

	// PROCESSING

	/// \defgroup Processing Processing
	/// \brief Standard Processing Routines.
    ///
	/// \enum ProcessingType
	/// \ingroup Processing
	/// \brief defindes the algorithm used for dechirping the input signal and Fourier transformation
	typedef enum {
		/// FFT with no dehchirp algorithm applied.
		Processing_StandardFFT,
		/// Full matrix multiplication ("filter bank"). Mathematical precise dechirp, but rather slow.
		Processing_StandardNDFT,
		/// Linear interpolation prior to FFT.
		Processing_iFFT1,
		/// Linear interpolation with 2x oversampling prior to FFT.
		Processing_iFFT2,
		/// Linear interpolation with 3x oversampling prior to FFT.
		Processing_iFFT3,
		/// Linear interpolation with 4x oversampling prior to FFT.
		Processing_iFFT4,
		/// NFFT algorithm with parameter m=1.
		Processing_NFFT1,
		/// NFFT algorithm with parameter m=2.
		Processing_NFFT2,
		/// NFFT algorithm with parameter m=3.
		Processing_NFFT3,
		/// NFFT algorithm with parameter m=4.
		Processing_NFFT4,
	} ProcessingType;

	/// \enum ApodizationWindow
	/// \ingroup Processing
	/// \brief To select the apodization window function.
	typedef enum {
		/// Hann window function
		Apodization_Hann = 0,	 
		/// Hamming window function
		Apodization_Hamming = 1,
		/// Gaussian window function
		Apodization_Gauss = 2,	 
		/// Tapered cosine window function
		Apodization_TaperedCosine = 3,
		/// Blackman window function
		Apodization_Blackman = 4,
		/// 4-Term Blackman-Harris window function
		Apodization_BlackmanHarris = 5,
		/// The apodizatin function is determined, based on the shape of the light source at hand. \warning{This feature is still experimental.}
		Apodization_LightSourceBased = 6,
		/// Unknown apodization window
		Apodization_Unknown = 999
	} ApodizationWindow;

	/// \enum ProcessingParameterInt
	/// \ingroup Processing
	/// \brief Parameters that set the behavious of the processing algorithms.
	typedef enum {
		/// Identifyer for averaging of several subsequent spectra prior to Fourier transform.
		Processing_SpectrumAveraging,
		/// Identifyer for averaging the absolute values of several subsequent A-scan after Fourier transform.
		Processing_AScanAveraging,
		/// Averaging of subsequent B-scans.
		Processing_BScanAveraging,
		/// Identifier for zero padding prior to Fourier transformation.
		Processing_ZeroPadding,
		/// The maximum number of threads to used by processing. A value of 0 indicates automatic selection, equal to the number of cores in the host PC.
		Processing_NumberOfThreads,
		/// Averaging of fourier spectra.
		Processing_FourierAveraging
	} ProcessingParameterInt;

	/// \enum ProcessingParameterFloat
	/// \ingroup Processing
	/// \brief Parameters that set the behaviour of the processing algorithms.
	typedef enum {
		/// Sets how much influence newly acquired apodizations have compared to older ones.
		Processing_ApodizationDamping,
		/// Determines the minimum signal intensity on the edge channels of the spectra. \warning{Setting this value may seriously reduce performance of the system.}
		Processing_MinElectrons
	} ProcessingParameterFloat;

	/// \enum CalibrationData
	/// \ingroup Processing
	/// \brief Data describing the calibration of the processing routines. 
	typedef enum {
		/// Calibration vector used as offset.
		Calibration_OffsetErrors,
		/// Calibration data used as reference spectrum.
		Calibration_ApodizationSpectrum,
		/// Calibration data used as apodization multiplicators.
		Calibration_ApodizationVector,
		/// Calibration data used to compensate for dispersion.
		Calibration_Dispersion,
		/// Calibration data used for dechirping spectral data.
		Calibration_Chirp,
		/// Calibration data used as extended adjust.
		Calibration_ExtendedAdjust,
		/// Calibration data used as fixed scan pattern data.
		Calibration_FixedPattern
	} CalibrationData;

	/// \enum ProcessingFlag
	/// \ingroup Processing
	/// \brief Flags that set the behaviour of the processing algorithms.
	typedef enum {
		/// Flag identifying whether to apply offset error removal. This flag is activated by default.
		Processing_UseOffsetErrors, // 0
		/// Flag sets whether the DC spectrum as measured is to be removed from the spectral data. This flag is activated by default.
		Processing_RemoveDCSpectrum, // 1
		/// Flag sets whether the DC spectrum to be removed is rescaled by the respective spectrum intensity it is applied to. This flag is activated by default.
		Processing_RemoveAdvancedDCSpectrum, // 2
		/// Flag identifying whether to apply apodization. This flag is activated by default.
		Processing_UseApodization, // 3
		/// Flag to determine whether the acquired data is to be averaged in order to compute an apodization spectrum. This flag is deactivated by default.
		Processing_UseScanForApodization, // 4
		/// Flag to activate or deactivate a filter removing undersampled signals from the A-scan. This flag is deactivated by default.
		Processing_UseUndersamplingFilter, // 5
		/// Flag activating or deactivating dispersion compensation. This flag is deactivated by default.
		Processing_UseDispersionCompensation, // 6
		/// Flag identifying whether to apply dechirp. This flag is activated by default.
		Processing_UseDechirp, // 7
		/// Flag identifying whether to use extended adjust. This flag is deactivated by default.
		Processing_UseExtendedAdjust, // 8
		/// Flag identifying whether to use full range output. This flag is deactivated by default.
		Processing_FullRangeOutput, // 9
		/// Experimental: Flag for an experimental lateral DC filtering algorithm. This flag is deactivated by default
		Processing_FilterDC, // 10
		/// Flag activating or deactivating autocorrelation compensation. This flag is deactivated by default.
		Processing_UseAutocorrCompensation, // 11
		/// Exprtimental: Toggles dispersion encoded full range processing mode, eliminating folding of the signal at the top. This flag is deactivated by default.
		Processing_UseDEFR, // 12
		/// Flag deactivating deconvolution in apodization processing, using windowing only. This flag is deactivated by default.
		Processing_OnlyWindowing, // 13
		/// Flag for removal of fixed pattern noise, used for swept source OCT systems. This flag is deactivated by default.
		Processing_RemoveFixedPattern // 14
	} ProcessingFlag;

	/// \enum ProcessingAveragingAlgorithm
	/// \ingroup Processing
	/// \brief This sets the averaging algorithm to be used for processing. \warning{This features is still experimental and might contain bugs.}
	typedef enum {
		Processing_Averaging_Min,
		Processing_Averaging_Mean,
		Processing_Averaging_Median,
		Processing_Averaging_Norm2,
		Processing_Averaging_Max,
		Processing_Averaging_Fourier_Min,
		Processing_Averaging_Fourier_Norm4,
		Processing_Averaging_Fourier_Max,
		Processing_Averaging_StandardDeviationAbs
	} ProcessingAveragingAlgorithm;

	/// \enum ApodizationWindowParameter
	/// \ingroup Processing
	/// \brief Sets certain parameters that are used by the window functions to be applied during apodization. 
	typedef enum {
		/// Sets the width of a Gaussian apodization window
		ApodizationWindowParameter_Sigma,
		/// Sets the ratio of the constant to the cosine part when using a tapered cosine window.
		ApodizationWindowParameter_Ratio,
		/// Sets the corner frequency of the filter applied when using a light-source based apodization. \warning{Light source based apodization 
		/// is still experimental and might contatin bugs or decrease performance of the OCT system.}
		ApodizationWindowParameter_Frequency
	} ApodizationWindowParameter;

	/// \fn ProcessingHandle createProcessing(int SpectrumSize, int BytesPerRawPixel, float ScalingFactor, float MinElectrons, ProcessingType Type, float FFTOversampling);
	/// \ingroup Processing
	/// \brief Creates processing routines with the specified properties. 
    ///
	/// \fn ProcessingHandle createProcessingForDevice(OCTDeviceHandle Dev);
	/// \ingroup Processing
	/// \brief Creates suitable standard processing routines for the specified device (#OCTDeviceHandle).
    ///
	/// \fn ProcessingHandle createProcessingForDeviceEx(OCTDeviceHandle Dev, int CameraIndex)
	/// \ingroup Processing
	/// \brief Creates suitable standard processing routines for the specified device (#OCTDeviceHandle) with camera index.
	///
	/// \fn int getInputSize(ProcessingHandle);
	/// \ingroup Processing
	/// \brief Returns the expected input size (pixels per spectrum); of the processing algorithms. 
	/// 
	/// This function is provided for convenience as processing routines can be used independently of the device.
    ///
	/// \fn int getAScanSize(ProcessingHandle Proc);
	/// \ingroup Processing
	/// \brief gives the number of pixels in an A-Scan of the SpectralRadar device. This number is identical to the number of rows in a finished B-Scan.
	/// \param Proc Processing that is used to get the A-Scan.
	/// \return The number of pixels in an A-Scan of the SpectralRadar device.
    ///
	/// \fn void setApodizationWindow(ProcessingHandle, ApodizationWindow);
	/// \ingroup Processing
	/// \brief Sets the window function that is to be used for apodization. The selected function will be used in all subsequent processings.
	/// 
	/// If this function is not explicitly called a Hann window will be used.
	/// \param Handle Processing handle.
	/// \param Window The apodization window that is used for data processing. 
	///
	/// \fn int getApodizationWindow(ProcessingHandle);
	/// \ingroup Processing
	/// \brief Gets the window function that is being used for apodization.
	/// \param Proc handle to the #OCTDeviceHandle that was initially provided by initDevice.
	///
	/// \fn void setApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection, double Value);
	/// \ingroup Processing
	/// \brief Sets the apodization window parameter, such as window width or ratio between constant and cosine part.
	///
	/// \fn double getApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection);
	/// \ingroup Processing
	/// \brief Gets the apodization window parameter, such as window width or ratio between constant and cosine part.
    ///
	/// \fn void setDechirpAlgorithm(ProcessingHandle, ProcessingType)
	/// \ingroup Processing
	/// \brief Sets the algorithm that is to be sued for dechirping the input spectra.
    ///
	/// \fn setProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection, int Value);
	/// \ingroup Processing
	/// \brief Sets the specified integer value processing parameter.
    ///
	/// \fn int getProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection);
	/// \ingroup Processing
	/// \brief Returns the specified integer value processing parameter.
	/// 
	/// \fn  void setProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterInt Selection, double Value);
	/// \ingroup Processing
	/// \brief Sets the specified processing paramter. 
    ///
	/// \fn  double getProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection)
	/// \ingroup Processing
	/// \brief Gets the specified processing paramter. 
	///
	/// \fn void setProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag, BOOL Value);
	/// \ingroup Processing
	/// \brief Sets the specified processing flag.
    ///
	/// \fn BOOL getProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag);
	/// \ingroup Processing
	/// \brief Returns TRUE if the specified processing flag is set, FALSE otherwise.
	/// 
	/// \fn void setProcessingAveragingAlgorithm(ProcessingHandle Proc, ProcessingAveragingAlgorithm Algorithm);
	/// \ingroup Processing
	/// \brief Sets the algorithm that is used for averaing by the processing.
    ///
	/// \fn void setCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);
	/// \ingroup Processing
	/// \brief Sets the current active calibration data.
    ///
	/// \fn void getCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);
	/// \ingroup Processing
	/// \brief Returns the currently active calibration parameter.
    ///
	/// \fn void measureCalibration(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection);
	/// \ingroup Processing
	/// \brief Measures the specified calibration parameters and uses them in subsequent processing.
    ///
	/// \fn void void measureCalibrationEx(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection, int CameraIndex)
	/// \ingroup Processing
	/// \brief Measures the specified calibration parameters and uses them in subsequent processing with specified camera index.
	///
	/// \fn void measureSpectrum(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc, BOOL moveToApoPos)
	/// \ingroup Processing
	/// \brief Measures apodization spectrum and uses them in subsequent processing.
	///
	/// \fn void saveCalibrationAuto(ProcessingHandle Proc, CalibrationData Selection);
	/// \ingroup Processing
	/// \brief Saves the selected calibration in its default path.
	/// \warning This will override your default calibration of the device.
    ///
	/// \fn void saveCalibration(ProcessingHandle Proc, CalibrationData Selection, const char Path[]);
	/// \ingroup Processing
	/// \brief Saves the selected calibration in the specified path.
	/// \warning This will override your default calibration of the device if you specifiy the default path. 
	/// 
	/// \fn void loadCalibration(ProcessingHandle Proc, CalibrationData Selection, const char Path[]);
	/// \ingroup Processing
	/// \brief Will load a specified calibration file and use for subsequent processing. 
    ///
	/// \fn void setSpectrumOutput(ProcessingHandle Proc, DataHandle Spectrum);
	/// \ingroup Processing
	/// \brief Sets the location for the resulting spectral data.
    ///
	/// \fn void setOffsetCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle OffsetCorrectedSpectrum);
	/// \ingroup Processing
	/// \brief Sets the location for the resulting offset corrected spectral data.
    ///
	/// \fn void setDCCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle ProcessingCorrectedSpectrum)
	/// \ingroup Processing
	/// \brief Sets the location for the resulting DC removed spectral data.
	///
	/// \fn void setApodizedSpectrumOutput(ProcessingHandle Proc, DataHandle ApodizedSpectrum);
	/// \ingroup Processing
	/// \brief Sets the location for the resulting apodized spectral data.
    ///
	/// \fn void setComplexDataOutput(ProcessingHandle, ComplexDataHandle);
	/// \ingroup Processing
	/// \brief Sets the pointer the resulting complex B-Scan of the next processing is written to. 
	///
	/// If set to 0 no complex data result will be created in the next processing.
    ///
	/// \fn void setProcessedDataOutput(ProcessingHandle, DataHandle)
	/// \ingroup Processing
	/// \brief Sets the pointer the resulting B-Scan of the next processing is written to. 
	///
	/// If set to 0 no processed floating point data in dB will be created in the next processing.
    ///
	/// \fn void setHorMirroredDataOutput(ProcessingHandle, DataHandle)
	/// \ingroup Processing
	/// \brief Sets the pointer the resulting B-Scan of the next processing is written to. The result will be written mirrored at the horizontal axis.
	///
	/// If set to 0 no floating point processed data in dB will be created in the next processing.
    ///
	/// \fn void setColoredDataOutput(ProcessingHandle, ColoredDataHandle, Coloring32BitHandle)
	/// \ingroup Processing
	/// \brief Sets the pointer the resulting colored B-Scan of the next processing is written to. 
	///
	/// \fn void setTransposedColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle BScan, Coloring32BitHandle Color);
	/// \ingroup Processing
	/// \brief Sets the pointer the resulting colored B-Scan of the next processing is written to. The data will be transposed so that the first axis is the x-axis. 
    ///
	/// \fn void setComplexDataOutput(ProcessingHandle Proc, ComplexDataHandle ComplexBScan)
	/// \ingroup Processing
	/// \brief Sets the pointer the resulting complex B-Scan of the next processing is written to.
	///
	/// \fn void executeProcessing(ProcessingHandle, RawDataHandle)
	/// \ingroup Processing
	/// \brief Execute the processing.
	///
	/// The specified raw data will be transformed. Results will be written to data objects specified by #setProcessedDataOutput(), #setComplexDataOutput(), #setColoredDataOutput(), etc. 
    ///
	/// \fn void closeProcessing(ProcessingHandle Proc);
	/// \ingroup Processing
	/// \brief Closes the processing and frees all temporary memory that was associated with it. Processing threads will be stopped.
	///
	/// \fn void computeDispersion(DataHandle Spectrum1In, DataHandle Spectrum2In, DataHandle ChirpOut, DataHandle DispOut);
	/// \ingroup Processing
	/// \brief Computes the dispersion and chirp of the two provided spectra, where both spectra need to have been subjected 
	/// to same dispersion mismatch. Both spectra need to have been acquired for different path length differences.
	///
	/// \fn void computeDispersionByCoeff(double QuadraticIn, DataHandle ChirpIn, DataHandle DispOut);
	/// \ingroup Processing
	/// \brief Computes dispersion by a quadratic approximation specified by the quadratic factor.
	///
	/// \fn void computeDispersionByImage(DataHandle LinearKSpectraIn, DataHandle ChirpIn, DataHandle DispOut);
	/// \ingroup Processing
	/// \brief Guesses the dispersion based on the raw data specified. The raw data needs to be linearized in k before applying to this function.
	///
	/// \fn int getNumberOfDispersionPresets(ProcessingHandle Proc)
	/// \ingroup Processing
	/// \brief Gets the number of dispersion presets.
	///
	/// \fn const char* getDispersionPresetName(ProcessingHandle Proc, int Index)
	/// \ingroup Processing
	/// \brief Gets the name of the dispersion preset specified with index.
	/// 
	/// \fn void setDispersionPresetByName(ProcessingHandle Proc, const char* Name)
	/// \ingroup Processing
	/// \brief Sets the dispersion preset specified with name.
	/// 
	/// \fn void setDispersionPresetByIndex(ProcessingHandle Proc, int Index)
	/// \ingroup Processing
	/// \brief Sets the dispersion preset specified with index.
	/// 	
	/// \fn void setDispersionPresets(ProcessingHandle Proc, ProbeHandle Probe)
	/// \ingroup Processing
	/// \brief Sets the dispersion presets for the probe.
	/// 

	SPECTRALRADAR_API ProcessingHandle createProcessing(int SpectrumSize, int BytesPerRawPixel, BOOL Signed, float ScalingFactor, float MinElectrons, ProcessingType Type, float FFTOversampling); 
	SPECTRALRADAR_API ProcessingHandle createProcessingForDevice(OCTDeviceHandle Dev);
	SPECTRALRADAR_API ProcessingHandle createProcessingForDeviceEx(OCTDeviceHandle Dev, int CameraIndex);
	SPECTRALRADAR_API int getInputSize(ProcessingHandle Proc);
	SPECTRALRADAR_API int getAScanSize(ProcessingHandle Handle);
	SPECTRALRADAR_API int getApodizationWindow(ProcessingHandle Proc);
	SPECTRALRADAR_API void setApodizationWindow(ProcessingHandle Proc, ApodizationWindow Window);
	SPECTRALRADAR_API void setApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection, double Value);
	SPECTRALRADAR_API double getApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection);
	SPECTRALRADAR_API void setDechirpAlgorithm(ProcessingHandle Proc, ProcessingType Type);
	SPECTRALRADAR_API void setProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection, int Value);
	SPECTRALRADAR_API int getProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection);
	SPECTRALRADAR_API void setProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection, double Value);
	SPECTRALRADAR_API double getProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection);
	SPECTRALRADAR_API void setProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag, BOOL Value);
	SPECTRALRADAR_API BOOL getProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag);
	SPECTRALRADAR_API void setProcessingAveragingAlgorithm(ProcessingHandle Proc, ProcessingAveragingAlgorithm Algorithm);
	SPECTRALRADAR_API void setCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);
	SPECTRALRADAR_API void getCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);
	SPECTRALRADAR_API void measureCalibration(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection);
	SPECTRALRADAR_API void measureCalibrationEx(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection, int CameraIndex);
	SPECTRALRADAR_API void measureSpectrum(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc, BOOL moveToApoPos);
	SPECTRALRADAR_API void saveCalibrationAuto(ProcessingHandle Proc, CalibrationData Selection);
	SPECTRALRADAR_API void saveCalibration(ProcessingHandle Proc, CalibrationData Selection, const char Path[]);
	SPECTRALRADAR_API void loadCalibration(ProcessingHandle Proc, CalibrationData Selection, const char Path[]);
	SPECTRALRADAR_API void setSpectrumOutput(ProcessingHandle Proc, DataHandle Spectrum);
	SPECTRALRADAR_API void setOffsetCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle OffsetCorrectedSpectrum);
	SPECTRALRADAR_API void setDCCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle ProcessingCorrectedSpectrum);
	SPECTRALRADAR_API void setApodizedSpectrumOutput(ProcessingHandle Proc, DataHandle ApodizedSpectrum);
	SPECTRALRADAR_API void setProcessedDataOutput(ProcessingHandle Proc, DataHandle Scan);
	SPECTRALRADAR_API void setHorMirroredDataOutput(ProcessingHandle Proc, DataHandle Scan);
	SPECTRALRADAR_API void setColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle BScan, Coloring32BitHandle Color);
	SPECTRALRADAR_API void setTransposedColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle BScan, Coloring32BitHandle Color);
	SPECTRALRADAR_API void setComplexDataOutput(ProcessingHandle Proc, ComplexDataHandle ComplexBScan);
	SPECTRALRADAR_API void executeProcessing(ProcessingHandle Proc, RawDataHandle RawData);
	SPECTRALRADAR_API void closeProcessing(ProcessingHandle Proc);

	SPECTRALRADAR_API void computeDispersion(DataHandle Spectrum1In, DataHandle Spectrum2In, DataHandle ChirpOut, DataHandle DispOut);
	SPECTRALRADAR_API void computeDispersionByCoeff(double QuadraticIn, DataHandle ChirpIn, DataHandle DispOut);
	SPECTRALRADAR_API void computeDispersionByImage(DataHandle LinearKSpectraIn, DataHandle ChirpIn, DataHandle DispOut);

	SPECTRALRADAR_API int getNumberOfDispersionPresets(ProcessingHandle Proc);
	SPECTRALRADAR_API const char* getDispersionPresetName(ProcessingHandle Proc, int Index);
	SPECTRALRADAR_API void setDispersionPresetByName(ProcessingHandle Proc, const char* Name);
	SPECTRALRADAR_API void setDispersionPresetByIndex(ProcessingHandle Proc, int Index);
	SPECTRALRADAR_API void setDispersionPresets(ProcessingHandle Proc, ProbeHandle Probe);

	// EXPORT 
	
	/// \defgroup Export Export and Import
	/// \brief Export functionality to store data to disk and load it from there. 
    ///
	/// \enum Data1DExportFormat
	/// \ingroup Export
	/// \brief Export format for 1-dimensional data (#DataHandle). 
	typedef enum {
		/// RAW data format containing the data of the object as binary, single precision floating point values, little endian.
		Data1DExport_RAW,
		/// TXT is a text file having all values stored space seperated and human readable.
		Data1DExport_TXT,
		/// CSV (Comma Seperated Values) is a text file having all values stored, comma seperated and human readable.
		Data1DExport_CSV,
		/// TableTXT is a human readable text-file in a table like format, having the physical 1- and 2-axis as first two columns and the data value as third.
		Data1DExport_TableTXT,
		/// FITS Data format
		Data1DExport_Fits
	} Data1DExportFormat;

	/// \enum Data2DExportFormat
	/// \ingroup Export
	/// \brief Export format for 2-dimensional data (#DataHandle). 
	typedef enum {
		/// Spectral Radar Metaformat, containing no data but all additinal parameters, such as spacing, size, etc.
		Data2DExport_SRM,
		/// RAW data format containing the data of the object as binary, single precision floating point values, little endian.
		Data2DExport_RAW,
		/// TXT is a text file having all values stored space seperated and human readable.
		Data2DExport_TXT,
		/// CSV (Comma Seperated Values) is a text file having all values stored, comma seperated and human readable.
		Data2DExport_CSV,
		/// TableTXT is a human readable text-file in a table like format, having the physical 1- and 2-axis as first two columns and the data value as third.
		Data2DExport_TableTXT,
		/// FITS Data format
		Data2DExport_Fits
	} Data2DExportFormat;

	/// \enum Data3DExportFormat
	/// \ingroup Export
	/// \brief Export format for 3-dimensional data (#DataHandle).
	typedef enum {
		/// Spectral Radar Metaformat, containing no data but all additinal parameters, such as spacing, size, etc.
		Data3DExport_SRM,
		/// RAW data format containing the data of the object as binary, single precision floating point values, little endian.
		Data3DExport_RAW,
		/// TXT is a text file having all values stored space seperated and human readable.
		Data3DExport_TXT,
		/// CSV (Comma Separated Values) is a text file having all values stored, comma seperated and human readable.
		Data3DExport_CSV,
		/// VFF data format.
		// TODO: More text.
		Data3DExport_VFF,
		/// VTK data format.
		// TODO: More text.
		Data3DExport_VTK,
		/// FITS Data format
		Data3DExport_Fits,
		/// TIFF Data format
		Data3DExport_TIFF
	} Data3DExportFormat;


	/// \enum ComplexDataExportFormat
	/// \ingroup Export
	/// \brief Export format for complex data
	typedef enum {
		/// RAW data format containg binary data.
		ComplexDataExport_RAW
	} ComplexDataExportFormat;

	/// \enum ColoredDataExportFormat
	/// \ingroup Export
	/// \brief Export format for images (#ColoredDataHandle).
	typedef enum {
		/// Spectral Radar Metaformat, containing no data but all additinal parameters, such as spacing, size, etc.
		ColoredDataExport_SRM,
		/// RAW data format containing the data of the object as binary, 32-bit unsigned integer values, little endian. 
		/// The concrete format of the data depends on the colored data object (#ColoredDataHandle). In most cases it will be RGB32 or RGBA32.
		ColoredDataExport_RAW,
		/// BMP - Bitmap image format.
		ColoredDataExport_BMP,
		/// PNG image format.
		ColoredDataExport_PNG, 
		/// JPG/JPEG image format.
		ColoredDataExport_JPG,
		/// PDF image format.
		ColoredDataExport_PDF,
		/// TIFF image format
		ColoredDataExport_TIFF
	} ColoredDataExportFormat;

	/// \enum Direction
	/// \ingroup Volume 
	/// \brief Specifies a direction
	typedef enum {
		/// The 1-axis direction.
		Direction_1,
		/// The 2-axis direction.
		Direction_2,
		/// The 3-axis direction.
		Direction_3
	} Direction;

	/// \enum DataImportFormat
	/// \ingroup Export
	/// \brief Supported import format to load data from disk.
	typedef enum {
		/// Spectral Radar Metaformat, containing no data but all additinal parameters, such as spacing, size, etc. 
		/// It is searched for an appropriate file with same name but different extension containg the according data.
		DataImport_SRM
	} DataImportFormat;

	/// \enum RawDataExportFormat
	/// \ingroup Export
	/// \brief Supported raw data export formats to store data to disk.
	typedef enum {
		/// Single precision floating point raw data
		RawDataExport_RAW,
		/// Spectral Radar raw data format, specified additional information such as apodization scans, scan range, etc.
		RawDataExport_SRR
	} RawDataExportFormat;

	/// \enum RawDataImportFormat
	/// \ingroup Export
	/// \brief Supported raw data import formats to load data from disk.
	typedef enum {
		/// Spectral Radar raw data format, specified additional information such as apodization scans, scan range, etc.
		RawDataImport_SRR
	} RawDataImportFormat;

	/// \fn void exportData1D(DataHandle Data, Data1DExportFormat Format, const char* Path);
	/// \ingroup Export
	/// \brief Exports 1-dimensional data (#DataHandle). 
	///
	/// \fn void exportData2D(DataHandle, Data2DExportFormat, const char*);
	/// \ingroup Export
	/// \brief Exports 2-dimensional data (#DataHandle). 
	///
	/// \fn void exportData3D(DataHandle, Data3DExportFormat, const char*);
	/// \ingroup Export
	/// \brief Exports 3-dimensional data (#DataHandle). 
	///
	/// \fn void exportData2DAsImage(DataHandle, Coloring32BitHandle, ColoredDataExportFormat format, const char*);
	/// \ingroup Export
	/// \brief Exports 2-dimensional data (#DataHandle) as image data (such as BMP, PNG, JPEG, ...). 
	///
	/// \fn void exportData3DAsImage(DataHandle Data, Coloring32BitHandle Color, ColoredDataExportFormat format, Direction SliceNormalDirection,´const char* fileName, 
	/// BOOL drawScale, BOOL drawMarkers, BOOL physicalAspectRatio)
	/// \ingroup Export
	/// \brief Exports 3-dimensional data (#DataHandle) as image data (such as BMP, PNG, JPEG, ...). 
	///
	/// \fn void exportComplexData(ComplexDataHandle, ComplexDataExportFormat, const char*)
	/// \ingroup Export
	/// \brief Exports 1-, 2- and 3-dimensional complex data (#ComplexDataHandle)
	/// 
	/// \fn void exportColoredData(ColoredDataHandle, ColoredDataExportFormat, const char*);
	/// \ingroup Export
	/// \brief Exports colored data (#ColoredDataHandle)
	///
	/// \fn void importColoredData(ColoredDataHandle ColoredData, DataImportFormat Format, const char* Path)
	/// \ingroup Export
	/// \brief Imports colored data (#ColoredDataHandle) with the specified format and copied it into a data object (#ColoredDataHandle)
	/// 
	/// \fn void importData(DataHandle Data, DataImportFormat Format, const char* Path);
	/// \ingroup Export
	/// \brief Imports data with the specified format and copies it into a dat data object (#DataHandle).
	///
	/// \fn  void exportRawData(RawDataHandle Raw, RawDataExportFormat Format, const char* Path);
	/// \ingroup Export
	/// \brief Exports the specified data to disk. 
	///
	/// \fn  void importRawData(RawDataHandle Raw, RawDataImportFormat Format, const char* Path);
	/// \ingroup Export
	/// \brief Imports the specified data from disk.

	SPECTRALRADAR_API void exportData1D(DataHandle Data, Data1DExportFormat Format, const char* Path);
	SPECTRALRADAR_API void exportData2D(DataHandle Data, Data2DExportFormat Format, const char* Path);
	SPECTRALRADAR_API void exportData3D(DataHandle Volume, Data3DExportFormat Format, const char* Path);
	SPECTRALRADAR_API void exportData2DAsImage(DataHandle Data, Coloring32BitHandle Color, ColoredDataExportFormat format,
		const char* fileName, BOOL drawScale, BOOL drawMarkers, BOOL physicalAspectRatio);
	SPECTRALRADAR_API void exportData3DAsImage(DataHandle Data, Coloring32BitHandle Color, ColoredDataExportFormat format, Direction SliceNormalDirection,
		const char* fileName, BOOL drawScale, BOOL drawMarkers, BOOL physicalAspectRatio);
	SPECTRALRADAR_API void importData(DataHandle Data, DataImportFormat Format, const char* Path);

	SPECTRALRADAR_API void exportComplexData(ComplexDataHandle, ComplexDataExportFormat, const char*);

	SPECTRALRADAR_API void exportColoredData(ColoredDataHandle Image, ColoredDataExportFormat Format, const char* fileName);
	SPECTRALRADAR_API void importColoredData(ColoredDataHandle ColoredData, DataImportFormat Format, const char* Path);

	SPECTRALRADAR_API void exportRawData(RawDataHandle Raw, RawDataExportFormat Format, const char* Path);
	SPECTRALRADAR_API void importRawData(RawDataHandle Raw, RawDataImportFormat Format, const char* Path);

	// STACK PATTERN VOLUME

	/// \defgroup Volume Volume
	/// \brief Functionality to store and access volume data.
    ///
	/// \enum Plane2D
	/// \ingroup Volume
	/// \brief Planes for slices of the volume data.
	typedef enum {
		/// The 12 (XZ) plane, orthogonal to the 3 (Y) axis
		Plane2D_12,
		/// The 23 (XY) plane, orthogonal to the 3 (Z) axis
		Plane2D_23,
		/// The 13 (ZY) plane, orthogonal to the 2 (X) axis
		Plane2D_13
	} Plane2D;

	
	/// \fn void appendRawData(RawDataHandle Data, RawDataHandle NewData, Direction direction)
	/// \ingroup Volume
	/// \brief Appends the new raw data to the old raw data in the specified direction.
	///
	/// \fn void getRawDataSliceIndex(RawDataHandle Data, RawDataHandle Slice, Direction SliceNormalDirection, int Index)
	/// \ingroup Volume
	/// \brief Returns a slice of raw data in the specified direction at the specified index.
	///
    SPECTRALRADAR_API void appendRawData(RawDataHandle Data, RawDataHandle DataToAppend, Direction direction);
	SPECTRALRADAR_API void getRawDataSliceIndex(RawDataHandle Data, RawDataHandle Slice, Direction SliceNormalDirection, int Index);

	// FLOATING POINT DATA ANALYSIS AND MANIPULATION
	/// \fn double analyzeData(DataHandle Data, DataAnalyzation Selection);
	/// \ingroup Volume
	/// \brief Performs the selected analyzation of the specified data and returns the resulting value.
	///
	/// \fn double analyzeAScan(DataHandle Data, AScanAnalyzation Selection);
	/// \ingroup Volume
	/// \brief Performs the selected analyzation of the specified A-scan and returns the resulting value.

	/// This functions assumes that the data contains an A-scan and performs A-scan specific analysis on it.
	///
	/// \fn void determineDynamicRange(DataHandle Data, float* MinRange_dB, float* MaxRange_dB);
	/// \ingroup Volume
	/// \brief Gives a rough estimation of the dynamic range of the specified data object.
	///
	/// \fn void void transpose(DataHandle DataIn, DataHandle DataOut)
	/// \ingroup Volume
	/// \brief Transposes the given data and writes the result to DataOut.
	///
	/// \fn void transposeInplace(DataHandle Data)
	/// \ingroup Volume
	/// \brief Transposes the given Data.
	///
	/// \fn void transposeAndScaleData(DataHandle DataIn, DataHandle DataOut, float Min, float Max)
	/// \ingroup Volume
	/// \brief Transposes the given data and scales it to the range [Min, Max].
	///
	/// \fn void normalizeData(DataHandle DataIn, float Min, float Max)
	/// \ingroup Volume
	/// \brief Scales the given data to the range [Min, Max].
	///
	/// \fn void lockData(DataHandle Data)
	/// \ingroup Volume
	/// \brief Locks the given data.
	///
	/// \fn void unlockData(DataHandle Data)
	/// \ingroup Volume
	/// \brief Unlocks the given data.
	///
	/// \fn void getDataSlicePos(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, double Pos)
	/// \ingroup Volume
	/// \brief Returns a slice of data in the specified direction at the specified position.
	///
	/// \fn void getDataSliceIndex(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, int Index)
	/// \ingroup Volume
	/// \brief Returns a slice of data in the specified direction at the specified index.
	/// 
	/// \fn void getDataSliceAnalyzed(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, DataAnalyzation Selection);
	/// \ingroup Volume
	/// \brief Returns a slice of data that has been computed of all slice using the specified analyzation method. 
	///
	/// \fn void appendData(DataHandle Data, DataHandle NewData, Direction direction)
	/// \ingroup Volume
	/// \brief Appends the new data to the old data in the specified direction.
	///
	/// \fn void cropData(DataHandle Data, Direction direction, int index_max)
	/// \ingroup Volume
	/// \brief Crops the data at the specific direction at the given index. The result will contain the data with range [0, index] at the cropping dirction.
	///
	/// \fn void cropDataEx(DataHandle Data, Direction direction, int IndexMax, int IndexMin)
	/// \ingroup Volume
	/// \brief Crops the data at the specific direction at the given indeces. The result will contain the data with range [IndexMin, IndexMax] at the cropping dirction.
	///	
	/// \fn void separateData(DataHandle Data1, DataHandle Data2, int SeparationIndex, Direction Dir);
	/// \ingroup Volume
	/// \brief Separates the data at the given index at specific separation direction. The first part of the separated data will be in Data1, the second separated in Data2.
	///	
	/// \fn void void flipData(DataHandle Data, Direction FlippingDirection);
	/// \ingroup Volume
	/// \brief Flips the data around the specific direction.
	///
	SPECTRALRADAR_API double analyzeData(DataHandle Data, DataAnalyzation Selection);
	SPECTRALRADAR_API double analyzeAScan(DataHandle Data, AScanAnalyzation Selection);
	SPECTRALRADAR_API void determineDynamicRange(DataHandle Data, float* MinRange_dB, float* MaxRange_dB);
	SPECTRALRADAR_API void transpose(DataHandle DataIn, DataHandle DataOut);
	SPECTRALRADAR_API void transposeInplace(DataHandle Data);
	SPECTRALRADAR_API void transposeAndScaleData(DataHandle DataIn, DataHandle DataOut, float Min, float Max);
	SPECTRALRADAR_API void normalizeData(DataHandle Data, float Min, float Max);
	SPECTRALRADAR_API void lockData(DataHandle Data);
	SPECTRALRADAR_API void unlockData(DataHandle Data);

	SPECTRALRADAR_API void getDataSlicePos(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, double Pos);
	SPECTRALRADAR_API void getDataSliceIndex(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, int Index);
	SPECTRALRADAR_API void getDataSliceAnalyzed(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, DataAnalyzation Selection);
	SPECTRALRADAR_API void appendData(DataHandle Data, DataHandle NewData, Direction direction);
	SPECTRALRADAR_API void cropData(DataHandle Data, Direction direction, int Index);
	SPECTRALRADAR_API void cropDataEx(DataHandle Data, Direction direction, int IndexMax, int IndexMin);
	SPECTRALRADAR_API void separateData(DataHandle Data1, DataHandle Data2, int SeparationIndex, Direction Dir);
	SPECTRALRADAR_API void flipData(DataHandle Data, Direction FlippingDirection);

	/// \fn void getComplexDataSlicePos(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, double Pos);
	/// \ingroup Volume
	/// \brief Returns a slice of data in the specified direction at the specified position.
	///
	/// \fn void getComplexDataSliceIndex(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, int Index);
	/// \ingroup Volume
	/// \brief Returns a slice of data in the specified direction at the specified index.
	///
	/// \fn void appendComplexData(ComplexDataHandle Data, ComplexDataHandle DataToAppend, Direction direction);
	/// \ingroup Volume
	/// \brief Appends the new data to the old data in the specified direction.
	///
	/// \fn void cropComplexData(ComplexDataHandle Data, Direction CroppingDirection, int IndexMax, int IndexMin)
	/// \ingroup Volume
	/// \brief Crops the complex data at the specific direction at the given indeces. The result will contain the data with range [IndexMin, IndexMax] at the cropping dirction.
	///	
	SPECTRALRADAR_API void appendComplexData(ComplexDataHandle Data, ComplexDataHandle DataToAppend, Direction direction);
	SPECTRALRADAR_API void getComplexDataSlicePos(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, double Pos);
	SPECTRALRADAR_API void getComplexDataSliceIndex(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, int Index);
	SPECTRALRADAR_API void cropComplexData(ComplexDataHandle Data, Direction CroppingDirection, int IndexMax, int IndexMin);

	/// \fn void cropColoredData(ColoredDataHandle Data, Direction CroppingDirection, int IndexMax, int IndexMin);
	/// \ingroup Volume
	/// \brief Crops the colored data at the specific direction at the given indeces. The result will contain the data with range [IndexMin, IndexMax] at the cropping dirction.
	///	
	/// \fn void appendColoredData(ColoredDataHandle Data, ColoredDataHandle DataToAppend, Direction AppendingDirection)
	/// \ingroup Volume
	/// \brief Appends the new colored data to the old colored data in the specified direction.
	///	
	/// \fn void getColoredDataSlicePos(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, double Pos)
	/// \ingroup Volume
	/// \brief Get a slice of the colored data with specific slicing direction at given index position.
	///	
	/// \fn void getColoredDataSliceIndex(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, int Index)
	/// \ingroup Volume
	/// \brief Get a slice of the colored data with specific slicing direction at given index.
	SPECTRALRADAR_API void appendColoredData(ColoredDataHandle Data, ColoredDataHandle DataToAppend, Direction AppendingDirection);
	SPECTRALRADAR_API void cropColoredData(ColoredDataHandle Data, Direction CroppingDirection, int IndexMax, int IndexMin);
	SPECTRALRADAR_API void getColoredDataSlicePos(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, double Pos);
	SPECTRALRADAR_API void getColoredDataSliceIndex(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, int Index);

	/// \fn ImageFieldHandle createImageField(void);
	/// \ingroup Volume
	/// \brief Creates an object holding image field data.
    ///
	/// \fn void clearImageField(ImageFieldHandle ImageField);
	/// \ingroup Volume
	/// \brief Frees an object holding image field data.
    ///
	/// \fn void saveImageField(ImageFieldHandle ImageField, const char* Path);
	/// \ingroup Volume
	/// \brief Saves data containing image field data.
    ///
	/// \fn void loadImageField(ImageFieldHandle ImageField, const char* Path);
	/// \ingroup Volume
	/// \brief Loads data containing image field data.
    ///
	/// \fn void determineImageField(ImageFieldHandle ImageField, DataHandle Surface);
	/// \ingroup Volume
	/// \brief Determines the image field correction of the surface.
    ///
	/// \fn void determineImageFieldForProbe(ProbeHandle Probe, DataHandle Surface)
	/// \ingroup Volume
	/// \brief Determines the image field correction of the surface for the specified probe handle.
	///
	/// \fn void determineImageFieldForProbeWithMap(ProbeHandle Probe, DataHandle Surface, DataHandle Map);
	/// \ingroup Volume
	/// \brief Determines the image field correction of the surface for the specified probe handle using the given map. 
	/// Values != 0 in the map specifies to use the data in the surface handle otherwise thex will be interpolated. 
	///
	/// \fn void correctImageField(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Data)
	/// \ingroup Volume
	/// \brief Applies the image field correction to the B-Scan or volume data .
	///
	/// \fn void correctSurface(ImageFieldHandle ImageField, DataHandle Surface);
	/// \ingroup Volume
	/// \brief Applies the image field correction to the given Surface.
	///
	/// \fn void correctImageFieldFromProbe(ProbeHandle Probe, ScanPatternHandle Pattern, DataHandle Data);
	/// \ingroup Volume
	/// \brief Applies the image field correction saved in the probe handle to the B-Scan or volume data .
	///
	SPECTRALRADAR_API ImageFieldHandle createImageField(void);
	SPECTRALRADAR_API void clearImageField(ImageFieldHandle ImageField);
	SPECTRALRADAR_API void saveImageField(ImageFieldHandle ImageField, const char* Path);
	SPECTRALRADAR_API void loadImageField(ImageFieldHandle ImageField, const char* Path);
	SPECTRALRADAR_API void determineImageField(ImageFieldHandle ImageField, DataHandle Surface);
	SPECTRALRADAR_API void determineImageFieldForProbe(ProbeHandle Probe, DataHandle Surface);
	SPECTRALRADAR_API void determineImageFieldForProbeWithMap(ProbeHandle Probe, DataHandle Surface, DataHandle Map);
	SPECTRALRADAR_API void correctImageField(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Data);
	SPECTRALRADAR_API void correctImageFieldFromProbe(ProbeHandle Probe, ScanPatternHandle Pattern, DataHandle Data);
	SPECTRALRADAR_API void correctSurface(ImageFieldHandle ImageField, DataHandle Surface);

	// PROBE CALIBRATION

	/// \defgroup ProbeCalibration ProbeCalibration
	/// \brief Functionality to perform the probe calibration. Please use the ThorImageOCT software to perform the probe calibration if necessary. 
	///
	// The galvo offset/factor / Draw & Scan overlay calibration uses a triangle dot pattern with a fixed edge length which must
	// be aligned parallel to the video image egdes
	// CornerPos specifies the position of the right-angle corner of the triangle.
	typedef enum {
		NotRecognized,
		BottomLeft,
		BottomRight,
		TopRight,
		TopLeft
	} CornerPos;

	SPECTRALRADAR_API VisualCalibrationHandle createVisualCalibration(OCTDeviceHandle Device, double TargetCornerLength_mm, BOOL CheckAngle, BOOL SaveData);
	SPECTRALRADAR_API void clearVisualCalibration(VisualCalibrationHandle Handle);
	SPECTRALRADAR_API BOOL visualCalibrate_1st_CameraScaling(VisualCalibrationHandle Handle, ProbeHandle Probe, ColoredDataHandle Image);
	SPECTRALRADAR_API BOOL visualCalibrate_2nd_Galvo(VisualCalibrationHandle Handle, ProbeHandle Probe, ColoredDataHandle Image);
	SPECTRALRADAR_API BOOL visualCalibrate_previewImage(VisualCalibrationHandle Handle, ColoredDataHandle Image);
	SPECTRALRADAR_API void visualCalibration_getHoles(VisualCalibrationHandle Handle, int* x0, int* y0, int* x1, int* y1, int* x2, int* y2);
	SPECTRALRADAR_API const char* visualCalibrate_Status(VisualCalibrationHandle Handle);
	/* SPECTRALRADAR_API void getVisualCalibrationImage(VisualCalibrationHandle Handle, ColoredDataHandle Image);
	SPECTRALRADAR_API int determineCalibrationTriangle(ColoredDataHandle Image, VisualCalibrationHandle VisCal, float EdgeLength_mm, int threshold);
	SPECTRALRADAR_API BOOL doVisualCalibration(OCTDeviceHandle Dev, VisualCalibrationHandle VisCal, ColoredDataHandle Image);
	SPECTRALRADAR_API void drawCentricityGuide(VisualCalibrationHandle VisCal, ColoredDataHandle Image);
	SPECTRALRADAR_API float getVisualCalibrationValueFloat(VisualCalibrationHandle handle, ProbeParameterFloat value); */
	SPECTRALRADAR_API BOOL visualCalibrate_CameraCenter(VisualCalibrationHandle Handle, OCTDeviceHandle Device, ColoredDataHandle Image);
	SPECTRALRADAR_API BOOL visualCalibrate_getVideoCameraCenterImage(VisualCalibrationHandle Handle, OCTDeviceHandle Device, ColoredDataHandle Image);


	// DOPPLER

	/// \defgroup Doppler Doppler 
	/// \brief Doppler Processing Routines.
    ///
	/// \enum DopplerPropertyInt
	/// \ingroup Doppler
	/// \brief Values that determine the behaviour of the Doppler processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		DopplerAveraging_1,
		/// Averaging along the first axis, usually the first transversal axis (x)
		DopplerAveraging_2,
		/// Step size for calculating the doppler processing in the longitudinal axis (z). Stride needs to be smaller or equal to DopplerAveraging_1 and larger or equal to 1.
		DopplerStride_1,
		/// Step size for calculating the doppler processing in the transversal axis (x). Stride needs to be smaller or equal to DopplerAveraging_2 and larger or equal to 1.
		DopplerStride_2
	} DopplerPropertyInt;

	/// \enum DopplerPropertyFloat
	/// \ingroup Doppler
	/// \brief Values that determine the behaviour of the Doppler processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		DopplerRefractiveIndex,
	} DopplerPropertyFloat;

	/// \enum DopplerFlag
	/// \ingroup Doppler
	/// \brief Flats that determine the behaviour of the Doppler processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		DopplerVelocityScaling,
	} DopplerFlag;


	/// \fn DopplerProcessingHandle createDopplerProcessing(void);
	/// \ingroup Doppler
	/// \return #DopplerProcessingHandle to the created Doppler routines.
    ///
	/// \fn void createDopplerProcessingForProcessing(DopplerProcessingHandle*, ProcessingHandle);
	/// \ingroup Doppler
	/// \param Doppler Handle to the Doppler processing.
	/// \param Proc Handle to the Processing routines whose results are used as input for Doppler routines.
    ///
	/// \fn void setDopplerPropertyInt(DopplerProcessingHandle, DopplerPropertyInt, int Value)
	/// \ingroup Doppler
	/// \brief Sets Doppler processing properties.
	///
	/// \fn void setDopplerPropertyFloat(DopplerProcessingHandle Doppler, DopplerPropertyFloat Property, float Value);
	/// \ingroup Doppler
	/// \brief Sets Doppler processing properties.
	/// 
	/// \fn void setDopplerFlag(DopplerProcessingHandle Doppler, DopplerFlag Flag, BOOL OnOff);
	/// \ingroup Doppler
	/// \brief Sets the Doppler processing flags. 
    ///
	/// \fn void setDopplerAmplitudeOutput(DopplerProcessingHandle Doppler, DataHandle AmpOut)
	/// \ingroup Doppler
	/// \brief Sets the location of the resulting doppler amplitude output.
    ///
	/// \fn void setDopplerPhaseOutput(DopplerProcessingHandle Doppler, DataHandle PhasesOut)
	/// \ingroup Doppler
	/// \brief Sets the location of the resulting doppler phase output.
    ///
	/// \fn void executeDopplerProcessing(DopplerProcessingHandle, ComplexDataHandle)
	/// \ingroup Doppler
	/// \brief Executes the Doppler processing of the input data and returns phases and amplitudes.
    ///
	/// \fn void closeDopplerProcessing(DopplerProcessingHandle)
	/// \ingroup Doppler
	/// \brief Closes the Doppler processing routines and frees the memory that has been allocated for these to work properly.
	/// 
	/// \fn void getDopplerOutputSize(DopplerProcessingHandle Doppler, int Size1In, int Size2In, int* Size1Out, int* Size2Out);
	/// \inroup Doppler
	/// \brief Returns the final size of the Doppler output if executeDopplerProcessing is executed using data of the specified input size.

	SPECTRALRADAR_API DopplerProcessingHandle createDopplerProcessing(void);
	SPECTRALRADAR_API void createDopplerProcessingForProcessing(DopplerProcessingHandle* Doppler, ProcessingHandle Proc);
	SPECTRALRADAR_API void closeDopplerProcessing(DopplerProcessingHandle Doppler);
	SPECTRALRADAR_API void setDopplerPropertyInt(DopplerProcessingHandle Doppler, DopplerPropertyInt Property, int Value);
	SPECTRALRADAR_API void setDopplerPropertyFloat(DopplerProcessingHandle Doppler, DopplerPropertyFloat Property, float Value);
	SPECTRALRADAR_API void setDopplerFlag(DopplerProcessingHandle Doppler, DopplerFlag Flag, BOOL OnOff);
	SPECTRALRADAR_API void setDopplerAmplitudeOutput(DopplerProcessingHandle Doppler, DataHandle AmpOut);
	SPECTRALRADAR_API void setDopplerPhaseOutput(DopplerProcessingHandle Doppler, DataHandle PhasesOut);
	SPECTRALRADAR_API void getDopplerOutputSize(DopplerProcessingHandle Doppler, int Size1In, int Size2In, int* Size1Out, int* Size2Out);
	SPECTRALRADAR_API void executeDopplerProcessing(DopplerProcessingHandle Doppler, ComplexDataHandle Input);
	

	// SERVICE

	/// \defgroup Service Service
	/// \brief Service functions for additional analyzing of OCT functionality.
    ///
	/// \fn void calcContrast(DataHandle ApodizedSpectrum, DataHandle Contrast);
	/// \ingroup Service
	/// \brief Computes the contrast for the specified (apodized); spectrum.

	SPECTRALRADAR_API void calcContrast(DataHandle ApodizedSpectrum, DataHandle Contrast);

	// DIRECT INI FILE ACCESS

	/// \defgroup Settings Settings
	/// \brief Direct access to INI files and settings.
    ///
	/// \typedef SettingsHandle
	/// \ingroup Settings
	/// \brief Handle for saving settings on disk.
	struct C_Settings;
	typedef struct C_Settings* SettingsHandle;

	/// \fn SettingsHandle loadSettingsFile(const char*);
	/// \ingroup Settings
	/// \brief Loads a settings file (usually *.ini); and prepares its properties to be read.
    ///
	/// \fn int getSettingsEntryInt(SettingsHandle, const char*, int);
	/// \ingroup Settings
	/// \brief Gets an integer number from the specified ini file (see #SettingsHandle and #loadSettingsFile);.
    ///
	/// \fn double getSettingsEntryFloat(SettingsHandle, const char*, double);
	/// \ingroup Settings
	/// \brief Gets an floating point number from the specified ini file (see #SettingsHandle and #loadSettingsFile);.
    ///
	/// \fn void getSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Default, char* Data, int MaxDataSize);
	/// \ingroup Settings
	/// \brief Gets a string from the specified ini file (see #SettingsHandle and #loadSettingsFile);.
    ///
	/// \fn void setSettingsEntryInt(SettingsHandle SettingsFile, const char* Node, int Value);
	/// \ingroup Settings
	/// \brief Sets an integer entry in the specified ini file (see #SettingsHandle and #loadSettingsFile);.
    ///
	/// \fn void setSettingsEntryFloat(SettingsHandle SettingsFile, const char* Node, double Value);
	/// \ingroup Settings
	/// \brief Sets a floating point entry in the specified ini file (see #SettingsHandle and #loadSettingsFile);.
    ///
	/// \fn void setSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Value);
	/// \ingroup Settings
	/// \brief Sets a string in the specified ini file (see #SettingsHandle and #loadSettingsFile);.
    ///
	/// \fn void saveSettings(SettingsHandle)
	/// \ingroup Settings
	/// \brief Saves the changes to the specified Settings file.
    ///
	/// \fn void closeSettingsFile(SettingsHandle SettingsFile);
	/// \ingroup Settings
	/// \brief Closes the specified ini file and stores the set entries (see #SettingsHandle and #loadSettingsFile);.

	SPECTRALRADAR_API SettingsHandle loadSettingsFile(const char* Path);
	SPECTRALRADAR_API int getSettingsEntryInt(SettingsHandle SettingsFile, const char* Node, int DefaultValue);
	SPECTRALRADAR_API double getSettingsEntryFloat(SettingsHandle SettingsFile, const char* Node, double DefaultValue);
	SPECTRALRADAR_API void getSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Default, char* Data, int MaxDataSize);
	SPECTRALRADAR_API void setSettingsEntryInt(SettingsHandle SettingsFile, const char* Node, int Value);
	SPECTRALRADAR_API void setSettingsEntryFloat(SettingsHandle SettingsFile, const char* Node, double Value);
	SPECTRALRADAR_API void saveSettings(SettingsHandle SettingsFile);
	SPECTRALRADAR_API void closeSettingsFile(SettingsHandle SettingsFile);
	SPECTRALRADAR_API void setSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Value);

	// COLORING

	/// \defgroup Coloring Coloring
	/// \brief Functions used for coloring of floating point data.
    ///
	/// \enum ColorScheme
	/// \ingroup Coloring
	/// \brief selects the ColorScheme of the data to transform real data to colored data.
	typedef enum {
		/// Black and white (monochrome) coloring
		ColorScheme_BlackAndWhite = 0,
		/// Black and white inverted (monochrome inverted) coloring
		ColorScheme_Inverted = 1,
		/// colored
		ColorScheme_Color = 2,
		/// orange and black coloring
		ColorScheme_BlackAndOrange = 3,
		/// red and black coloring
		ColorScheme_BlackAndRed = 4,
		/// black, red and yellow coloring
		ColorScheme_BlackRedAndYellow = 5,
		/// Doppler phase data coloring. Red and blue allways colored in a range from -pi to +pi. 
		/// Setting the boundaries for this color scheme is only allowed inbetween +pi and -pi
		ColorScheme_DopplerPhase = 6,
		/// blue and black coloring
		ColorScheme_BlueAndBlack = 7,
		/// colorful colorscheme
		ColorScheme_PolarizationRetardation = 8,
	} ColorScheme;

	/// \enum ColoringByteOrder
	/// \ingroup Coloring
	/// \brief Selects the byte order of the coloring to be applied.
	typedef enum {
		/// Byte order RGBA.
		Coloring_RGBA = 0,
		/// Byte order BGRA.
		Coloring_BGRA = 1,
		/// Byte order ARGB.
		Coloring_ARGB = 2
	} ColoringByteOrder;

	/// \enum ColorEnhancement
	/// \ingroup Coloring
	/// \brief Selects the byte order of the coloring to be applied.
	typedef enum {
		/// Use no color enhancement
		ColorEnhancement_None = 0,
		/// Apply a sine function as enhancement
		ColorEnhancement_Sine = 1,
		/// Apply a parable as enhancement
		ColorEnhancement_Parable = 2,
		/// Apply a cubic function as enhancement
		ColorEnhancement_Cubic = 3,
		/// Aplly a sqrt function as enhancement
		ColorEnhancement_Sqrt = 4
	} ColorEnhancement;

	/// \fn Coloring32BitHandle createColoring32Bit(ColorScheme, ColoringByteOrder)
	/// \ingroup Coloring
	/// \brief Creates processing that can be used to color given floating point B-scans to 32 bit colored images.
	///
	/// \param Color The color-table to be used
	/// \param ByteOrder The byte order the coloring is supposed to use. 
	/// \return The handle (#Coloring32BitHandle) to the coloring algorithm. 
	/// 
	/// \fn Coloring32BitHandle createCustomColoring32Bit(int LUTSize, unsigned long LUT[]);
	/// \ingroup Coloring
	/// \brief Create custom coloring using the specified color look-up-table. 
    ///
	/// \fn void setColoringBoundaries(Coloring32BitHandle, float, float)
	/// \ingroup Coloring
	/// \brief Sets the boundaries in dB which are used by the coloring algorithm to map colors to floating point values in dB.
	/// 
	/// \fn void setColoringEnhancement(Coloring32BitHandle, ColorEnhancement)
	/// \ingroup Coloring
	/// \brief Selects a function for non-linear coloring to enhance (subjective) image impression.
    ///
	/// \fn void colorizeData(Coloring32BitHandle Coloring, DataHandle Data, ColoredDataHandle ColoredData, BOOL Transpose)
	/// \ingroup Coloring
	/// \brief Colors a given data object (#DataHandle) into a given colored object (#ColoredDataHandle).
	///
	/// \fn oid colorizeDopplerData(Coloring32BitHandle AmpColoring, Coloring32BitHandle PhaseColoring, DataHandle AmpData, 
	/// DataHandle PhaseData, ColoredDataHandle Output, double MinSignal_dB, BOOL Transpose); 
	/// \ingroup Coloring
	/// \brief Colors a two given data object (#DataHandle) using overlay and intensity to represent phase and amplitude data. Used for Doppler imaging.
    ///
	/// \fn void clearColoring32Bit(Coloring32BitHandle Color)
	/// \ingroup Coloring
	/// \brief Clears the coloring previously created by #createColoring32Bit().
	/// 
	SPECTRALRADAR_API Coloring32BitHandle createColoring32Bit(ColorScheme Color, ColoringByteOrder ByteOrder);
	SPECTRALRADAR_API Coloring32BitHandle createCustomColoring32Bit(int LUTSize, unsigned long LUT[]);
	SPECTRALRADAR_API void setColoringBoundaries(Coloring32BitHandle Colorng, float Min_dB, float Max_dB);
	SPECTRALRADAR_API void setColoringEnhancement(Coloring32BitHandle Coloring, ColorEnhancement Enhancement);
	SPECTRALRADAR_API void colorizeData(Coloring32BitHandle Coloring, DataHandle Data, ColoredDataHandle ColoredData, BOOL Transpose);
	SPECTRALRADAR_API void colorizeDopplerData(Coloring32BitHandle AmpColoring, Coloring32BitHandle PhaseColoring, DataHandle AmpData, DataHandle PhaseData, 
										   ColoredDataHandle Output, double MinSignal_dB, BOOL Transpose);
	SPECTRALRADAR_API void clearColoring32Bit(Coloring32BitHandle Coloring);

	// CAMERA IMAGE
	
	/// \defgroup Camera Camera
	/// \brief Functions for acquiring camera video images.
    ///
	/// \enum CameraPropertyFloat
	/// \ingroup Camera
	/// \brief Enum identifying properties of the camera.
	typedef enum {
		/// Saturation of the video camera.
		Camera_Saturation,
		/// Brightness of the video camera.
		Camera_Brightness, 
		/// Contrast of the video camera.
		Camera_Contrast,
		/// Red white-balance value
		Camera_WB_Red,
		/// Red white-balance value
		Camera_WB_Green,
		/// Red white-balance value
		Camera_WB_Blue,
		/// Automatic setting of white balance values
		Camera_WB_Auto
	} CameraPropertyFloat;

	/// \fn void getMaxCameraImageSize(OCTDeviceHandle, int*, int*)
	/// \ingroup Camera
	/// \brief Returns the maximum possible camera image size for the current device.
    ///
	/// \fn void getCameraImage(OCTDeviceHandle, int SizeX, int SizeY, ColoredDataHandle)
	/// \ingroup Camera
	/// \brief Gets a camera image.
    ///
	/// \fn void getFlippedCameraImage(OCTDeviceHandle, int SizeX, int SizeY, ColoredDataHandle)
	/// \ingroup Camera
	/// \brief Gets a camera image.
	///
	/// The returned camera image is mirrored in the Y axis.
    ///
	/// \fn void getMirroredCameraImage(OCTDeviceHandle, int SizeX, int SizeY, ColoredDataHandle)
	/// \ingroup Camera
	/// \brief Gets a camera image.
	///
	/// The returned camera image is mirrored in the X axis.
    ///
	/// \fn void setCameraPropertyFloat(OCTDeviceHandle, CameraPropertyFloat, double);
	/// \ingroup Camera
	/// \brief Sets saturation, brightness and contrast for the camera images if this option is available for the current device.
	///
	/// \fn void getCameraImageWithOverlay(OCTDeviceHandle Dev, ProbeHandle Probe, ScanPatternHandle Pattern, int SizeX, int SizeY, ColoredDataHandle Image, float Opacity);
	/// \ingroup Camera
	/// \brief Gives the camera image with an overlay of the specified #ScanPatternHandle drawn onto the image. 
	///
	/// \fn void setCameraShowScanPattern(OCTDeviceHandle Dev, BOOL Value);
	/// \ingroup Camera
	/// \brief Enables to turn on/off the scan pattern overlay in the video camera image.
	///
	/// \fn void void visualizeScanPattern(OCTDeviceHandle Dev, ProbeHandle Probe, ScanPatternHandle Pattern, BOOL showRawPattern);
	/// \ingroup Camera
	/// \brief Visualizes the scan pattern in top of the camera image.

	SPECTRALRADAR_API void getMaxCameraImageSize(OCTDeviceHandle Dev, int* SizeX, int* SizeY);
	SPECTRALRADAR_API void getCameraImage(OCTDeviceHandle Dev, int SizeX, int SizeY, ColoredDataHandle Image);
	SPECTRALRADAR_API void getFlippedCameraImage(OCTDeviceHandle Dev, int SizeX, int SizeY, ColoredDataHandle Image, BOOL InvertX, BOOL InvertY);
	SPECTRALRADAR_API void getMirroredCameraImage(OCTDeviceHandle Dev, int SizeX, int SizeY, ColoredDataHandle Image);
	SPECTRALRADAR_API void setCameraPropertyFloat(OCTDeviceHandle Dev, CameraPropertyFloat Selection, double Value);
	SPECTRALRADAR_API void setCameraShowScanPattern(OCTDeviceHandle Dev, BOOL Value);
	SPECTRALRADAR_API void visualizeScanPattern(OCTDeviceHandle Dev, ProbeHandle Probe, ScanPatternHandle Pattern, BOOL showRawPattern);
	// SPECTRALRADAR_API void getCameraImageWithOverlay(OCTDeviceHandle Dev, ProbeHandle Probe, ScanPatternHandle Pattern, int SizeX, int SizeY, ColoredDataHandle Image, float Opacity);

	// HELPER
	
	/// \fn void getConfigPath(char* Path, int StrSize);
	/// \ingroup Helper
	/// \brief Returns the path that hold the config files.
	///
	/// \fn void getPluginPath(char* Path, int StrSize);
	/// \ingroup Helper
	/// \brief Returns the path that hold the plugins.
	///
	/// \fn void getInstallationPath(char* Path, int StrSize);
	/// \ingroup Helper
	/// \brief Returns the installation path.
    ///
	/// \fn double getReferenceIntensity(ProcessingHandle Proc)
	/// \ingroup Helper
	/// \brief Returns an absolute value that indicates the refernce intensity that was present when the currently used apodization was determined.
    ///
	/// \fn double double getRelativeReferenceIntensity(OCTDeviceHandle Dev, ProcessingHandle Proc)
	/// \ingroup Helper
	/// \brief Returns a value larger than 0.0 and smaller than 1.0 that indicates the refernce intensity that was present when the currently used apodization was determined.

	SPECTRALRADAR_API double getReferenceIntensity(ProcessingHandle Proc);
	SPECTRALRADAR_API double getRelativeReferenceIntensity(OCTDeviceHandle Dev, ProcessingHandle Proc);
	SPECTRALRADAR_API void getConfigPath(char* Path, int StrSize);
	SPECTRALRADAR_API void getPluginPath(char* Path, int StrSize);
	SPECTRALRADAR_API void getInstallationPath(char* Path, int StrSize);

	// BUFFER
	
	/// \defgroup Buffer Buffer
	/// \brief Functions for acquiring camera video images.
    ///
	/// \fn BufferHandle createMemoryBuffer()
	/// \ingroup Buffer
	/// \brief Creates a buffer holding data and colored data.
    ///
	/// \fn void appendToBuffer(BufferHandle, DataHandle, ColoredDataHandle)
	/// \ingroup Buffer
	/// \brief Appends specified data and colored data to the requested buffer. 
	/// 
	/// If insufficient memory is availalbe the oldest items in the buffer will be freed automatically. 
    ///
	/// \fn int getBufferSize(BufferHandle)
	/// \ingroup Buffer
	/// \brief Returns the currently avaiable data sets in the buffer.
    ///
	/// \fn DataHandle getBufferData(BufferHandle, int Index)
	/// \ingroup Buffer
	/// \brief Returns the data in the buffer.
    ///
	/// \fn ColoredDataHandle getColoredBufferData(BufferHandle, int Index)
	/// \ingroup Buffer
	/// \brief Returns the colored data in the buffer.
    ///
	/// \fn void clearBuffer(BufferHandle)
	/// \ingroup Buffer
	/// \brief Clears the buffer and frees all data and colored data objects in it. 

	SPECTRALRADAR_API BufferHandle createMemoryBuffer(void);
	SPECTRALRADAR_API void appendToBuffer(BufferHandle, DataHandle, ColoredDataHandle);
	SPECTRALRADAR_API int getBufferSize(BufferHandle);
	SPECTRALRADAR_API DataHandle getBufferData(BufferHandle, int Index);
	SPECTRALRADAR_API ColoredDataHandle getColoredBufferData(BufferHandle, int Index);
	SPECTRALRADAR_API void clearBuffer(BufferHandle);

	/// \fn int getNumberOfOutputValues(OCTDeviceHandle Dev)
	/// \ingroup Output Values
	/// \brief Returns the number of output values.
    ///
	/// \fn void getOutputValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);
	/// \ingroup Output Values
	/// \brief Returns names and units of the requested output values.
    ///
	/// \fn void setOutputValueByName(OCTDeviceHandle Dev, const char* Name, double value);
	/// \ingroup Output Values
	/// \brief Sets the specified output value.
    ///
	/// \fn void setOutputValueByIndex(OCTDeviceHandle Dev, int Index, double Value);
	/// \ingroup Output Values
	/// \brief Sets the specified output value.
	/// 
	/// \fn void getOutputValueRangeByName(OCTDeviceHandle Dev, const char* Name, double* Min, double* Max);
	/// \ingroup Output Values
	/// \brief Gives the range of the specified output value.
	/// 
	/// \fn void getOutputValueRangeByIndex(OCTDeviceHandle Dev, int Index, double* Min, double* Max);
	/// \ingroup Output Values
	/// \brief Gives the range of the specified output value.

	SPECTRALRADAR_API int getNumberOfOutputValues(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void getOutputValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);
	SPECTRALRADAR_API BOOL doesOutputExist(OCTDeviceHandle Dev, const char* Name);
	SPECTRALRADAR_API void setOutputValueByName(OCTDeviceHandle Dev, const char* Name, double value);
	SPECTRALRADAR_API void setOutputValueByIndex(OCTDeviceHandle Dev, int Index, double Value);
	SPECTRALRADAR_API void getOutputValueRangeByName(OCTDeviceHandle Dev, const char* Name, double* Min, double* Max);
	SPECTRALRADAR_API void getOutputValueRangeByIndex(OCTDeviceHandle Dev, int Index, double* Min, double* Max);

	/// \fn void computeLinearKRawData(ComplexDataHandle ComplexDataAfterFFT, DataHandle LinearKData);
	/// \ingroup Processing
	/// \brief Computes the linear k raw data of the complex data after FFT by an inverse Fourier transform.
	///
	/// \fn void linearizeSpectralData(DataHandle SpectraIn, DataHandle SpectraOut, DataHandle Chirp);
	/// \ingroup Processing
	/// \brief Linearizes the spectral data using the given chirp vector.
	SPECTRALRADAR_API void computeLinearKRawData(ComplexDataHandle ComplexDataAfterFFT, DataHandle LinearKData);
	SPECTRALRADAR_API void linearizeSpectralData(DataHandle SpectraIn, DataHandle SpectraOut, DataHandle Chirp);
	
	// FILE HANDLING

	/// \defgroup filehandling File Handling

	/// \enum OCTFileFormat
	/// \ingroup filehandling
	/// \brief Enum identifying possible file formats
	typedef enum _OCTFileFormat {
		// Unknown (auto-determine) and proprietary file formats
		FF_Unknown,
		FF_SRM,
		FF_RAW,
		FF_SDR,
		FF_PHS,
		FF_IMG,
		FF_CSV,
		FF_TXT,
		FF_TABLETXT,
		FF_OCITY,

		// Full-resolution capable file formats
		FF_FITS,
		FF_VTK,
		FF_VFF,

		// Colored data only file formats
		FF_TIFF, 
		FF_JPG,	// 8 bit/channel
		FF_BMP, // 8 bit/channel
		FF_PNG	// up to 16bit/channel
	} OCTFileFormat;


	/// \enum DataKind
	/// \ingroup filehandling
	/// \brief Enum identifying 
	typedef enum _DataKind {
		dkReal,
		dkColored,
		dkComplex,
		dkRaw,
		dkBinary,
		dkText,
		dkUnknown = 999
	} DataKind;


	/// \enum FileMetadataFloatField
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of floating point type
	typedef enum {
		/// The refractive index applied to the whole image. 
		FMD_RefractiveIndex,
		/// The FOV in longitudinal axis (z) in mm.
		FMD_RangeX,
		/// The FOV in axial direction (x) in mm.
		FMD_RangeY,
		/// The FOV in axial direction (y) in mm.
		FMD_RangeZ,
		/// The center of the scan pattern in axial direction (x) in mm.
		FMD_CenterX,
		/// The center of the scan pattern in axial direction (y) in mm.
		FMD_CenterY,
		/// The angle betwenn the scanner and the video camera image.
		FMD_Angle,
		/// Ratio between the binary value from the camera to the count of electrons.
		FMD_BinToElectronScaling,
		/// Central wavelength of the device.
		FMD_CentralWavelength,
		/// Bandwidth of the light source.
		FMD_SourceBandwidth,
		FMD_ElectronFloor,
		/// Lower border of the dynamic range.
		FMD_DynamicRange_Lower,
		/// Upper border of the dynamic range.
		FMD_DynamicRange_Upper,
		FMD_Rotation3D_X,
		FMD_Rotation3D_Y,
		FMD_ClipPlaneDepth3D,
		FMD_QuadraticDispersionCorrectionFactor,
		/// Threshold for speckle variance mode.
		FMD_SpeckleVarianceThreshold,
		/// Time needed for data acqusition. The orocessing and saving time is not included.
		FMD_ScanTime,
		/// Value for the reference intensity.
		FMD_ReferenceIntensity,
		/// 
		FMD_ScanPause,
		/// Zooms the scan pattern.
		FMD_Zoom,
		/// Minimum distance between two points of the scan pattern used for freeform scan patterns.
		FMD_MinPointDistance,
		/// Maximum distance between two points of the scan pattern used for freeform scan patterns.
		FMD_MaxPointDistance
	} FileMetadataFloatField;

	/// \enum FileMetadataIntField
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of integral type
	typedef enum {
		/// Contains the specifif data format.
		FMD_ProcessState,
		/// Number of pixels in x.
		FMD_SizeX,
		/// Number of pixels in y.
		FMD_SizeY,
		/// Number of pixels in z.
		FMD_SizeZ,
		/// Oversampling parameter.
		FMD_Oversampling,
		/// 
		FMD_IntensityAveragedSpectra,
		/// A-scan averaging
		FMD_IntensityAveragedAScans,
		/// B-scan averaging
		FMD_IntensityAveragedBScans,
		/// Averaging for doppler processing in x-direction.
		FMD_DopplerAverageX,
		/// Averaging for doppler processing in z-direction.
		FMD_DopplerAverageZ,
		/// Type of window used for apodization.
		FMD_ApoWindow,
		/// Bits per pixel of the camera.
		FMD_DeviceBitDepth,
		/// Number of elements of the spectrometer.
		FMD_SpectrometerElements,
		/// Actual colormpa
		FMD_Colormap,
		/// Ratio between pixel and real size.
		FMD_Aspect,
		/// Serial number of the dataset
		FMD_ExperimentNumber,
		/// Preset for scan speed. 
		FMD_DevicePreset,
		/// Timestamp of acquired dataset.
		FMD_Timestamp,
		/// Compression used to create the zipped container (.oct-file). No compression used right now.
		FMD_CompressionLevel,
		/// Bytes per pixel of the camera.
		FMD_DeviceBytesPerPixel,
		/// Averaging parameter of the fast scan axis in speckle variance mode.
		FMD_SpeckleAveragingFastAxis,
		/// Averaging parameter of the slow scan axis in speckle variance mode.
		FMD_SpeckleAveragingSlowAxis
	} FileMetadataIntField;

	/// \enum FileMetadataStringField
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of character string type
	typedef enum {
		/// Order of the axis, e.g. ZXY.
		FMD_AxisOrder,
		/// Name of the OCT device.
		FMD_DeviceName,
		/// Serial number of the OCT device.
		FMD_Serial,
		/// Comment of the OCT data file.
		FMD_Comment,
		/// 
		FMD_CustomInfo,
		/// Acquisition mode of the OCT data file.
		FMD_AcquisitionMode,
		/// Study of the OCT data file.
		FMD_Study,
		/// Dispersion Preset of the OCT data file.
		FMD_DispersionPreset,
		/// Name of the probe.
		FMD_ProbeName,
		/// Description of the scan speed, e.g. "Default (48kHz").
		FMD_DevicePresetDescription
	} FileMetadataStringField;

	/// \enum FileMetadataBoolField
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of bool type
	typedef enum {
		FMD_OffsetApplied,
		FMD_DCSubracted,
		FMD_ApoApplied,
		FMD_DechirpApplied,
		FMD_UndersamplingFilterApplied,
		FMD_DispersionCompensationApplied,
		FMD_QuadraticDispersionCorrectionUsed,
		FMD_ImageFieldCorrectionApplied,
		FMD_ScanLineShown,
		FMD_AutoCorrCompensationUsed,
		FMD_BScanCrossCorrelation,
		FMD_DCSubractedAdvanced,
		FMD_OnlyWindowing,
		FMD_RawDataIsSigned,
		FMD_FreeformScanPatternIsActive,
		FMD_FreeformScanPatternCloseLoop,
		FMD_FreeformScanPatternSplineInterpolation

	} FileMetadataBoolField;

	SPECTRALRADAR_API void TestFileEngine();
	SPECTRALRADAR_API OCTFileHandle createOCTFile(OCTFileFormat format);
	SPECTRALRADAR_API void clearOCTFile(OCTFileHandle handle);
	SPECTRALRADAR_API int getFileDataFileCount(OCTFileHandle handle);
	SPECTRALRADAR_API void loadFile(OCTFileHandle handle, const char* fileName);
	SPECTRALRADAR_API void saveFile(OCTFileHandle handle, const char* fileName);
	SPECTRALRADAR_API void copyFileMetadata(OCTFileHandle src, OCTFileHandle dest);
	SPECTRALRADAR_API double getFileMetadataFloat(OCTFileHandle handle, FileMetadataFloatField floatfield);
	SPECTRALRADAR_API void setFileMetadataFloat(OCTFileHandle handle, FileMetadataFloatField floatfield, double value);
	SPECTRALRADAR_API int getFileMetadataInt(OCTFileHandle handle, FileMetadataIntField intfield);
	SPECTRALRADAR_API void setFileMetadataInt(OCTFileHandle handle, FileMetadataIntField intfield, int value);
	SPECTRALRADAR_API size_t getFileMetadataString(OCTFileHandle handle, FileMetadataStringField stringfield, char* content, int length);
	SPECTRALRADAR_API void setFileMetadataString(OCTFileHandle handle, FileMetadataStringField stringfield, const char* content);
	SPECTRALRADAR_API BOOL getFileMetadataBool(OCTFileHandle handle, FileMetadataBoolField boolfield);
	SPECTRALRADAR_API void setFileMetadataBool(OCTFileHandle handle, FileMetadataBoolField boolfield, BOOL value);

	SPECTRALRADAR_API DataHandle getFileRealData(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API ColoredDataHandle getFileColoredData(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API ComplexDataHandle getFileComplexData(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API RawDataHandle getFileRawData(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API ColoredDataHandle getFileBinaryData(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API void getFile(OCTFileHandle handle, size_t index, const char* filenameOnDisk);

	SPECTRALRADAR_API void addFileRealData(OCTFileHandle handle, DataHandle data, const char* filename);
	SPECTRALRADAR_API void addFileColoredData(OCTFileHandle handle, ColoredDataHandle data, const char* filename);
	SPECTRALRADAR_API void addFileComplexData(OCTFileHandle handle, ComplexDataHandle data, const char* filename);
	SPECTRALRADAR_API void addFileRawData(OCTFileHandle handle, RawDataHandle data, const char* filename);
	SPECTRALRADAR_API void addFileBinaryData(OCTFileHandle handle, ColoredDataHandle data, const char* filename);
	SPECTRALRADAR_API void addFileBinary(OCTFileHandle handle, const char* filenameOnDisk, const char* filename);
	SPECTRALRADAR_API void addFileText(OCTFileHandle handle, const char* filenameOnDisk, const char* filename);

	SPECTRALRADAR_API DataKind getFileDataFileType(OCTFileHandle handle, int index);
	SPECTRALRADAR_API void getFileDataFileName(OCTFileHandle handle, int index, char* filename, int length);
	SPECTRALRADAR_API int getFileDataSizeX(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API int getFileDataSizeY(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API int getFileDataSizeZ(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API float getFileDataRangeX(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API float getFileDataRangeY(OCTFileHandle handle, size_t index);
	SPECTRALRADAR_API float getFileDataRangeZ(OCTFileHandle handle, size_t index);

	SPECTRALRADAR_API void setMarkerListFromRealData(OCTFileHandle handle, DataHandle data);
	SPECTRALRADAR_API void setMarkerListInRealData(OCTFileHandle handle, DataHandle data);

	// SPECKLE VARIANCE
	struct C_SpeckleVariance;
	typedef struct C_SpeckleVariance* SpeckleVarianceHandle;

	typedef enum {
		SpeckleVariance_LogscaleVariance_Linear,
		SpeckleVariance_LogscaleVariance_Logscale,
		SpeckleVariance_LinearVariance_Linear,
		SpeckleVariance_LinearVariance_Logscale,
		SpeckleVariance_ComplexVariance_Linear,
		SpeckleVariance_ComplexVariance_Logscale
	} SpeckleVarianceType;

	SPECTRALRADAR_API SpeckleVarianceHandle initSpeckleVariance(void);
	SPECTRALRADAR_API void closeSpeckleVariance(SpeckleVarianceHandle SpeckleVar);
	SPECTRALRADAR_API void setAveraging(SpeckleVarianceHandle SpeckleVar, int Av1, int Av2, int Av3);
	SPECTRALRADAR_API void setSpeckleVarianceType(SpeckleVarianceHandle SpeckleVar, SpeckleVarianceType Type);
	SPECTRALRADAR_API void computeSpeckleVariance(SpeckleVarianceHandle SpeckleVar, ComplexDataHandle CompDataIn, DataHandle DataOutMean, DataHandle DataOutVar);
	SPECTRALRADAR_API void setSpeckleVarianceThreshold(SpeckleVarianceHandle SpecleVar, double Threshold);


	/// BETA_API
	//
	/// \defgroup BETA_API
	// BETA APIS 
	// DO NOT USE, UNLESS YOU KNOW WHAT YOU ARE DOING
	// 
	typedef enum {
		Trigger_FreeRunning,
		Trigger_TrigBoard_ExternalStart
	} Device_TriggerType;


	SPECTRALRADAR_API void setTrigger(OCTDeviceHandle Dev, Device_TriggerType Trigger, double Timeout);

	/// \enum InterpolationMethod
	/// \ingroup BETA_API
	/// \brief Selects the interpolation method.
	typedef enum {
		/// Linear interpolation.
		Interpolation_Linear,
		/// Cubic B-Spline interpolation.
		Interpolation_Spline,
	} InterpolationMethod;

	/// \enum BoundaryCondition
	/// \ingroup BETA_API
	/// \brief Selects the boundary conditions for the interpolation.
	typedef enum {
		BoundaryCondition_standard,
		BoundaryCondition_natural,
		BoundaryCondition_periodic,
	} BoundaryCondition;

	/// \fn BOOL checkAvailableMemoryForScanPattern(OCTDeviceHandle Dev, ScanPatternHandle Pattern, ptrdiff_t AdditionalMemory)
	/// \brief Checks whether sufficient memory is available for acquiring the specified scan pattern.
	/// 
	/// \par AdditionalMemory The parameter specifies additional memory that will be required during the measurement (from startMeasurement() 
	/// to stopMeasruement()) unknown to the SDK and/or memory that will be freed/available prior to the call of startMeasurement(). 
	
	SPECTRALRADAR_API void interpolateScanPattern(DataHandle DataIn, DataHandle DataOut, InterpolationMethod method, BoundaryCondition condition);
	SPECTRALRADAR_API double meanFreeformScanPatternPathLength(DataHandle ScanPattern);


	SPECTRALRADAR_API BOOL checkAvailableMemoryForScanPattern(OCTDeviceHandle Dev, ScanPatternHandle Pattern, ptrdiff_t AdditionalMemory);

	SPECTRALRADAR_API void startMeasurePulseResponse(OCTDeviceHandle Dev, double time_ms, double voltage, int* samplesPerChannel);
	SPECTRALRADAR_API void getPulseResponseInput(OCTDeviceHandle Dev, double* Raw);
	SPECTRALRADAR_API void getPulseResponse(OCTDeviceHandle Dev, double* RawX, double* RawY);
	SPECTRALRADAR_API void stopMeasurePulseRespone(OCTDeviceHandle Dev);
	

	SPECTRALRADAR_API void startScanBenchmark(OCTDeviceHandle Dev, double* RawX, double* RawY, int Size, double RateHz);
	SPECTRALRADAR_API void stopScanBenchmark(OCTDeviceHandle Dev);
	SPECTRALRADAR_API void getScanFeedback(OCTDeviceHandle Dev, double* RawX, double* RawY);
	SPECTRALRADAR_API int getScanFeedbackSize(OCTDeviceHandle Dev);

	SPECTRALRADAR_API double QuantumEfficiency(OCTDeviceHandle Dev, double CenterWavelength_nm, double PowerIntoSpectrometer_W, DataHandle Spectrum_e);


	// 
	// FULL RANGE
	// 
	struct C_FullRange;
	typedef struct C_FullRange* FullRangeHandle;
	SPECTRALRADAR_API FullRangeHandle initFullRange();
	SPECTRALRADAR_API void executeFullRange(FullRangeHandle FullRange, DataHandle ApodizedDataIn, ComplexDataHandle ApodizedDataOut);
	SPECTRALRADAR_API void closeFullRange(FullRangeHandle FullRange);
	SPECTRALRADAR_API void setFullRangeSensitivity(FullRangeHandle FullRangeObject, float CutOff, float Smoothness);
	SPECTRALRADAR_API void executeComplexProcessing(ProcessingHandle Proc, ComplexDataHandle ApodizedSpectralData);

	// POST PROCESSING 
	SPECTRALRADAR_API void determineThickness(DataHandle Data, float* front, float* back);

	SPECTRALRADAR_API void flattenImage(DataHandle ImageData);
	SPECTRALRADAR_API void determineSurface(DataHandle Volume, DataHandle Surface);

	SPECTRALRADAR_API unsigned long long getFreeMemory();

	SPECTRALRADAR_API void absComplexData(ComplexDataHandle ComplexData, DataHandle Abs);
	SPECTRALRADAR_API void logAbsComplexData(ComplexDataHandle ComplexData, DataHandle dB);
	SPECTRALRADAR_API void argComplexData(ComplexDataHandle ComplexData, DataHandle Arg);
	SPECTRALRADAR_API void realComplexData(ComplexDataHandle ComplexData, DataHandle Real);
	SPECTRALRADAR_API void imagComplexData(ComplexDataHandle ComplexData, DataHandle Imag);

	SPECTRALRADAR_API void equalizeColoredDataHistogram(ColoredDataHandle ColoredData);
	SPECTRALRADAR_API void equalizeDataHistogram(DataHandle Data, double Min, double Max);


	// Filter
	// POSTPROCESSING

	/// \defgroup PostProcessing Post Processing
	/// \brief Algorithms and functions used for post processing of floating point data.
	///
	/// \enum PepperFilterType
	/// \ingroup PostProcessing
	/// \brief Specifies the type of pepper filter to be applied.
	typedef enum {
		/// Values along the horizontal axis are taken into account for the pepper filter.
		PepperFilter_Horizontal,
		/// Values along the vertical axis are taken into account for the pepper filter.
		PepperFilter_Vertical,
		/// Values along the vertical and horizontal axis (star shape) are taken into account for the pepper filter.
		PepperFilter_Star,
		/// Values in a block surrounding the destination pixel are taken into account.
		PepperFilter_Block
	} PepperFilterType;

	typedef enum {
		GaussianFilter_3x3,
		GaussianFilter_5x5,
	} GaussianFilterType;

	typedef enum {
		LaplacianFilter_3x3,
	} LaplacianFilterType;

	typedef enum {
		PrewittFilter_Horizontal_3x3,
		PrewittFilter_Vertical_3x3
	} PrewittFilterType;

	typedef enum {
		SobelFilter_Horizontal_3x3,
		SobelFilter_Vertical_3x3
	} SobelFilterType;

	/// \fn void medianFilter(DataHandle, int);
	/// \ingroup PostProcessing
	/// \brief Computes a median filter on the specified 2D data. 
	///
	/// \fn void levelData(DataHandle Data)
	/// \ingroup PostProcessing
	/// \brief Levels the specified data and removes tilt.
	///
	/// \fn void pepperFilter(DataHandle Data, PepperFilterType Type, float Threshold)
	/// \ingroup PostProcessing
	/// \brief Removes pepper-noise (very low values, i. e. dark spots in the data). This enhances the visual (colored) representation of the data.
	///
	/// The pepper filter compares all pixels to a mean of surrounding pixels. The surrouding pixels taking into account are specified by #PepperFilterType. 
	/// If the pixels is lower than specified by the Threshold the pixel will be replaced by the mean.

	SPECTRALRADAR_API void medianFilter(DataHandle Data, int Rank);
	SPECTRALRADAR_API void pepperFilter(DataHandle Data, PepperFilterType Type, float Threshold);	
	SPECTRALRADAR_API void polynomialFilter(DataHandle Data, int SizeX, int SizeY);
	//SPECTRALRADAR_API void gaussianFilter(DataHandle Data, float* Sigma);

	SPECTRALRADAR_API void gaussianFilter(DataHandle Data, GaussianFilterType Type);
	SPECTRALRADAR_API void prewittFilter(DataHandle Data, PrewittFilterType Type);
	SPECTRALRADAR_API void sobelFilter(DataHandle Data, SobelFilterType Type);
	SPECTRALRADAR_API void laplacianFilter(DataHandle Data, LaplacianFilterType Type);

	// for backwards compatibility
	SPECTRALRADAR_API void applyNonlinearSobelFilter2D(DataHandle Data);
	SPECTRALRADAR_API void applyNonlinearPrewittFilter2D(DataHandle Data);
	SPECTRALRADAR_API void applyConvolutionFilter2D(DataHandle Data, int* Filter);
	SPECTRALRADAR_API void applyMedianFilter2D_1x(DataHandle Data, int Rank);
	
	typedef enum {
		Filter_DarkField,
		Filter_BrightField,
		Filter_PhaseContrast,
	} FilterType;
	
	// TODO: clean up
	//typedef enum {
	//	ConvolutionShape_Full,
	//	ConvolutionShape_Same,
	//	ConvolutionShape_Valid,
	//} ConvolutionShape;

	//SPECTRALRADAR_API void convolveData(DataHandle Data1, DataHandle Data2, ConvolutionShape Shape = ConvolutionShape_Full);

	SPECTRALRADAR_API void applyFilter1D(DataHandle Data, int* Size, float* FilterKernel, Direction FilterDirection = Direction_1, bool Normalization = false);
	SPECTRALRADAR_API void applyFilter2D(DataHandle Data, int* Size, float* FilterKernel, Direction FilterNormalDirection = Direction_3, bool Normalization = false);
	SPECTRALRADAR_API void applyFilter3D(DataHandle Data, int* Size, float* FilterKernel, bool Normalization = false);

	SPECTRALRADAR_API void applyFilter(ComplexDataHandle ComplexData, FilterType Type, double FilterParameter); 
	SPECTRALRADAR_API void normalizeFilter(float* FilterKernel, int FilterSize); 

	SPECTRALRADAR_API void smoothCurve1D(int Size, float* Curve, int DegreePolynom, int NumberPolynoms = 1); 
	SPECTRALRADAR_API float* polynomialFitAndEval1D(int Size, float* OrigPosX, float* OrigY, int DegreePolynom, int EvalSize, float* EvalPosX);
	SPECTRALRADAR_API float calcParableMaximum(float x0, float y0, float yp1, float ym1, float* peak_height = nullptr);

	SPECTRALRADAR_API void thresholdData(DataHandle Data, double Threshold);
	SPECTRALRADAR_API void levelData(DataHandle Data);


	// Experimental
	// SPECTRALRADAR_API void computeDEFR(DataHandle LinearKSpectraIn, DataHandle ImageOut, double Disp);
	SPECTRALRADAR_API void importRealBinaryData(DataHandle RealData, int Size1, int Size2, int Size3, const char* Path);
	SPECTRALRADAR_API void filterDC(DataHandle Data);

	SPECTRALRADAR_API void crossCorrelatedProjection(DataHandle DataIn, DataHandle Res);

	SPECTRALRADAR_API void thresholdDataPtByPt(DataHandle Phase, DataHandle Intensity, float threshold, float targetValue);
	SPECTRALRADAR_API void analyzeScatteringSample(DataHandle Data);

	SPECTRALRADAR_API void getCurrentIntensityStatistics(OCTDeviceHandle Dev, ProcessingHandle Proc, float* relToRefIntensity, float* relToProjAbsIntensity);
	SPECTRALRADAR_API void getCurrentApodizationEdgeChannels(ProcessingHandle Proc, int* LeftPix, int*RightPix);
	
	SPECTRALRADAR_API int getNumberOfProbeConfigs();
	SPECTRALRADAR_API void getProbeConfigName(int index, char* ProbeName, int StringSize);

/// \endcond

// PolarizationProcessing
	/// Polarization code

	/// \defgroup Polarization Polarization 
	/// \brief Polarization Sensitive OCT Processing Routines.
	///
	/// \enum ProcessingPropertyInt
	/// \ingroup Polarization
	/// \brief Values that determine the behaviour of the Polarization processing routines.
	typedef enum {
	} PolarizationPropertyInt;

	/// \enum PolarizationPropertyFloat
	/// \ingroup Polarization
	/// \brief Values that determine the behaviour of the Polarization processing routines.
	typedef enum {
		PolarizationProcessing_SurfaceThreshold
	} PolarizationPropertyFloat;

	/// \enum PolarizationFlag
	/// \ingroup Polarization
	/// \brief Flags that determine the behaviour of the Polarization processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		PolarizationVelocityScaling,
	} PolarizationFlag;

	SPECTRALRADAR_API PolarizationProcessingHandle createPolarizationProcessing(void);
	SPECTRALRADAR_API void closePolarizationProcessing(PolarizationProcessingHandle Polarization);
	SPECTRALRADAR_API void setPolarizationPropertyInt(PolarizationProcessingHandle Polarization, PolarizationPropertyInt Property, int Value);
	SPECTRALRADAR_API void setPolarizationPropertyFloat(PolarizationProcessingHandle Polarization, PolarizationPropertyFloat Property, float Value);
	SPECTRALRADAR_API void setPolarizationFlag(PolarizationProcessingHandle Polarization, PolarizationFlag Flag, BOOL OnOff);
	SPECTRALRADAR_API void setPolarizationIntensityOutput(PolarizationProcessingHandle Polarization, DataHandle AmpOut);
	SPECTRALRADAR_API void setPolarizationRetardationOutput(PolarizationProcessingHandle Polarization, DataHandle PhasesOut);
	SPECTRALRADAR_API void executePolarizationProcessing(PolarizationProcessingHandle Polarization, ComplexDataHandle SData, ComplexDataHandle PData);


// USB controller

	// USB button IDs
	typedef enum
	{
		USB_BTN1 = 0xA0u,
		USB_BTN2 = 0xA1u,
		USB_BTN3 = 0xA2u,
		USB_BTN4 = 0xA3u,
		USB_BTN5 = 0xA4u
	} USBProbeButtonID;

	// USB Interrupt message defines
	typedef enum
	{
		USB_INT_ERR_INVALID_CFG = 0x10,
		USB_INT_BTN = 0xA0u,
		USB_INT_BTN1 = 0xA0u,
		USB_INT_BTN2 = 0xA1u,
		USB_INT_BTN3 = 0xA2u,
		USB_INT_BTN4 = 0xA3u,
		USB_INT_BTN5 = 0xA4u,
		// Messages received from probe controller
		USB_INT_MSG_START = 0xB0u,
		USB_INT_MSG_STOP = 0xB1u,
		USB_INT_MSG_SNAPSHOT = 0xB2u,
		USB_INT_MSG_LED1_ON = 0xB3u,
		USB_INT_MSG_LED1_OFF = 0xB4u,
		USB_INT_MSG_LED2_ON = 0xB5u,
		USB_INT_MSG_LED2_OFF = 0xB6u,
		USB_INT_MSG_FILESIZE = 0xB7u,
		USB_INT_MSG_FIRMWARE_VER = 0xB8u,
		USB_INT_MSG_BITMAPSIZE = 0xB9u
	} USBProbeMessage;

	// Commands issued to probe controller

	typedef enum
	{
		USB_CMD_CONFIGURE = 0x80u,
		USB_CMD_RINGLIGHT_SET = 0x81u,
		USB_CMD_LED1_TOGGLE = 0x82u,
		USB_CMD_LED2_TOGGLE = 0x83u,
		USB_CMD_GET_FIRMWARE_VER = 0x84u,
		USB_CMD_READ_USER_SIG = 0x87u,
		USB_CMD_READ_FILE = 0x88u,
		USB_CMD_READ_KEY_BITMAP = 0x89u
	} USBProbeCommand;

	typedef enum
	{
		USB_CFG_RINGLIGHT_UP = 0x90u,
		USB_CFG_RINGLIGHT_DOWN = 0x91u,
		USB_CFG_ACQ_STARTSTOP = 0x92u,
		USB_CFG_ACQ_SNAPSHOT = 0x93u,
		USB_CFG_NOFUNCTION = 0x9fu
	} USBProbeConfiguration;

	typedef enum
	{
		USB_INT_NOFUNCTION = 0x9Fu,
		USB_INT_ACK = 0x99u
	} USBProbeMisc;

	SPECTRALRADAR_API BOOL initUSBProbeCtrl(OCTDeviceHandle dev);
	SPECTRALRADAR_API BOOL configureUSBProbeCtrlButton(OCTDeviceHandle dev, USBProbeButtonID btn, USBProbeCommand cmd);
	SPECTRALRADAR_API BOOL getLastUSBProbeMessage(OCTDeviceHandle dev, char* msg, size_t size);
	SPECTRALRADAR_API BOOL toggleUSBProbeLED(OCTDeviceHandle Dev, int LED, BOOL OnOff);

#ifdef __cplusplus 
}
#endif

#endif // _SPECTRALRADAR_H

