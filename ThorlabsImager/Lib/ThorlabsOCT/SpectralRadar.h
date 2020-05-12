#ifndef _SPECTRALRADAR_H
#define _SPECTRALRADAR_H

#include <stddef.h>

/*! \mainpage Spectral Radar SDK
\section license SpectralRadar SDK License

By using the Thorlabs SpectralRadar SDK you agree to the terms and conditions detailed in the license agreement provided here: <a href="THORLABS SpectralRadar SDK License Agreement.pdf">
THORLABS SpectralRadar SDK License Agreement</a> (PDF reader required). If this link does not work, you will also find this license agreement in Start Menu -> All Programs -> Thorlabs -> SpectralRadar-SDK.

\section intro Introduction

This document gives an introduction into using the ANSI C Spectral Radar SDK and demonstrates the use of the most important functions.

\subsection overview Overview

The ANSI C Spectral Radar SDK follows an object-oriented approach.
All objects are represented by pointers where appropriate typedefs are provided for convenience. The defined types are called Handles and are used as
return values when created and are passed as value when used. All functionality has been created with full LabVIEW compatibility in mind and it should
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
Data properties can be read via #getDataPropertyInt(), #getDataPropertyFloat(), etc. These include sizes along their first, second and third axis,
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
If no device is available or the processing routines are to be tweaked manually #createProcessing() must be used. When all required processing is done, #clearProcessing()
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
- #ColoringHandle: Handle to processing routines that can map floating point data to color data. In general this will 32 bit color data, such as RGBA or BGRA.

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
clearProcessing(Proc);
// ...
\endcode

\subsection scanpattern Creating A Scan Pattern
In order to scan a sample and acquire B-scan OCT data one needs to specifiy a scan pattern that describes at which point to acquire data. To get the data of a
simple B-Scan on can simply use #createBScanPattern():
\code
// ...
ProbeHandle Probe = initProbe(Dev, "Probe");
ScanPatternHandle Pattern = createBScanPattern(Probe, 2.0, 512); // get B-scans with 2.0mm scanning range and 512 A-scans per B-scan
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
DataHandle BScan = createData();
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
Error handling is done by calling the function #getError(). The function will return an #ErrorCode and if the result is not NoError an error string
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

\section examples Examples

The examples are meant to illustrate the usage of the SDK, and not the way to structure a program.
As such, the examples contain almost no error checking or error handling code. 

The code snippets shown here are part of a Microsoft Visual Studio project, located in directory SpectralRadarDemos, 
which may be useful as a guideline to compile and link the code. 

The SDK functions in the code are links that can be clicked to access deeper information.

\subsection simple_examples Simple

The purpose of the simple examples is to demonstrate the usage of the basic building blocks of the SDK.
Each code snippet focusses on a single concept.

\subsubsection bscan B-Scan measurement

Acquire a simple B-Scan measurement (1024 A-Scans distributed along a segment 2 millimeter long).
\include Simple/BScanMeasurement.cpp

\subsubsection export Export data and images

After a simple B-Scan measurement has been acquired, data are stored (for future post-processing) and exported (for beautiful pictures).
\include Simple/ExportDataAndImage.cpp

\subsection advanced_examples Advanced

The purpose of the advanced examples is to show
- practical combinations of the building blocks,
- auxiliary operations that complement the acquisition and processing routines.

\subsubsection readoct Load and read an OCT file

Load and read the content of previously exported data.
\include Advanced/ReadOCTFile.cpp
To find out other metadata of interest, click on the function and explore other functions in the same group.

\subsubsection writeoct Write an OCT file

Write the results from a measurement to an OCT file.
\include Advanced/WriteOCTFile.cpp

*/

/// \file SpectralRadar.h
/// \brief Header containing all functions of the Spectral Radar SDK. This SDK can be used for Callisto, Ganymede, Hyperion, Telesto and Vega devices.
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
	///
	/// This data structure is an ANSI C equivalent to the C++ data type \code{.cpp} std::complex<float> \endcode.
	/// Notice that arrays of complex data are always in interleaved format (real and imaginary parts of each
	/// element in contiguous memory addresses) and not in split format, where real and imaginary parts are
	/// stored in separate arrays.
	// needed for compatibility reasons if compiled with x byte aligned struct data.
	typedef struct {
		/// data[0] is the real part and data[1] is the imaginary part. 
		float data[2];
	} ComplexFloat;

	/// \typedef RawDataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding the unprocessed raw data. 
	///
	/// Raw data refers to the spectra as acquired, without processing of any kind. This structure accomodates not
	/// only the actual pixel values, but also the meta-data, such as the number of bytes per pixel, the sizes,
	/// the number of elements, or the number of frames that had been lost during the acquisition.
	struct C_RawData;
	typedef struct C_RawData* RawDataHandle;

	/// \typedef DataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding 1-, 2- or 3-dimensional floating point data.
	///
	/// This structure may hold data generated by processing raw data (#RawDataHandle), and also more abstract data,
	/// such as point sequences intended to determine a scan pattern (see e.g. #getSizeOfScanPointsFromDataHandle
	/// or #getScanPointsFromDataHandle below).
	/// The associated properties of the data (dimensionality, sizes in pixels, spacings/ranges in millimeters)
	/// are also part of this structure.\n
	/// This structure supports reuse. That is, once created, it can be reused many times to hold different data.
	/// If passed as a parameter to the processing (e.g. throught the function #setProcessedDataOutput), the meta
	/// data (sizes, ranges, etc.) will be adjusted automatically each time.
	struct C_Data;
	typedef struct C_Data* DataHandle;

	/// \typedef ColoredDataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding 1-, 2- or 3-dimensional colored data.
	///
	/// Colored data handles are used to obtain processed data in a format that can readily be exported into a graphics
	/// format file, using a user selected palette. Otherwise the are the same as processed data (#DataHandle).\n
	/// In order to specify the desired palette and its properties, users should refer to coloring handles (#ColoringHandle)
	/// and associated functions.\n
	/// This structure supports reuse. That is, once created, it can be reused many times to hold different data.
	/// If passed as a parameter to the processing (e.g. throught the function #setColoredDataOutput), the meta
	/// data (sizes, ranges, etc.) will be adjusted automatically each time.
	struct C_ColoredData;
	typedef struct C_ColoredData* ColoredDataHandle;

	/// \typedef ComplexDataHandle
	/// \ingroup Data
	/// \brief Handle to an object holding complex 1-, 2- or 3-dimensional complex floating point data.
	///
	/// This structure supports reuse. That is, once created, it can be reused many times to hold different data.
	/// If passed as a parameter to the processing (e.g. throught the function #setComplexDataOutput), the meta
	/// data (sizes, ranges, etc.) will be adjusted automatically each time.
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
	/// \brief Handle for creating, manipulating, and discarding a scan pattern.
	///
	/// A scan pattern can be created with one of the functions #createNoScanPattern, #createAScanPattern, #createBScanPattern,
	/// #createBScanPatternManual, #createIdealBScanPattern, #createCirclePattern, #createVolumePattern, #createFreeformScanPattern2D,
	/// #createFreeformScanPattern2DFromLUT, #createFreeformScanPattern3DFromLUT, or #createFreeformScanPattern3D.
	struct C_ScanPattern;
	typedef struct C_ScanPattern* ScanPatternHandle;

	/// \typedef ProcessingHandle
	/// \ingroup Processing
	/// \brief Handle for a processing routine.
	///
	/// The purpose of the processing routines is to compute A-Scans (light intensity as a function of depth) from 
	/// spectra (light intensity as a function of wavelength). The former is typically stored in different types of data
	/// (#DataHandle, #ComplexDataHandle, #ColoredDataHandle) whereas the latter is raw data (#RawDataHandle).\n
	/// A handle of processing routines can be obtained with one of the functions #createProcessing, #createProcessingForDevice,
	/// #createProcessingForDeviceEx or #createProcessingForOCTFile.
	struct C_Processing;
	typedef struct C_Processing* ProcessingHandle;

	/// \typedef DopplerProcessingHandle
	/// \ingroup Doppler
	/// \brief Handle used for Doppler processing.
	struct C_DopplerProcessing;
	typedef struct C_DopplerProcessing* DopplerProcessingHandle;

	/// \typedef SpeckleVarianceHandle
	/// \ingroup SpeckleVariance
	/// \brief Handle used for SpeckleVariance  processing.
	/// \defgroup SpeckleVariance Speckle Variance Contrast Processing
	struct C_SpeckleVariance;
	typedef struct C_SpeckleVariance* SpeckleVarianceHandle;

	/// \typedef PolarizationProcessingHandle
	/// \ingroup Polarization
	/// \brief Handle used for Polarization processing.
	struct C_PolarizationProcessing;
	typedef struct C_PolarizationProcessing* PolarizationProcessingHandle;

	/// \typedef ColoringHandle
	/// \ingroup Coloring
	/// \brief Handle for routines that color avaible scans for displaying.
	struct C_Coloring32Bit;
	typedef struct C_Coloring32Bit* ColoringHandle;

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
	/// \ingroup filehandling
	/// \brief Handle to the marker list class
	struct C_MarkerList;
	typedef struct C_MarkerList* MarkerListHandle;

	/// \typedef OCTFileHandle
	/// \ingroup Data
	/// \brief Handle to the OCT file class
	struct C_OCTFileHandle;
	typedef struct C_FileHandling* OCTFileHandle;

	/// \fn void setErrorsPerThreadFlag(bool getErrorsPerThread)
	/// \ingroup ErrorHandling
	/// \brief Sets the whether or not errors are returned per thread in #isError and #getError. By default, errors are collected globally and a call to #getError will return the last error from any thread. If this flag is set to true, only errors from the current thread will be returned.
	SPECTRALRADAR_API void setErrorsPerThreadFlag(bool getErrorsPerThread);

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
		NoError = 0x0000,
		/// Error occurred. This entry can be cast to TRUE.
		Error = 0xE000
	} ErrorCode;

	/// \fn ErrorCode isError(void)
	/// \ingroup ErrorHandling
	/// \brief Returns error code. The error flag will not be cleared; a following call to #getError thus provides detailed error information. 
	SPECTRALRADAR_API ErrorCode isError(void);

	/// \fn ErrorCode getError(char* Message, int StringSize);
	/// \ingroup ErrorHandling
	/// \brief Returns an error code and a message if an error occurred. The error flag will be cleared.
	/// \param[out] Message Error message describing the error.
	/// \param[in] StringSize Size of the string that was given to Message. 
    /// \returns The error code (no error can be casted to FALSE, error can be casted to TRUE).
	/// \sa #ErrorCode.
	///
	/// This function is the ultimate criterium to establish if an error occurred or not. Under certain circumstances SpectralRadar might log
	/// text lines that look like errors, but are no necessarily so. This is because the library has been conceived for very general
	/// settings, across a wide variety of hardware configurations, and messages might be generated to document a particular execution context.
	SPECTRALRADAR_API ErrorCode getError(char* Message, int StringSize);

	/// \enum LogOutputType
	/// \ingroup ErrorHandling
	/// \brief Specifies where to write text output by the SDK.
	typedef enum {
		/// Write to standard output
		Standard,
		/// Write to text file
		File,
		/// Do not write output
		None
	} LogOutputType;

	/// \fn setLog(LogOutputType Type, const char* Filename)
	/// \brief Specifies where to write text output by the SDK. The respective text output might help to debug applications or identify errors and faults.
	/// \param[in] Type Location where to write text output.
	/// \param[out] Filename Full path and filename where to write output, if Type is set to File.
	SPECTRALRADAR_API void setLog(LogOutputType Type, const char* Filename);

	/// \ingroup Hardware
	/// \brief Definition for a generic callback function indicating progress. The argument will be passed a value from 0.0 to 1.0 indicating progress in a respective process. 
	/// \param progress Current progress in a range from 0.0 to 1.0
	/// \param msg given an additional message for the current progress
	typedef void(__stdcall* genericProgressCallback)(double progress, const char* msg);

	/// \defgroup Data Data Access
	/// \brief Functions for accessing the information stored in data objects. 
	///
	/// \enum RawDataPropertyInt
	/// \ingroup Data
	/// \brief Integer properties of raw data (#RawDataHandle) that can be retrieved with the function #getRawDataPropertyInt.
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
		/// The number of lost frames during data acqusition. Lost frames are the usual consequence of a scanning
		/// dynamics going too fast in comparison to the acquisition settings (or capabilities) of the camera. To
		/// remedy the situation, consider increasing the number of measurements along the x-axis (size 2), or
		/// increasing the A-scan averaging. A rule of thumb to check if the settings are safe would be
		/// \f$\lceil 2s_{Hz}f_s\rceil<X_{px}A\f$, where \f$s_{Hz}\f$ is the scan speed expressed in Herz,
		/// \f$f_s\f$ is the flyback time expressed in seconds (as written in the probe configuration file, e.g.
		/// Probe.ini), \f$X_{px}\f$ is the number of desired A-scans along the x axis (i.e. #RawData_Size2), and
		/// \f$A\f$ is the A-scan averaging.
		RawData_LostFrames
	} RawDataPropertyInt;

	/// \enum DataPropertyInt
	/// \ingroup Data
	/// \brief Integer properties of data (#DataHandle) that can be retrieved with the function #getDataPropertyInt.
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
	/// \brief Floating point properties of data (#DataHandle), that can be retrieved with the function #getDataPropertyFloat.
	typedef enum {
		/// Spacing between two subsequent data elements in direction of the first axis in physical units (millimeter).
		Data_Spacing1,
		/// Spacing between two subsequent data elements in direction of the second axis in physical units (millimeter).
		Data_Spacing2,
		/// Spacing between two subsequent data elements in direction of the third axis in physical units (millimeter).
		Data_Spacing3,
		/// Total range of the data in direction of the first axis in physical units (millimeter).
		Data_Range1,
		/// Total range of the data in direction of the second axis in physical units (millimeter).
		Data_Range2,
		/// Total range of the data in direction of the third axis in physical units (millimeter).
		Data_Range3
	} DataPropertyFloat;

	/// \enum DataAnalyzation
	/// \ingroup Data
	/// \brief Analysis types accepted by the functions #analyzeData and #computeDataProjection.
	typedef enum {
		/// Minimum of the values in the data.
		Data_Min,
		/// Arithmetic mean of all values in the data.
		Data_Mean,
		/// Maximum of the values in the data.
		Data_Max,
		/// The depth of the maximum of the values in the data.
		Data_MaxDepth,
		/// The median of the values in the data.
		Data_Median,
	} DataAnalyzation;

	/// \enum AScanAnalyzation
	/// \ingroup Data
	/// \brief Analysis types accepted by the functions #analyzeAScan and #analyzeComplexAScan.
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
		/// The peak height is determined by computing a parable going through the maximum value point and its surrounding pixels. 
		/// The height of the resulting parable is returned. 
		Data_PeakHeight_dB,
		/// Signal width at -6dB. This is the FWHM.
		Data_PeakWidth_6dB,
		/// Signal width at -20dB.
		Data_PeakWidth_20dB,
		/// Signal width at -40dB.
		Data_PeakWidth_40dB,
		/// Phase of the highest peak in radians. This value is only accepted by the function #analyzeComplexAScan.
		Data_PeakPhase,
		/// Real part of the highest peak, expressed in $e^-$. This value is only accepted by the function #analyzeComplexAScan.
		Data_PeakRealPart,
		/// Imaginary part of the highest peak, expressed in $e^-$. This value is only accepted by the function #analyzeComplexAScan.
		Data_PeakImagPart
	} AScanAnalyzation;

	/// \enum DataOrientation
	/// \ingroup Data
	/// \brief Supported data orientations. The default orientation is the first one.
	/// \sa #getDataOrientation, #setDataOrientation, #getColoredDataOrientation, #setColoredDataOrientation.
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
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] Selection The desired property.
    /// \return The value of the desired property.
	SPECTRALRADAR_API int getDataPropertyInt(DataHandle Data, DataPropertyInt Selection);

	/// \fn double getDataPropertyFloat(DataHandle Data, DataPropertyFloat Selection)
	/// \ingroup Data
	/// \brief Returns the selected floating point property of the specified data.
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] Selection The desired property.
    /// \return The value of the desired property.
	SPECTRALRADAR_API double getDataPropertyFloat(DataHandle Data, DataPropertyFloat Selection);

	/// \fn void copyData(DataHandle DataSource, DataHandle DataDestination);
	/// \ingroup Data
	/// \brief Copies the content of the specified source to the specified destination.
	/// \param[in] DataSource A valid (non null) data handle of the source (#DataHandle).
	/// \param[in] DataDestination A valid (non null) data handle of the destination (#DataHandle).
	SPECTRALRADAR_API void copyData(DataHandle DataSource, DataHandle DataDestination);

	/// \fn void copyDataContent(DataHandle, float* )
	/// \ingroup Data
	/// \brief Copies the data in the specified data object (#DataHandle) into the specified pointer.
	/// \param[in] DataSource A valid (non null) data handle of the source (#DataHandle).
	/// \param[out] Destination A valid (non null) pointer to float array, with enough space to copy the data.
	///
	/// In order to find out the amount of memory that has to be reserved, the size(s) of the
	/// source data has to be inquired with the function #getDataPropertyInt (because they are integer properties).
	SPECTRALRADAR_API void copyDataContent(DataHandle DataSource, float* Destination);

	/// \fn float* getDataPtr(DataHandle Data)
	/// \ingroup Data
	/// \brief Returns a pointer to the content of the specified data.
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
    /// \return A pointer to the memory owned by the handle.
	///
	/// The returned pointer points to memory owned by SpectralRadar.dll. The user should not attempt to free it.
	SPECTRALRADAR_API float* getDataPtr(DataHandle Data);

	/// \fn void reserveData(DataHandle Data, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Reserves the amount of data specified. This might improve performance if appending data to the #DataHandle as no additional
	/// memory needs to be reserved then. 
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] Size1 The number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The number of data along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void reserveData(DataHandle Data, int Size1, int Size2, int Size3);

	/// \fn void resizeData(DataHandle Data, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Resizes the respective data object. In general the data will be 1-dimensional if Size2 and Size3 are equal to 
	/// 1, 2-dimensional if Size3 is equal to 1 dn 3-dimensional if all, Size1, Size2, Size3, are unequal to 1.
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void resizeData(DataHandle Data, int Size1, int Size2, int Size3);

	/// \fn void setDataRange(DataHandle Data, double range1, double range2, double range3)
	/// \ingroup Data
	/// \brief Sets the range in mm in the 3 axes represented in the RealData buffer.
	/// \param[in] Data A valid (non null) data handle.
	/// \param[in] range1 The desired physical extension, in mm, along the first axis ("z" in the default orientation).
	/// \param[in] range2 The desired physical extension, in mm, along the second axis ("x" in the default orientation).
	/// \param[in] range3 The desired physical extension, in mm, along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void setDataRange(DataHandle Data, double range1, double range2, double range3);

	/// \fn void setDataContent(DataHandle Data, float* NewContent)
	/// \ingroup Data
	/// \brief Sets the data content of the data object. The data chunk pointed to by NewContent needs
	/// to be of the size expected by the data object, i. e. Size1*Size2*Size*sizeof(float).
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] NewContent A valid (non null) pointer to float array with the source data.
	///
	/// The amount of data that will be copied depends on the size(s) that had previously been
	/// setup in the data object (using #resizeData to ensure that enough space has been allocated).
	SPECTRALRADAR_API void setDataContent(DataHandle Data, float* NewContent);

	/// \fn void DataOrientation getDataOrientation(DataHandle Data)
	/// \ingroup Data
	/// \brief Returns the data orientation of the data object.
	/// \param[in] Data A valid (non null) data handle.
	SPECTRALRADAR_API DataOrientation getDataOrientation(DataHandle Data);

	/// \fn void setDataOrientation(DataHandle Data, DataOrientation)
	/// \ingroup Data
	/// \brief Sets the data oritentation of the data object to the given orientation.
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] Orientation The desired orientation.
	SPECTRALRADAR_API void setDataOrientation(DataHandle Data, DataOrientation Orientation);

	/// \fn int getComplexDataPropertyInt(ComplexDataHandle Data, DataPropertyInt Selection)
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified data.
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
	/// \param[in] Selection The desired property.
    /// \returns The value of the desired property.
	SPECTRALRADAR_API int getComplexDataPropertyInt(ComplexDataHandle Data, DataPropertyInt Selection);

	/// \fn double getComplexDataPropertyFloat(ComplexDataHandle Data, DataPropertyFloat Selection)
	/// \ingroup Data
	/// \brief Returns the selected floating-point property of the specified data.
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
	/// \param[in] Selection The desired property.
	/// \returns The value of the desired property.
	SPECTRALRADAR_API double getComplexDataPropertyFloat(ComplexDataHandle Data, DataPropertyFloat Selection);

	/// \fn void copyComplexDataContent(ComplexDataHandle DataSource, ComplexFloat* Destination);
	/// \ingroup Data
	/// \brief Copies the content of the complex data to the pointer specified as destination.
	/// \param[in] DataSource A valid (non null) complex data handle of the source (#ComplexDataHandle).
	/// \param[out] Destination A valid (non null) pointer to a complex array, with enough space to copy the data.
	///
	/// In order to find out the amount of memory that has to be reserved, the size(s) of the
	/// source data has to be inquired with the function #getComplexDataPropertyInt (because they are integer properties).
	SPECTRALRADAR_API void copyComplexDataContent(ComplexDataHandle DataSource, ComplexFloat* Destination);

	/// \fn void copyComplexData(ComplexDataHandle DataSource, ComplexDataHandle DataDestination);
	/// \ingroup Data
	/// \brief Copies the contents of the specified #ComplexDataHandle to the specified destination #ComplexDataHandle.
	/// \param[in] DataSource A valid (non null) complex data handle of the source (#ComplexDataHandle).
	/// \param[in] DataDestination A valid (non null) complex data handle of the destination (#ComplexDataHandle).
	SPECTRALRADAR_API void copyComplexData(ComplexDataHandle DataSource, ComplexDataHandle DataDestination);

	/// \fn ComplexFloat* getComplexDataPtr(ComplexDataHandle Data);
	/// \ingroup Data
	/// \brief Returns a pointer to the data represented by the #ComplexDataHandle. The data is still managed by the #ComplexDataHandle object.
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
    /// \return A pointer to the memory owned by the handle.
	///
	/// The returned pointer points to memory owned by SpectralRadar.dll. The user should not attempt to free it.
	SPECTRALRADAR_API ComplexFloat* getComplexDataPtr(ComplexDataHandle Data);

	/// \fn void setComplexDataContent(ComplexDataHandle Data, ComplexFloat* NewContent);
	/// \ingroup Data
	/// \brief Sets the data content of the #ComplexDataHandle to the content specified by the pointer.
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
	/// \param[in] NewContent A valid (non null) pointer to an array of complex numbers (#ComplexFloat) with the desired content.
	///
	/// The amount of data that will be copied depends on the size(s) that had previously been
	/// setup in the complex data object.
	SPECTRALRADAR_API void setComplexDataContent(ComplexDataHandle Data, ComplexFloat* NewContent);

	/// \fn void reserveComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Reserves the amount of data specified. This might improve performance if appending data to the #ComplexDataHandle as no additional
	/// memory needs to be reserved then. 
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void reserveComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3);

	/// \fn void resizeComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Resizes the respective data object. In general the data will be 1-dimensional if Size2 and Size3 are equal to 
	/// 1, 2-dimensional if Size3 is equal to 1 dn 3-dimensional if all, Size1, Size2, Size3, are unequal to 1.
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void resizeComplexData(ComplexDataHandle Data, int Size1, int Size2, int Size3);

	/// \fn void setComplexDataRange(ComplexDataHandle Data, double range1, double range2, double range3)
	/// \ingroup Data
	/// \brief Sets the range in mm in the 3 axes represented in the RealData buffer.
	/// \param[in] Data A valid (non null) complex data handle (#ComplexDataHandle).
	/// \param[in] range1 The desired physical extension, in mm, along the first axis ("z" in the default orientation).
	/// \param[in] range2 The desired physical extension, in mm, along the second axis ("x" in the default orientation).
	/// \param[in] range3 The desired physical extension, in mm, along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void setComplexDataRange(ComplexDataHandle Data, double range1, double range2, double range3);

	/// \fn int getColoredDataPropertyInt(ColoredDataHandle ColData, DataPropertyInt Selection);
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified colored data.
	/// \param[in] ColData A valid (non null) colored data handle (#ColoredDataHandle).
	/// \param[in] Selection The desired property.
    /// \return The value of the desired property.
	SPECTRALRADAR_API int getColoredDataPropertyInt(ColoredDataHandle ColData, DataPropertyInt Selection);

	/// \fn int getColoredDataPropertyFloat(ColoredDataHandle ColData, DataPropertyFloat Selection);
	/// \ingroup Data
	/// \brief Returns the selected integer property of the specified colored data.
	/// \param[in] ColData A valid (non null) colored data handle (#ColoredDataHandle).
	/// \param[in] Selection The desired property.
    /// \return The value of the desired property.
	SPECTRALRADAR_API double getColoredDataPropertyFloat(ColoredDataHandle ColData, DataPropertyFloat Selection);

	/// \fn void copyColoredData(ColoredDataHandle ImageSource, ColoredDataHandle ImageDestionation);
	/// \ingroup Data
	/// \brief Copies the contents of the specified #ColoredDataHandle to the specified destination #ColoredDataHandle.
	/// \param[in] ImageSource A valid (non null) colored data handle of the source (#ColoredDataHandle).
	/// \param[in] ImageDestionation A valid (non null) colored data handle of the destination (#ColoredDataHandle).
	SPECTRALRADAR_API void copyColoredData(ColoredDataHandle ImageSource, ColoredDataHandle ImageDestionation);

	/// \fn void copyColoredDataContent(ColoredDataHandle Source, unsigned long* Destination);
	/// \ingroup Data
	/// \brief Copies the data in the specified colored data object (#ColoredDataHandle) into the specified pointer.
	/// \param[in] Source A valid (non null) colored data handle of the source (#ColoredDataHandle).
	/// \param[out] Destination A valid (non null) pointer to an integer array, with enough space to copy the data.
	///
	/// In order to find out the amount of memory that has to be reserved, the size(s) of the
	/// source data has to be inquired with the function #getColoredDataPropertyInt (because they are integer properties).
	SPECTRALRADAR_API void copyColoredDataContent(ColoredDataHandle Source, unsigned long* Destination);

	/// \fn void copyColoredDataContentAligned(ColoredDataHandle ImageSource, unsigned long* Destination, int Stride);
	/// \ingroup Data
	/// \brief Copies the data in the specified colored data object (#ColoredDataHandle) into the specified pointer.
	/// \param[in] ImageSource A valid (non null) colored data handle of the source (#ColoredDataHandle).
	/// \param[out] Destination A valid (non null) pointer to an integer array, with enough space to copy the data.
	/// \param[in] Stride The total amount of bytes per row, which may contain some padding after the last pixel.
	///
	/// In order to find out the amount of memory that has to be reserved, the size(s) of the
	/// source data has to be inquired (they are integer properties).
	SPECTRALRADAR_API void copyColoredDataContentAligned(ColoredDataHandle ImageSource, unsigned long* Destination, int Stride);

	/// \fn unsigned long* getColoredDataPtr(ColoredDataHandle ColData)
	/// \ingroup Data
	/// \brief Returns a pointer to the content of the specified #ColoredDataHandle.
	/// \param[in] ColData A valid (non null) colored data handle (#ColoredDataHandle).
    /// \return A pointer to the memory owned by the handle.
	///
	/// The returned pointer points to memory owned by SpectralRadar.dll. The user should not attempt to free it.
	SPECTRALRADAR_API unsigned long* getColoredDataPtr(ColoredDataHandle ColData);

	/// \fn void resizeColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Resizes the respective colored data object. In general the data will be 1-dimensional if Size2 and Size3 are equal to 
	/// 1, 2-dimensional if Size3 is equal to 1 dn 3-dimensional if all, Size1, Size2, Size3, are unequal to 1.
	/// \param[in] ColData A valid (non null) complex data handle (#ColoredDataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void resizeColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3);

	/// \fn void reserveColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3)
	/// \ingroup Data
	/// \brief Reserves the amount of colored data specified. This might improve performance if appending data to the #ColoredDataHandle as no additional
	/// memory needs to be reserved then. 
	/// \param[in] ColData A valid (non null) colored data handle (#ColoredDataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void reserveColoredData(ColoredDataHandle ColData, int Size1, int Size2, int Size3);

	/// \fn  void setColoredDataContent(ColoredDataHandle ColData, unsigned long* NewContent)
	/// \ingroup Data
	/// \brief Sets the data content of the colored data object. The data chung pointed to by NewContent needs to be of the size expected 
	/// by the data object, i. e. Size1*Size2*Size*sizeof(unsigned long).
	/// \param[in] ColData A valid (non null) colored data handle (#ColoredDataHandle).
	/// \param[in] NewContent A valid (non null) pointer to an integer array with the source data.
	///
	/// The amount of data that will be copied depends on the size(s) that had previously been
	/// setup in the colored data object.
	SPECTRALRADAR_API void setColoredDataContent(ColoredDataHandle ColData, unsigned long* NewContent);

	/// \fn void setColoredDataRange(ColoredDataHandle Data, double range1, double range2, double range3)
	/// \ingroup Data
	/// \brief Sets the range in mm in the 3 axes represented in the data object buffer.
	/// \param[in] Data A valid (non null) colored data handle (#ColoredDataHandle).
	/// \param[in] range1 The desired physical extension, in mm, along the first axis ("z" in the default orientation).
	/// \param[in] range2 The desired physical extension, in mm, along the second axis ("x" in the default orientation).
	/// \param[in] range3 The desired physical extension, in mm, along the third axis ("y" in the default orientation).
	SPECTRALRADAR_API void setColoredDataRange(ColoredDataHandle Data, double range1, double range2, double range3);

	/// \fn DataOrientation getColoredDataOrientation(ColoredDataHandle Data)
	/// \ingroup Data
	/// \brief Returns the data orientation of the colored data object.
	/// \param[in] Data A valid (non null) colored data handle (#ColoredDataHandle).
    /// \return The current orientation (#DataOrientation).
	SPECTRALRADAR_API DataOrientation getColoredDataOrientation(ColoredDataHandle Data);

	/// \fn void setColoredDataOrientation(ColoredDataHandle Data, DataOrientation Orientation)
	/// \ingroup Data
	/// \brief Sets the data oritentation of the colored data object to the given orientation.
	/// \param[in] Data A valid (non null) colored data handle (#ColoredDataHandle).
	/// \param[in] Orientation The desired orientation (#DataOrientation).
	SPECTRALRADAR_API void setColoredDataOrientation(ColoredDataHandle Data, DataOrientation Orientation);

	/// \fn void copyRawDataContent(RawDataHandle RawDataSource, void* DataContent);
	/// \ingroup Data
	/// \brief Copies the content of the raw data into the specified buffer.
	/// \param[in] RawDataSource A valid (non null) raw data handle of the source (#RawDataHandle).
	/// \param[out] DataContent A valid (non null) pointer to an array, with enough space to copy the data.
	///
	/// In order to find out the amount of memory that has to be reserved, the size(s) of the
	/// source data has to be inquired with the function #getRawDataPropertyInt (because they are integer properties).\n
	/// Notice that raw data refers to the spectra acquired, without processing of any kind.
	/// The pointer is void because different cameras/sensors with different amount of bytes per pixel
	/// are supported.
	SPECTRALRADAR_API void copyRawDataContent(RawDataHandle RawDataSource, void* DataContent);

	/// \fn void copyRawData(RawDataHandle RawDataSource, RawDataHandle RawDataTarget)
	/// \ingroup Data
	/// \brief Copies raw data content and metadata into the specified target handle. 
	/// \param[in] RawDataSource A valid (non null) raw data handle of the source (#RawDataHandle).
	/// \param[in] RawDataTarget A valid (non null) raw data handle of the target (#RawDataHandle).
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	/// The pointer is void because different cameras/sensors with different amount of bytes per pixel
	/// are supported.
	SPECTRALRADAR_API void copyRawData(RawDataHandle RawDataSource, RawDataHandle RawDataTarget);

	/// \fn void* getRawDataPtr(RawDataHandle RawDataSource);
	/// \ingroup Data
	/// \brief Returns the pointer to the raw data content. The pointer might no longer after additional actions using the RawDataHandle. 
	/// \param[in] RawDataSource A valid (non null) raw data handle of the source (#RawDataHandle).
    /// \return A pointer to the memory owned by the handle. The pointer is void because different cameras/sensors
    ///         with different amount of bytes per pixel are supported.
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void* getRawDataPtr(RawDataHandle RawDataSource);

	/// \fn int getRawDataPropertyInt(RawDataHandle RawData, RawDataPropertyInt Property);
	/// \ingroup Data
	/// \brief Returns a raw data property
	/// \param[in] RawData A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] Property The desired property.
    /// \return The value of the desired property.
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API int getRawDataPropertyInt(RawDataHandle RawData, RawDataPropertyInt Property);

	/// \fn void setRawDataBytesPerPixel(RawDataHandle Raw, int BytesPerPixel);
	/// \ingroup Data
	/// \brief Sets the bytes per pixel for raw data.
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] BytesPerPixel The number of bytes per pixel supported by the camera or sensor.
	///
	/// If the raw data are retrieved using getRawData(), this parameter is automatically set to the right value.
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void setRawDataBytesPerPixel(RawDataHandle Raw, int BytesPerPixel);

	/// \fn void reserveRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Reserves the amount of data specified. This might improve performance if appending data to the #RawDataHandle as no additional memory needs to be reserved then. 
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void reserveRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);

	/// \fn void resizeRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);
	/// \ingroup Data
	/// \brief Resizes the specified raw data buffer accordingly.
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] Size1 The desired number of data along the first axis ("z" in the default orientation).
	/// \param[in] Size2 The desired number of data along the second axis ("x" in the default orientation).
	/// \param[in] Size3 The desired number of data along the third axis ("y" in the default orientation).
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void resizeRawData(RawDataHandle Raw, int Size1, int Size2, int Size3);

	/// \fn void setRawDataContent(RawDataHandle RawData, void* NewContent);
	/// \ingroup Data
	/// \brief Sets the content of the raw data buffer. The size of the RawDataHandle needs to be adjusted first, as otherwise not all data might be copied. 
	/// \param[in] RawData A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] NewContent A valid (non null) pointer to a void array with the source data.
	///
	/// The amount of data that will be copied depends on the size(s) that had previously been
	/// setup in the raw data object.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.\n
	/// The pointer is void because different cameras/sensors with different amount of bytes per pixel
	/// are supported.
	SPECTRALRADAR_API void setRawDataContent(RawDataHandle RawData, void* NewContent);

	/// \fn void setScanSpectra(RawDataHandle RawData, int NumberOfScanRegions, int* ScanRegions);
	/// \ingroup Data
	/// \brief Sets the number of the spectra in the raw data that are used for creating A-scan/B-scan data. 
	/// \param[in] RawData A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] NumberOfScanRegions is the number of desired scan regions.
	/// \param[in] ScanRegions is an array containing 2*NumberOfScanRegions elements that delimit the regions.
	///
	/// During the scanning, the light spot travels along a curve, also known as scan pattern. At each of the
	/// points of the curve, spectra are measured. Some spectra are acquired at points where an A-Scans are
	/// desired (the scan region(s)), some others where an Apodization is desired (apodization region(s)),
	/// and some others at less than interesting positions.\n
    /// This function sets the regions where A-Scan computation is desired. The supplied array must contain
    /// an even number of indices. The even indices give the start of a region, and the odd indices give the
    /// end of the regions (actually, one point past-the-end). In other words, the index pair at position
    /// (2n,2n+1) in the array (third argument) gives the n-th region, starting at point ScanRegions[2n] and
    /// ending at (but not including) the point ScanRegions[2*n+1]. Here 0 <= n < NumberOfScanRegions.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.\n
	SPECTRALRADAR_API void setScanSpectra(RawDataHandle RawData, int NumberOfScanRegions, int* ScanRegions);

	/// \fn void setApodizationSpectra(RawDataHandle RawData, int NumberOfApoRegions, int* ApodizationRegions)
	/// \ingroup Data
	/// \brief Sets the number of the spectra in the raw data that contain data useful as apodization spectra. 
	/// \param[in] RawData A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] NumberOfApoRegions is the number of desired apodization regions.
	/// \param[in] ApodizationRegions is an array containing 2*NumberOfApoRegions elements that delimit the regions.
	///
	/// During the scanning, the light spot travels along a curve, also known as scan pattern. At each of the
	/// points of the curve, spectra are measured. Some spectra are acquired at points where an A-Scans are
	/// desired (the scan region(s)), some others where an Apodization is desired (apodization region(s)),
	/// and some others at less than interesting positions.\n
    /// This function sets the regions where apodization spectra will be measured. The supplied array must contain
    /// an even number of indices. The even indices give the start of a region, and the odd indices give the
    /// end of the regions (actually, one point past-the-end). In other words, the index pair at position
    /// (2n,2n+1) in the array (third argument) gives the n-th region, starting at point ApodizationRegions[2n] and
    /// ending at (but not including) the point ApodizationRegions[2*n+1]. Here 0 <= n < NumberOfApoRegions.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void setApodizationSpectra(RawDataHandle RawData, int NumberOfApoRegions, int* ApodizationRegions);

	/// \fn  int getNumberOfScanRegions(RawDataHandle Raw)
	/// \ingroup Data
	/// \brief Returns the number of regions that have been acquired that contain scan data, i. e. spectra that are used to compute A-scans.
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
    /// \return The number of scan regions (each region may contain several scans).
    ///
	/// During the scanning, the light spot travels along a curve, also known as scan pattern. At each of the
	/// points of the curve, spectra are measured. Some spectra are acquired at points where an A-Scans are
	/// desired (the scan region(s)), some others where an Apodization is desired (apodization region(s)),
	/// and some others at less than interesting positions.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API int getNumberOfScanRegions(RawDataHandle Raw);

	/// \fn  int getNumberOfApodizationRegions(RawDataHandle Raw)
	/// \ingroup Data
	/// \brief Returns the number of regions in the raw data containing spectra that are supposed to be used for apodization.
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
    /// \return The number of apodization regions (each region may contain several apodizations).
    ///
	/// During the scanning, the light spot travels along a curve, also known as scan pattern. At each of the
	/// points of the curve, spectra are measured. Some spectra are acquired at points where an A-Scans are
	/// desired (the scan region(s)), some others where an Apodization is desired (apodization region(s)),
	/// and some others at less than interesting positions.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API int getNumberOfApodizationRegions(RawDataHandle Raw);

	/// \fn  void getScanSpectra(RawDataHandle Raw, int* SpectraIndex)
	/// \ingroup Data
	/// \brief Returns the indices of spectra that contain scan data, i. e. spectra that are supposed to be used to compute A-scans. 
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
	/// \param[out] SpectraIndex the array of indices delimiting the scan regions. The size of this array should be twice the number
    /// of scan regions which can be obtained by getNumberOfScanRegions()
	/// 
	/// During the scanning, the light spot travels along a curve, also known as scan pattern. At each of the
	/// points of the curve, spectra are measured. Some spectra are acquired at points where an A-Scans are
	/// desired (the scan region(s)), some others where an Apodization is desired (apodization region(s)),
	/// and some others at less than interesting positions.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void getScanSpectra(RawDataHandle Raw, int* SpectraIndex);

	/// \fn  void getApodizationSpectra(RawDataHandle Raw, int* SpectraIndex)
	/// \ingroup Data
	/// \brief Returns the indices of spectra that contain apodization data, i. e. spectra that are supposed to be used as input for apodization. 
	/// \param[in] Raw A valid (non null) raw data handle (#RawDataHandle).
	/// \param[out] SpectraIndex the array of indices delimiting the apodization regions. The size of this array should be twice the number
    /// of apodization regions which can be obtained by getNumberOfApodizationRegions()
	/// 
	/// During the scanning, the light spot travels along a curve, also known as scan pattern. At each of the
	/// points of the curve, spectra are measured. Some spectra are acquired at points where an A-Scans are
	/// desired (the scan region(s)), some others where an Apodization is desired (apodization region(s)),
	/// and some others at less than interesting positions.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void getApodizationSpectra(RawDataHandle Raw, int* SpectraIndex);

	/// \defgroup DataCreation Data Creation and Clearing
	/// \brief Functions to create and clear object containing data.
	///
	/// \fn RawDataHandle createRawData()
	/// \ingroup DataCreation
	/// \brief Creates a raw data object (#RawDataHandle).
	/// \return A valid raw data handle.
    ///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API RawDataHandle createRawData(void);

	/// \fn void clearRawData(RawDataHandle)
	/// \ingroup DataCreation
	/// \brief Clears a raw data object (#RawDataHandle)
	/// \param[in] Raw A raw data handle. If the handle is a nullptr, this function does nothing.
    ///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void clearRawData(RawDataHandle Raw);

	/// \fn DataHandle createData(void);
	/// \ingroup DataCreation
	/// \brief Creates a 1-dimensional data object, containing floating point data.
	/// \return A valid data handle (#DataHandle).
	SPECTRALRADAR_API DataHandle createData(void);

	/// \fn DataHandle createGradientData(int Size)
	/// \ingroup DataCreation
	/// \brief Creates a 1-dimensional data object, containing floating point data with equidistant arranged values between [0, size-1] with distance 1/(size-1).
	/// \param[in] Size Data number.
	/// \return A valid data handle (#DataHandle).
	SPECTRALRADAR_API DataHandle createGradientData(int Size);

	/// \fn void clearData(DataHandle)
	/// \ingroup DataCreation
	/// \brief Clears the specified #DataHandle object.
	/// \param[in] Data A data handle (#DataHandle). If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void clearData(DataHandle Data);

	/// \fn  ColoredDataHandle createColoredData(void)
	/// \ingroup DataCreation
	/// \brief Creates a colored data object (#ColoredDataHandle).
	/// \return A valid colored data handle (#ColoredDataHandle).
	SPECTRALRADAR_API ColoredDataHandle createColoredData(void);

	/// \fn void clearColoredData(ColoredDataHandle Volume)
	/// \ingroup DataCreation
	/// \brief Clears a colored volume object. 
	/// \param[in] Volume A colored data handle (#ColoredDataHandle).  If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void clearColoredData(ColoredDataHandle Volume);

	/// \fn ComplexDataHandle createComplexData(void)
	/// \ingroup DataCreation
	/// \brief Creates a data object holding complex data (#ComplexDataHandle). 
	/// \return A valid complex data handle (#ComplexDataHandle).
	SPECTRALRADAR_API ComplexDataHandle createComplexData(void);

	/// \fn void clearComplexData(ComplexDataHandle Data)
	/// \ingroup DataCreation
	/// \brief Clears a data object holding complex data (#ComplexDataHandle).
	/// \param[in] Data A complex data handle (#ComplexDataHandle). If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void clearComplexData(ComplexDataHandle Data);

	/// \defgroup Hardware Hardware 
	/// \brief Functions providing direct access to OCT Hardware functionality.
	///
	/// \enum DevicePropertyFloat
	/// \ingroup Hardware
	/// \brief Floating point properties of the device that can be retrieved with the function #getDevicePropertyFloat.
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
		/// The approximate spectral width of the spectrometer
		Device_SpectralWidth_nm, // FIXME Spectrometer bandwidth
		/// Maximal valid trigger frequency depending on the chosen camera preset.
		Device_MaxTriggerFrequency_Hz,
		// Expected line rate depending on the chosen camera preset.
		Device_LineRate_Hz
		
	} DevicePropertyFloat;

	/// \enum DevicePropertyInt
	/// \ingroup Hardware
	/// \brief Integer properties of the device that can be retrieved with the function #getDevicePropertyInt
	typedef enum {
		/// The number of pixels provided by the spectrometer.
		Device_SpectrumElements,
		/// The number of bytes one element of the spectrum occupies.
		Device_BytesPerElement,
		/// The maximum number of scans per dimension in the live volume rendering mode.
		Device_MaxLiveVolumeRenderingScans,
		/// Bit depth of the DAQ.
		Device_BitDepth,
		/// Number of spectrometer cameras
		Device_NumOfCameras,
		/// Revision number of the device
		Device_RevisionNumber,
		/// Number of analog input channels available (may be zero)
		Device_NumOfAnalogInputChannels,
	} DevicePropertyInt;

	/// \enum DevicePropertyString
	/// \ingroup Hardware
	/// \brief String-properties of the device that can be retrieved with the function #getDevicePropertyString.
	typedef enum {
		/// The type name of the device.
		Device_Type,
		/// The series of the device
		Device_Series,
		/// Serial number of the device.
		Device_SerialNumber,
		/// Hardware Config of the currently used device.
		Device_HardwareConfig,
	} DevicePropertyString;

	/// \enum DeviceFlag
	/// \ingroup Hardware
	/// \brief Boolean properties of the device that can be retrieved with the function #getDeviceFlag.
	typedef enum {
		/// The type name of the device.
		Device_On,
		/// Specifies if there is a video camera available
		Device_CameraAvailable,
		/// Specifies if there is a SLD available
		Device_SLDAvailable,
		/// Status of the SLD, either on (true) or off (false)
		Device_SLDStatus,
		/// Status of the laser diode, either on (true) or off (false)
		/// \warning Not all devices are equiped 
		Device_LaserDiodeStatus,
		/// Parameter for the overlay of the video camera which shows the scan pattern in red.
		Device_CameraShowScanPattern,
		/// Gives information whether a probe controller (with buttons) is available
		Device_ProbeControllerAvailable,
		/// Flag indicating if the data is signed 
		Device_DataIsSigned,
		/// Flag indicating whether system is a swept source (or spectral domain) system. 
		Device_IsSweptSource,
		/// Flag indicating if the system offers analog input capabilities
		Device_AnalogInputAvailable,
	} DeviceFlag;

	/// \enum StaticDeviceFlag
	/// \ingroup Hardware
	/// \brief Boolean properties of the device that can be queried before the device is initialized. Retrieved with the function #getStaticDeviceFlag.
	typedef enum {
		/// Read-only flag indicating if the system can be turned on/off using #setStaticDeviceFlag with the flag #Device_PowerOn
		Device_PowerControlAvailable,
		/// Write-only flag to enable or disable the device power supply.
		Device_PowerOn,
	} StaticDeviceFlag;

	/// \enum ScanAxis
	/// \ingroup Hardware
	/// \brief Axis selection for the function #moveScanner.
	typedef enum {
		/// X-Axis of the scanner
		ScanAxis_X = 0,
		/// Y-Axis of the scanner
		ScanAxis_Y = 1
	} ScanAxis;

	/// \enum DeviceState
	/// \ingroup Hardware
	/// \brief Results for function #getDeviceState.
	typedef enum {
		/// Device is not available (e.g. not connected, no power)
		DeviceState_Unavailable = 0,
		/// Device is available, but in standby mode (can be activated via software)
		DeviceState_Standby = 1,
		/// Device is enabled
		DeviceState_Enabled = 2,
		/// No configuration files found and could not be restored from device
		DeviceState_NoConfiguration = 3,
	} DeviceState;

	/// \fn OCTDeviceHandle initDevice(void);
	/// \ingroup Hardware
	/// \brief Initializes the installed device.
	/// \return Handle to the initialized OCT device (#OCTDeviceHandle).
	///
	/// This function attempts to discover the hardware specified in the file SpectralRadar.ini. The components
	/// of the hardware are represented on the software side by plugins. The discovering process will log its
	/// activity. As some of the messages may appear to be error messages to the untrained eye, it is recommended to
	/// invoke the function #getError to check if this function actually succeeded.
	SPECTRALRADAR_API OCTDeviceHandle initDevice(void);

	/// \fn int getDevicePropertyInt(OCTDeviceHandle Dev, DevicePropertyInt Selection)
	/// \ingroup Hardware
	/// \brief Returns properties of the device belonging to the specfied #OCTDeviceHandle.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
    /// \param[in] Selection The desired property.
    /// \return The value of the desired property.
	SPECTRALRADAR_API int getDevicePropertyInt(OCTDeviceHandle Dev, DevicePropertyInt Selection);

	/// \fn const char* getDevicePropertyString(OCTDeviceHandle Dev, DevicePropertyString Selection)
	/// \ingroup Hardware
	/// \brief Returns properties of the device belonging to the specfied #OCTDeviceHandle.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
    /// \param[in] Selection The desired property.
    /// \return The value of the desired property. This memory pointed belongs to SpectralRadar.dll and
    ///         the user should not attempt to free it.
	SPECTRALRADAR_API const char* getDevicePropertyString(OCTDeviceHandle Dev, DevicePropertyString Selection);

	/// \fn double getDevicePropertyFloat(OCTDeviceHandle Dev, DevicePropertyFloat)
	/// \ingroup Hardware
	/// \brief Returns properties of the device belonging to the specfied #OCTDeviceHandle.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
    /// \param[in] Selection The desired property.
    /// \return The value of the desired property.
	SPECTRALRADAR_API double getDevicePropertyFloat(OCTDeviceHandle Dev, DevicePropertyFloat Selection);

	/// \fn BOOL getDeviceFlag(OCTDeviceHandle Dev, DeviceFlag Selection)
	/// \ingroup Hardware
	/// \brief Returns properties of the device belonging to the specified #OCTDeviceHandle.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
    /// \param[in] Selection The desired flag.
    /// \return The value of the desired flag.
	SPECTRALRADAR_API BOOL getDeviceFlag(OCTDeviceHandle Dev, DeviceFlag Selection);

	/// \fn void setDeviceFlag(OCTDeviceHandle Dev, DeviceFlag Selection, BOOL Value)
	/// \ingroup Hardware
	/// \brief Sets the selcted flag of the device belonging to the specified #OCTDeviceHandle.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
    /// \param[in] Selection The desired flag.
    /// \param[in] Value The value of the desired flag.
	SPECTRALRADAR_API void setDeviceFlag(OCTDeviceHandle Dev, DeviceFlag Selection, BOOL Value);

	/// \fn BOOL getStaticDeviceFlag(StaticDeviceFlag Selection)
	/// \ingroup Hardware
	/// \brief Returns properties of the device available before device initialization
	/// \param[in] Selection The desired flag.
	/// \return The value of the desired flag.
	SPECTRALRADAR_API BOOL getStaticDeviceFlag(StaticDeviceFlag Selection);

	/// \fn void setStaticDeviceFlag(StaticDeviceFlag Selection, BOOL Value)
	/// \ingroup Hardware
	/// \brief Sets the selcted flag of the device  available before device initialization.
	/// \param[in] Selection The desired flag.
	/// \param[in] Value The value of the desired flag.
	SPECTRALRADAR_API void setStaticDeviceFlag(StaticDeviceFlag Selection, BOOL Value);

	/// \fn void closeDevice(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Closes the device opened previously with initDevice.
	/// \param[in] Dev An OCT device handle (#OCTDeviceHandle). If the handle is a nullptr, this function does nothing.
	///                In most cases, this handle will have been previously generated with the function #initDevice.
	SPECTRALRADAR_API void closeDevice(OCTDeviceHandle Dev);

	/// \fn void moveScanner(OCTDeviceHandle Dev, ProbeHandle Probe, ScanAxis Axis, double Position_mm)
	/// \ingroup Hardware
	/// \brief Manually moves the scanner to a given position
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Probe A handle to the probe (#ProbeHandle), whose galvo position is to be set.
	/// \param[in] Axis the axis in which you want to set the position manually
	/// \param[in] Position_mm the actual position in mm you want to move the galvo to.
	SPECTRALRADAR_API void moveScanner(OCTDeviceHandle Dev, ProbeHandle Probe, ScanAxis Axis, double Position_mm);

	/// \fn void moveScannerToApoPosition(OCTDeviceHandle Dev, ProbeHandle Probe)
	/// \ingroup Hardware
	/// \brief Moves the scanner to the apodization position
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Probe A handle to the probe (#ProbeHandle); whose galvo position is to be set.
	SPECTRALRADAR_API void moveScannerToApoPosition(OCTDeviceHandle Dev, ProbeHandle Probe);

	/// \fn int getNumberOfDevicePresetCategories(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief If the hardware supports multiple presets, the funciton returns the number of categories in which presets can be set. 
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category). 
	SPECTRALRADAR_API int getNumberOfDevicePresetCategories(OCTDeviceHandle Dev);

	/// \fn const char* getDevicePresetCategoryName(OCTDeviceHandle Dev, int Category)
	/// \ingroup Hardware
	/// \brief Gets a descriptor/name for the respective preset category
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Category An index describing the preset category in the range between 0 and the number of preset categories minus 1,
	///                     given by #getNumberOfDevicePresetCategories
	/// \return The name of the requested device preset category.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category). 
	SPECTRALRADAR_API const char* getDevicePresetCategoryName(OCTDeviceHandle Dev, int Category);

	/// \fn const char* getDevicePresetCategoryIndex(OCTDeviceHandle Dev, const char* Name)
	/// \ingroup Hardware
	/// \brief Gets the index of a preset category from the name of the category.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Name The name of the device preset category.
	/// \return An index describing the preset category in the range between 0 and the number of preset categories minus 1,
	///                     given by #getNumberOfDevicePresetCategories.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category). 
	SPECTRALRADAR_API int getDevicePresetCategoryIndex(OCTDeviceHandle Dev, const char* Name);

	/// \fn void setDevicePreset(OCTDeviceHandle Dev, int Category, ProbeHandle Probe, ProcessingHandle Proc, int Preset)
	/// \ingroup Hardware
	/// \brief Sets the preset of the device. Using presets the sensitivity and acquisition speed of the device can be influenced.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Category An index describing the preset category in the range between 0 and the number of preset categories minus 1,
	///                     given by #getNumberOfDevicePresetCategories
	/// \param[in] Probe A handle to the probe (#ProbeHandle); whose galvo position is to be set.
	/// \param[in] Proc A valid (non null) processing handle.
	/// \param[in] Preset The index of the preset.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category). 
	SPECTRALRADAR_API void setDevicePreset(OCTDeviceHandle Dev, int Category, ProbeHandle Probe, ProcessingHandle Proc, int Preset);

	/// \fn int getDevicePreset(OCTDeviceHandle Dev, int Category);
	/// \ingroup Hardware
	/// \brief Gets the currently used device preset.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Category An index describing the preset category in the range between 0 and the number of preset categories minus 1,
	///                     given by #getNumberOfDevicePresetCategories
    /// \return The current device preset index.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category).\n
	/// A description of the device preset associated with a particular index can be obtained by invoking the function
	/// #getDevicePresetDescription. The total number of presets for the active device can be retrieved with the function
	/// #getNumberOfDevicePresets.
	SPECTRALRADAR_API int getDevicePreset(OCTDeviceHandle Dev, int Category);

	/// \fn const char* getDevicePresetDescription(OCTDeviceHandle Dev, int Category, int Preset);
	/// \ingroup Hardware
	/// \brief Returns a description of the selected device preset. Using the description more information about sensitivity and acquisition speed of the respective set can be found.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Category An index describing the preset category in the range between 0 and the number of preset categories minus 1, 
	///                     given by #getNumberOfDevicePresetCategories.
	/// \param[in] Preset The index of the preset.
    /// \return A text describing the preset (speed, sensitivity). This pointer refers to memory owned by SpectralRadar.dll.
    ///         The er should not attempt to free it.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category).\n
	/// The current device preset can be obtained by invoking the function #getDevicePreset. The total number of presets for the active device
	/// can be retrieved with the function #getNumberOfDevicePresets.
	SPECTRALRADAR_API const char* getDevicePresetDescription(OCTDeviceHandle Dev, int Category, int Preset);

	/// \fn int getNumberOfDevicePresets(OCTDeviceHandle Dev, int Category);
	/// \ingroup Hardware
	/// \brief Returns the number of available device presets.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Category An index describing the preset category in the range between 0 and the number of preset categories minus 1, 
	///                     given by #getNumberOfDevicePresetCategories.
    /// \return The number of presets supported by the OCT device.
	///
	/// Different devices support different preset categories (gain, speed, etc). When getting or setting a preset, the right
	/// category must be provided. To get the number of supported categories, use the function #getNumberOfDevicePresetCategories.
	/// To get a name (i.e. a description of the category), use the function #getDevicePresetCategoryName. To get the index of
	/// a supported category, provided you know the name, use the function #getDevicePresetCategoryIndex (this is the index
	/// need when getting or setting a preset of a given category).\n
	/// The current device preset can be obtained by invoking the function #getDevicePreset. A description of the device preset associated
	/// with a particular index can be obtained by invoking the function #getDevicePresetDescription.
	SPECTRALRADAR_API int getNumberOfDevicePresets(OCTDeviceHandle Dev, int Category);

	/// \fn void setRequiredSLDOnTime_s(int Time_s)
	/// \ingroup Hardware
	/// \brief Sets the time the SLD needs to be switched on before any measurement can be started. Default is 3 seconds.
	/// \param[in] Time_s Minimum required on time in seconds.
	SPECTRALRADAR_API void setRequiredSLDOnTime_s(int Time_s);

	/// \fn void resetCamera(void);
	/// \ingroup Hardware
	/// \brief Resets the spectrometer camera.
	SPECTRALRADAR_API void resetCamera(void);

	/// \fn BOOL isDeviceAvailable(void)
	/// \ingroup Hardware
	/// \brief Returns whethter any supported Base-Unit is available.
	/// 
	/// This function attemps to communicate with the device, and returns TRUE if a minimum of working functionality
	/// can be guaranted, FALSE otherwise. This function can be invoked as many times as desired (e.g. in a polling
	/// strategy) without side effects.
	SPECTRALRADAR_API BOOL isDeviceAvailable(void);

	/// \fn DeviceState getDeviceState(void)
	/// \ingroup Hardware
	/// \brief Returns the state of supported base-unit.
	/// 
	/// This function attemps to communicate with the device, and returns the state of the base-unit if a minimum of working functionality
	/// can be guaranteed. If no device can be found or communication fails, this function will return #DeviceState_Unavailable.
	/// This function can be invoked as many times as desired (e.g. in a polling strategy) without side effects.
	SPECTRALRADAR_API DeviceState getDeviceState(void);

	/// \defgroup Internal Internal Values
	/// \brief Functions for access to all kinds of Digital-to-Analog and Analog-to-Digital on the device.
	///
	/// \fn int getNumberOfInternalDeviceValues(OCTDeviceHandle Dev);
	/// \ingroup Internal
	/// \brief Returns the number of Analog-to-Digital Converter present in the device.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
    /// \return The number of Analog-to-Digital Converter present in the device.
	SPECTRALRADAR_API int getNumberOfInternalDeviceValues(OCTDeviceHandle Dev);

	/// \fn void getInternalDeviceValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);
	/// \ingroup Internal
	/// \brief Returns names and unit for the specified Analog-to-Digital Converter. 
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Index The index of the internal value whose name and unit are sought.
	/// \param[out] Name Name of the internal device value. If this pointer is null, it will not be used.
	///             If it is non-null, it must point to a memory are at least as large as \p NameStringSize bytes.
	/// \param[in] NameStringSize The maximal number of bytes that will be copied onto the array holding the name.
	/// \param[out] Unit Unit of the internal device value. If this pointer is null, it will not be used.
	///             If it is non-null, it must point to a memory are at least as large as \p UnitStringSize bytes.
	/// \param[in] UnitStringSize The maximal number of bytes that will be copied onto the array holding the unit.
	/// 
	/// The index is a running number, starting with 0, smaller than the number specified by #getNumberOfInternalDeviceValues.
	SPECTRALRADAR_API void getInternalDeviceValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);

	/// \fn double getInternalDeviceValueByName(OCTDeviceHandle, const char*);
	/// \ingroup Internal 
	/// \brief Returns the value of the specified Analog-to-Digital Converter (ADC);. 
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Name Name of the internal device value.
	/// \return The internal device value.
	///
	/// The ADC is specified by the name returned by #getInternalDeviceValueName.
	SPECTRALRADAR_API double getInternalDeviceValueByName(OCTDeviceHandle Dev, const char* Name);

	/// \fn double getInternalDeviceValueByIndex(OCTDeviceHandle Dev, int Index);
	/// \ingroup Internal
	/// \brief Returns the value of the selected ADC. 
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Index The index of the internal device value.
	/// \return The internal device value.
	/// 
	/// The index is a running integer number, starting with 0, smaller than the number specified by #getNumberOfInternalDeviceValues.
	SPECTRALRADAR_API double getInternalDeviceValueByIndex(OCTDeviceHandle Dev, int Index);

	/// \fn void setInternalDeviceValueByIndex(OCTDeviceHandle Dev, int Index, double Value);
	/// \ingroup Internal
	/// \brief Sets the value of the selected ADC. 
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Index The index of the internal device value.
	/// \param[in] Value The internal device value.
	/// 
	/// The index is running number, starting with 0, smaller than the number specified by #getNumberOfInternalDeviceValues.
	SPECTRALRADAR_API void setInternalDeviceValueByIndex(OCTDeviceHandle Dev, int Index, double Value);

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
		/// Maximum scan range in X direction
		Probe_RangeMaxX,
		/// Maximum scan range in Y direction
		Probe_RangeMaxY,
		/// Maximum galvo slope (accounting for the distortion capabilities of different galvo types)
		Probe_MaximumSlope_XY,
		/// Speckle size to be used for scan pattern computation if speckle reduction is switched on.
		Probe_SpeckleSize,
		/// X-voltage used to acquire the apodization spectrum
		Probe_ApoVoltageX,
		/// Y-voltage used to acquire the apodization spectrum
		Probe_ApoVoltageY,
		/// Offset for reference stage marking the zero delay line
		Probe_ReferenceStageOffset,
		/// Optical path length, in millimeter (fiber length up to the scanner, multiplied by the refractive index)
		Probe_FiberOpticalPathLength_mm,
		/// Optical path length, in millimeter (from scanner input to objective mount, multiplied by the refractive index)
		Probe_ProbeOpticalPathLength_mm,
		/// Optical path length, in millimeter (without couning the focal length, multiplied by the equivalent refractive index)
		Probe_ObjectiveOpticalPathLength_mm,
		/// Optical focal length, in millimeter
		Probe_ObjectiveFocalLength_mm,
	} ProbeParameterFloat;

	/// \enum ProbeParameterString
	/// \ingroup Probe
	/// \brief Parameters describing the composition of the probe. These properties refer to a probe that has
	///        already been created and for which a valid #ProbeHandle has been obtained.
	typedef enum {
		/// The filename. Just Probe.ini, or some other name.
		Probe_Name,
		/// Serial number of the probe.
		Probe_SerialNumber,
		/// Name of the probe. From this name it is possible to find out the probe definition file. A version
		/// suffix (e.g. "_V2") might be part of it. The termination ".prdf" is not part of the name.
		Probe_Description,
		/// Objective from the probe.  From this string it is possible to find out the objective definition file.
		/// A version suffix (e.g. "_V2") might be part of it. The termination ".odf" is not part of the name.
		Probe_Objective
	} ProbeParameterString;
	

	/// \enum ProbeParameterInt
	/// \ingroup Probe
	/// \brief Parameters describing the behaviour of the Probe, such as calibration factors and scan parameters.
	typedef enum {
		/// The number of cycles used for apodization.
		Probe_ApodizationCycles,
		/// A factor used as oversampling.
		Probe_Oversampling,
		/// A factor used as oversampling of the slow scanner axis.
		Probe_Oversampling_SlowAxis,
		/// Number of speckles that are scanned over for averaging. Requires Oversampling >= SpeckleReduction
		Probe_SpeckleReduction
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
		Probe_HasMEMSScanner,
		/// Boolean if the apodization position is used for the x-mirror (or x-axis) only or both axes (mirrors) are used. This parameter can be set via the Probe.ini or prdf-file only.
		Probe_ApoOnlyX
	} ProbeFlag;

	/// \fn ProbeHandle initCurrentProbe(OCTDeviceHandle Dev);
	/// \ingroup Probe
	/// \brief Initializes the probe that is currently selected in the GUI. If no probe is configured, an error is raised.
	/// \param[in] Dev The #OCTDeviceHandle that was initially provided by #initDevice. Can be NULL in case no device is initialized or available. 
	/// \return A valid probe handle (#ProbeHandle).
	SPECTRALRADAR_API ProbeHandle initCurrentProbe(OCTDeviceHandle Dev);

	/// \fn ProbeHandle initProbe(OCTDeviceHandle Dev, const char* ProbeFile);
	/// \ingroup Probe
	/// \brief Initializes a probe specified by ProbeFile.
	/// \param[in] Dev The #OCTDeviceHandle that was initially provided by #initDevice. Can be NULL in case no device is initialized or available. 
	/// \param[in] ProbeFile The filename of the .ini. If the path is not given, it will be assumed that
	///            this file is in the configuration directory (typically C:\\Program Files\\Thorlabs\\SpectralRadar\\Config).
	///            To indicate that file is in the current working directory, prepend a "~\\" before the name.
	///            If a termination ".ini" is not there, it will be appended.
	/// \return A valid probe handle (#ProbeHandle).
	///
	/// In older systems up until a manufacturing date of May 2011 either "Handheld" or "Microscope" are used. An according .ini file 
	/// (i. e. "Handheld.ini" or "Microscope.ini) will be loaded from the config path of the SpectralRadar installation containing all necessary information.
	/// With systems manufactured after May 2011 "Probe" should be used.
	SPECTRALRADAR_API ProbeHandle initProbe(OCTDeviceHandle Dev, const char* ProbeFile);

	/// \fn ProbeHandle initDefaultProbe(OCTDeviceHandle Dev, const char* Type, const char* Objective)
	/// \ingroup Probe
	/// \brief Creates a standard probe using standard parameters for the specified probe type.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Type A zero terminated string with the probe type name (one of \p Standard_OCTG, \p UserCustomizable_OCTP, \p Handheld_OCTH).
	/// \param[in] Objective A zero terminated string with the objective name (e.g. \p "LSM03").
	/// \return A valid probe handle (#ProbeHandle).
	SPECTRALRADAR_API ProbeHandle initDefaultProbe(OCTDeviceHandle Dev, const char* Type, const char* Objective);

	/// \fn ProbeHandle initProbeFromOCTFile(OCTDeviceHandle Dev, OCTFileHandle File)
	/// \ingroup Probe
	/// \brief Creates a probe using the parameters from the specified OCT file.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] File A valid (non null) handle of an OCT file.
	/// \return A valid probe handle (#ProbeHandle).
	SPECTRALRADAR_API ProbeHandle initProbeFromOCTFile(OCTDeviceHandle Dev, OCTFileHandle File);

	/// \fn void saveProbe(ProbeHandle Probe, const char* ProbeFile);
	/// \ingroup Probe
	/// \brief Saves the current properties of the #ProbeHandle to a specified INI file to be reloaded using the #initProbe() function.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] ProbeFile The filename of the .ini. If the path is not given, it will be assumed that
	///            this file should go in the configuration directory (typically C:\\Program Files\\Thorlabs\\SpectralRadar\\Config).
	///            To indicate that file is in the current working directory, prepend a "~\\" before the name.
	///            If a termination ".ini" is not there, it will be appended.
	SPECTRALRADAR_API void saveProbe(ProbeHandle Probe, const char* ProbeFile);

	/// \fn void setProbeParameterInt(ProbeHandle Probe, ProbeParameterInt Selection, int Value)
	/// \ingroup Probe
	/// \brief Sets integer parameter of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired parameter.
	/// \param[in] Value The new value for the parameter.
	SPECTRALRADAR_API void setProbeParameterInt(ProbeHandle Probe, ProbeParameterInt Selection, int Value);

	/// \fn void setProbeParameterFloat(ProbeHandle Probe, ProbeParameterFloat Selection, double Value)
	/// \ingroup Probe
	/// \brief Sets floating point parameters of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired parameter.
	/// \param[in] Value The new value for the parameter.
	SPECTRALRADAR_API void setProbeParameterFloat(ProbeHandle Probe, ProbeParameterFloat Selection, double Value);

	/// \fn int getProbeParameterInt(ProbeHandle Probe, ProbeParameterInt Selection)
	/// \ingroup Probe
	/// \brief Gets integer parameters of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired parameter.
	/// \return The current value of the parameter.
	SPECTRALRADAR_API int getProbeParameterInt(ProbeHandle Probe, ProbeParameterInt Selection);

	/// \fn double getProbeParameterFloat(ProbeHandle Probe, ProbeParameterFloat Selection)
	/// \ingroup Probe
	/// \brief Gets floating point parameters of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired parameter.
	/// \return The current value of the parameter.
	SPECTRALRADAR_API double getProbeParameterFloat(ProbeHandle Probe, ProbeParameterFloat Selection);

	/// \fn BOOL getProbeFlag(ProbeHandle Probe, ProbeFlag Selection)
	/// \ingroup Probe
	/// \brief Returns the selected boolean value of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired flag.
	/// \return The current value of the flag.
	SPECTRALRADAR_API BOOL getProbeFlag(ProbeHandle Probe, ProbeFlag Selection);

	/// \fn void setProbeParameterString(ProbeHandle Probe, ProbeParameterString Selection, const char* Value)
	/// \ingroup Probe
	/// \brief Sets a string property of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired parameter.
	/// \param[in] Value The desired value for the parameter.
	SPECTRALRADAR_API void setProbeParameterString(ProbeHandle Probe, ProbeParameterString Selection, const char* Value);

	/// \fn const char* getProbeParameterString(ProbeHandle Probe, ProbeParameterString Selection)
	/// \ingroup Probe
	/// \brief Gets the desired string property of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Selection The desired parameter.
	/// \return The current value of the parameter. The pointer referes to memory owned by SpectralRadar.dll.
	///         The user should not attempt to free it.
	SPECTRALRADAR_API const char* getProbeParameterString(ProbeHandle Probe, ProbeParameterString Selection);

	/// \fn const char* getProbeType(ProbeHandle Probe)
	/// \ingroup Probe
	/// \brief Gets the type of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
    /// \return The current type name (one of \p Standard_OCTG, \p UserCustomizable_OCTP, \p Handheld_OCTH).
	SPECTRALRADAR_API const char* getProbeType(ProbeHandle Probe);

	/// \fn void setProbeType(ProbeHandle Probe, const char* Type);
	/// \ingroup Probe
	/// \brief Sets the type of the specified probe.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Type A zero terminated string describing the probe type (one of \p Standard_OCTG, \p UserCustomizable_OCTP, \p Handheld_OCTH).
	SPECTRALRADAR_API void setProbeType(ProbeHandle Probe, const char* Type);

	/// \fn void closeProbe(ProbeHandle Probe);
	/// \ingroup Probe
	/// \brief Closes the probe and frees all memory associated with it.
	/// \param[in] Probe A handle of a probe (#ProbeHandle). If the handle is a nullptr, this function does nothing.
	///                  In most cases this handle will have been previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	SPECTRALRADAR_API void closeProbe(ProbeHandle Probe);

	/// \enum ScanPatternAcquisitionOrder
	/// \ingroup ScanPattern
	/// \brief Parameters describing the behaviour of the scan pattern. 
	typedef enum ScanPatternAcquisitionOrder_ {
		/// The scan pattern will be acquired slice by slice which means that the function #getRawData() needs to be called more than
		/// once to get the data for the whole scan pattern
		ScanPattern_AcqOrderFrameByFrame,
		/// The scan patten will be acquired in one piece
		ScanPattern_AcqOrderAll
	} ScanPatternAcquisitionOrder;

	/// \enum ScanPatternApodizationType
	/// \ingroup ScanPattern
	/// \brief Parameters describing how often the apodization spectra will be acquired. 
	/// If you want to create a scan pattern without an apodization please use (#setProbeParameterInt) and (#Probe_ApodizationCycles)
	/// to set the size of apodization to zero.
	typedef enum ScanPatternApodizationType_ {
		/// The volume scan pattern will be acquired with one apodization for the whole pattern.
		ScanPattern_ApoOneForAll,
		/// The volume scan pattern will be acquired with one apodization before each B-scan which results in a slightly better image
		/// quality but longer acquisition time.
		ScanPattern_ApoEachBScan
	} ScanPatternApodizationType;

	/// \enum InflationMethod
	/// \ingroup ScanPattern
	/// \brief Describes how to use a 2D freeform scan pattern to create a 3D scan pattern.
	typedef enum InflationMethod_{
		/// Inflates the points to the outer normal direction 
		Inflation_NormalDirection
	} InflationMethod;

	/// \fn void CameraPixelToPosition(ProbeHandle Probe, ColoredDataHandle Image, int PixelX, int PixelY, double* PosX, double* PosY);
	/// \ingroup Probe
	/// \brief Computes the physical position of a camera pixel of the video camera in the probe.
	///        It assumes a properly calibrated device.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Image A valid (non null) handle of colored data.
	/// \param[in] PixelX The x-pixel coordinate.
	/// \param[in] PixelY The y-pixel coordinate.
	/// \param[out] PosX The x coordinate. If this pointer happens to be null, it will not be used.
	/// \param[out] PosY The y coordinate. If this pointer happens to be null, it will not be used.
	SPECTRALRADAR_API void CameraPixelToPosition(ProbeHandle Probe, ColoredDataHandle Image, int PixelX, int PixelY, double* PosX, double* PosY);

	/// \fn void PositionToCameraPixel(ProbeHandle Probe, ColoredDataHandle Image, double PosX, double PosY, int* PixelX, int* PixelY);
	/// \ingroup Probe
	/// \brief Computes the pixel of the video camera corresponding to a physical position. It needs to be assured that the device is properly calibrated.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Image A valid (non null) handle of colored data.
	/// \param[in] PosX The x coordinate.
	/// \param[in] PosY The y coordinate.
	/// \param[out] PixelX The x-pixel coordinate. If this pointer happens to be null, it will not be used.
	/// \param[out] PixelY The y-pixel coordinate. If this pointer happens to be null, it will not be used.
	SPECTRALRADAR_API void PositionToCameraPixel(ProbeHandle Probe, ColoredDataHandle Image, double PosX, double PosY, int* PixelX, int* PixelY);

	/// \fn void visualizeScanPatternOnDevice(OCTDeviceHandle Dev, ProbeHandle Probe, ScanPatternHandle Pattern, BOOL ShowRawPattern);
	/// \ingroup Probe
	/// \brief Visualizes the scan pattern on top of the camera image; if appropriate hardware is used for visualization.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[in] ShowRawPattern Indicates whether the scan should shown (TRUE) or hidden (FALSE).
	SPECTRALRADAR_API void visualizeScanPatternOnDevice(OCTDeviceHandle Dev, ProbeHandle Probe, ScanPatternHandle Pattern, BOOL ShowRawPattern);

	/// \fn void visualizeScanPatternOnImage(ProbeHandle Probe, ScanPatternHandle ScanPattern, ColoredDataHandle VideoImage);
	/// \ingroup Probe
	/// \brief Visualizes the scan pattern on top of the camera image; scan pattern data is written into the image.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] ScanPattern A valid (non null) handle of a scan pattern.
	/// \param[in] VideoImage A valid (non null) handle of colored data.
	SPECTRALRADAR_API void visualizeScanPatternOnImage(ProbeHandle Probe, ScanPatternHandle ScanPattern, ColoredDataHandle VideoImage);

	/// \defgroup ScanPattern Scan Pattern
	/// \brief Functions that describe the movement of the Scanner during measurement.
	///
	/// \fn ScanPatternHandle createNoScanPattern(ProbeHandle Probe, int AScans, int NumberOfScans);
	/// \ingroup ScanPattern
	/// \brief Creates a simple scan pattern that does not move the galvo. Use this pattern for point scans and/or non-scanning probes.
	///        The pattern will however use a specified amount of trigger signals. For continuous acquisition use NumberOfScans set to 1. 
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] AScans The number of A-Scans that will be measured in each part of this #ScanPatternHandle.
	/// \param[in] NumberOfScans The number of parts in this #ScanPatternHandle. It should be "1" for continuous acquisition.
	/// \return A valid (non null) handle to a scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createNoScanPattern(ProbeHandle Probe, int AScans, int NumberOfScans);

	/// \fn ScanPatternHandle createAScanPattern(ProbeHandle Probe, int AScans, double PosX_mm, double PosY_mm);
	/// \ingroup Scanpattern
	/// \brief Creates a scan pattern used to acquire a specific amount of Ascans at a specific position. 
	/// \param[in] Probe A valid (non null) handle of a probe.
	/// \param[in] AScans The number of A-Scans that will be measured.
	/// \param[in] PosX_mm The position of the light spot, in millimeter.
	/// \param[in] PosY_mm The position of the light spot, in millimeter.
	/// \return A valid (non null) handle to a scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createAScanPattern(ProbeHandle Probe, int AScans, double PosX_mm, double PosY_mm);
	
	/// \fn ScanPatternHandle createBScanPattern(ProbeHandle Probe, double Range_mm, int AScans);
	/// \ingroup ScanPattern
	/// \brief Creates a horizontal rectilinear-segment B-scan pattern that moves the galvo over a specified range.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Range_mm The extension of the horizontal segment, expressed in mm, centered at (0,0).
	/// \param[in] AScans The number of A-Scans that will be measured along the segment.
	/// \return A valid (non null) handle to a scan pattern.
	///
	/// If a different center position is desired, one of the functions #shiftScanPattern(), #shiftScanPatternEx() should
	/// be invoked afterwards, passing the scan pattern handle returned by this function.\n
	/// If a different orientation is desired (i.e other than horizontal), one of the functions
	/// #rotateScanPattern(ScanPatternHandle(), #rotateScanPatternEx(ScanPatternHandle() should
	/// be invoked afterwards, passing the scan pattern handle returned by this function.
	SPECTRALRADAR_API ScanPatternHandle createBScanPattern(ProbeHandle Probe, double Range_mm, int AScans);

	/// \fn ScanPatternHandle createBScanPatternManual(ProbeHandle Probe, double StartX_mm, double StartY_mm, double StopX_mm, double StopY_mm, int AScans);
	/// \ingroup ScanPattern
	/// \brief Creates a B-scan pattern specified by start and end points. 
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] StartX_mm The x-coordinate of the start point, in mm.
	/// \param[in] StartY_mm The y-coordinate of the start point, in mm.
	/// \param[in] StopX_mm The x-coordinate of the stop point, in mm.
	/// \param[in] StopY_mm The y-coordinate of the stop point, in mm.
	/// \param[in] AScans The number of A-Scans that will be measured along the segment.
	/// \return A valid (non null) handle to a scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createBScanPatternManual(ProbeHandle Probe, double StartX_mm, double StartY_mm, double StopX_mm, double StopY_mm, int AScans);

	/// \fn ScanPatternHandle createIdealBScanPattern(ProbeHandle Probe, double Range_mm, int AScans);
	/// \ingroup ScanPattern
	/// \brief Creates an ideal B-scan pattern assuming scanners with infinite speed. No correction factors are taken into account.
	/// This is only used for internal purposes and not as a scan pattern designed to be output to the galvo drivers.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Range_mm The extension of the segment, expressed in mm, centered at the current position.
	/// \param[in] AScans The number of A-Scans that will be measured along the segment.
	/// \return A valid (non null) handle to a scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createIdealBScanPattern(ProbeHandle Probe, double Range_mm, int AScans);
	
	/// \fn ScanPatternHandle createCirclePattern(ProbeHandle Probe, double Radius_mm, int AScans);
	/// \ingroup ScanPattern
	/// \brief Creates a circle scan pattern. 
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] Radius_mm The radius of the circle pattern.
	/// \param[in] AScans The number of A-Scans that will be measured along the segment.
	/// \warning Circle patterns cannot be rotated properly. 
	/// \return A valid (non null) handle to a scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createCirclePattern(ProbeHandle Probe, double Radius_mm, int AScans);
	
	/// \fn ScanPatternHandle createVolumePattern(ProbeHandle Probe, double RangeX_mm, int SizeX, double RangeY_mm, int SizeY, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);
	/// \ingroup ScanPattern
	/// \brief Creates a simple volume pattern.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] RangeX_mm The extension of the volume along the x-axis, expressed in mm, centered at the current position.
	/// \param[in] SizeX The number of planes that cross the x-axis..
	/// \param[in] RangeY_mm The extension of the volume along the y-axis, expressed in mm, centered at the current position.
	/// \param[in] SizeY The number of planes that cross the y-axis.
	/// \param[in] ApoType The apodization type decides whether one apodization suffices for the whole set of measurements in the volume,
	///            or one apodization will be measured for each B-Scan (each segment).
	/// \param[in] AcqOrder Dictates the acquisition strategy, as explained below, which reflects the way the user wants to
	///            retrieve the acquired data.
	/// \return A valid (non null) handle to a scan pattern.
	///
	/// A volume scan pattern is actually a stack of B-scan patterns. At creation time the stack fills a parallelepiped volume in space
	/// but the shape can be subsequently modified if the individual slices are rotated, translated, or both (see explanation of functions
	/// #rotateScanPatternEx(), #shiftScanPatternEx() for more information). Notice that the individual B-scans (the slices) will always
	/// contain the segment of the laser beam that iluminates the sample (rotations and translations cannot change that).\n
	/// This functions creates a parallelepiped volume scan pattern and in the default orientation (first axis is the depth "z", second
	/// axis is "x", third axis is "y") the slices will be accomodated along the "y" axis. Hence, the number of slices in the stack is
	/// given by the parameter \p SizeY. Afterwards, this parameter may be retrieved by invoking the function #getScanPatternPropertyInt()
	/// with the argument #ScanPattern_Cycles.\n
	/// Depending on the setting for #ScanPatternApodizationType, there will be either one apodization for the entire volume
	/// (#ScanPattern_ApoOneForAll) or a single apodization for each B-scan (#ScanPattern_ApoEachBScan).\n
	/// The volume pattern with ScanPatternAcquisitionOrder set to ScanPattern_AcqOrderAll  consists of a single uninterrupted scan
	/// and all data is acquired in a single measurement. The complete volume will be returned in one raw data (#RawDataHandle) by calling
	/// #getRawData().\n
	/// Otherwise (i.e. if #ScanPatternAcquisitionOrder is set to #ScanPattern_AcqOrderFrameByFrame) the scan pattern consists of individual
	/// B-Scan measurements that get retrieved separately through separate invokations of #getRawData(). In other words: The structure of
	/// the final dataset will be identical to the former case, but the stack will be returned slice-by-slice by calling #getRawData(), once for
	/// each slice.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API ScanPatternHandle createVolumePattern(ProbeHandle Probe, double RangeX_mm, int SizeX, double RangeY_mm, int SizeY, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);


	/// \fn ScanPatternHandle createVolumePatternEx(ProbeHandle Probe, double RangeX_mm, int SizeX, double RangeY_mm, int SizeY, double CenterX_mm, double CenterY_mm, double Angle_rad, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);
	/// \ingroup ScanPattern
	/// \brief Creates a simple volume pattern.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  #initDefaultProbe, or #initProbeFromOCTFile.
	/// \param[in] RangeX_mm The extension of the volume along the x-axis, expressed in mm, centered at the current position.
	/// \param[in] SizeX The number of planes that cross the x-axis.
	/// \param[in] RangeY_mm The extension of the volume along the y-axis, expressed in mm, centered at the current position.
	/// \param[in] SizeY The number of planes that cross the y-axis.
	/// \param[in] CenterX_mm Center of the volume pattern
	/// \param[in] CenterY_mm Center of the volume pattern
	/// \param[in] Angle_rad Rotation in radians of the entire scan pattern
	/// \param[in] ApoType The apodization type decides whether one apodization suffices for the whole set of measurements in the volume,
	///            or one apodization will be measured for each B-Scan (each segment).
	/// \param[in] AcqOrder Dictates the acquisition strategy, as explained below, which reflects the way the user wants to
	///            retrieve the acquired data.
	/// \return A valid (non null) handle to a scan pattern.
	///
	/// A volume scan pattern is actually a stack of B-scan patterns. At creation time the stack fills a parallelepiped volume in space
	/// but the shape can be subsequently modified if the individual slices are rotated, translated, or both (see explanation of functions
	/// #rotateScanPatternEx(), #shiftScanPatternEx() for more information). Notice that the individual B-scans (the slices) will always
	/// contain the segment of the laser beam that iluminates the sample (rotations and translations cannot change that).\n
	/// This functions creates a parallelepiped volume scan pattern and in the default orientation (first axis is the depth "z", second
	/// axis is "x", third axis is "y") the slices will be accomodated along the "y" axis. Hence, the number of slices in the stack is
	/// given by the parameter \p SizeY. Afterwards, this parameter may be retrieved by invoking the function #getScanPatternPropertyInt()
	/// with the argument #ScanPattern_Cycles.\n
	/// Depending on the setting for #ScanPatternApodizationType, there will be either one apodization for the entire volume
	/// (#ScanPattern_ApoOneForAll) or a single apodization for each B-scan (#ScanPattern_ApoEachBScan).\n
	/// The volume pattern with ScanPatternAcquisitionOrder set to ScanPattern_AcqOrderAll  consists of a single uninterrupted scan
	/// and all data is acquired in a single measurement. The complete volume will be returned in one raw data (#RawDataHandle) by calling
	/// #getRawData().\n
	/// Otherwise (i.e. if #ScanPatternAcquisitionOrder is set to #ScanPattern_AcqOrderFrameByFrame) the scan pattern consists of individual
	/// B-Scan measurements that get retrieved separately through separate invokations of #getRawData(). In other words: The structure of
	/// the final dataset will be identical to the former case, but the stack will be returned slice-by-slice by calling #getRawData(), once for
	/// each slice.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API ScanPatternHandle createVolumePatternEx(ProbeHandle Probe, double RangeX_mm, int SizeX, double RangeY_mm, int SizeY, double CenterX_mm, double CenterY_mm, double Angle_rad, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);
	
	/// \fn void updateScanPattern(ScanPatternHandle Pattern)
	/// \ingroup ScanPattern
	/// \brief Updates the specfied pattern (#ScanPatternHandle) and computes the full look-up-table.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	SPECTRALRADAR_API void updateScanPattern(ScanPatternHandle Pattern);

	/// \fn void rotateScanPattern(ScanPatternHandle, double Angle_rad);
	/// \ingroup ScanPattern
	/// \brief Rotates the specfied pattern (#ScanPatternHandle), counter-clockwise. The rotation is relative to current angle,
	///        not to the horizontal. That is, after multiple invokations of this function the final rotation is
	///        the addition of all rotations.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[in] Angle_rad The angle (expressed in radians) of the rotation. 
	SPECTRALRADAR_API void rotateScanPattern(ScanPatternHandle Pattern, double Angle_rad);

	/// \fn void rotateScanPatternEx(ScanPatternHandle, double Angle_rad, int Index);
	/// \ingroup ScanPattern
	/// \brief Counter-clockwise rotates the scan \p Index (0-based, i.e. zero for the first, one for the second, and so on) of the
	///        specfied volume scan pattern (#ScanPatternHandle). The rotation is relative to current angle, not to the horizontal.
	///        That is, after multiple invokations of this function the final rotation is the addition of all rotations.
	/// \param[in] Pattern A valid (non null) handle of a volume scan pattern.
	/// \param[in] Angle_rad The angle (expressed in radians) of the counter-clockwise rotation.
	/// \param[in] Index The slice of the stack that should be rotated.
	///
	/// This function is specific of volume scan patterns, although only a slice of it will be rotated.
	/// A volume scan pattern is actually a stack of B-scan patterns. In the default orientation (first axis is the depth "z",
	/// second axis is "x", third axis is "y"), the slices will be accomodated along the "y" axis. The number of slices in the
	/// stack may be retrieved by invoking the function #getScanPatternPropertyInt() with the argument #ScanPattern_Cycles.
	SPECTRALRADAR_API void rotateScanPatternEx(ScanPatternHandle Pattern, double Angle_rad, int Index);

	/// \fn void shiftScanPattern(ScanPatternHandle, double ShiftX, double ShiftY)
	/// \ingroup ScanPattern
	/// \brief Shifts the specified pattern (#ScanPatternHandle). The shift is relative to current position,
	///        not to (0,0). That is, after multiple invokations of this function the final shift is
	///        the addition of all shifts.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[in] ShiftX The relative shift in the x-axis direction, expressed in mm.
	/// \param[in] ShiftY The relative shift in the y-axis direction, expressed in mm.
	SPECTRALRADAR_API void shiftScanPattern(ScanPatternHandle Pattern, double ShiftX_mm, double ShiftY_mm);

	/// \fn void shiftScanPatternEx(ScanPatternHandle, double ShiftX_mm, double ShiftY_mm, BOOL ShiftApo, int Index)
	/// \ingroup ScanPattern
	/// \brief Shifts the scan \p Index (0-based, i.e. zero for the first, one for the second, and so on) of the specified
	///        volume pattern (#ScanPatternHandle). The shift is relative to current position, not to (0,0). That is, after
	///        multiple invokations of this function the final shift is the addition of all shifts.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[in] ShiftX_mm The relative shift in the x-axis direction, expressed in mm.
	/// \param[in] ShiftY_mm The relative shift in the y-axis direction, expressed in mm.
	/// \param[in] ShiftApo TRUE if the apodization should also be shifted. FALSE otherwise.
	/// \param[in] Index The slice of the stack that should be shifted.
	///
	/// This function is specific of volume scan patterns, although only a slice of it will be shifted.
	/// A volume scan pattern is actually a stack of B-scan patterns. In the default orientation (first axis is the depth "z",
	/// second axis is "x", third axis is "y"), the slices will be accomodated along the "y" axis. The number of slices in the
	/// stack may be retrieved by invoking the function #getScanPatternPropertyInt() with the argument #ScanPattern_Cycles.
	SPECTRALRADAR_API void shiftScanPatternEx(ScanPatternHandle Pattern, double ShiftX_mm, double ShiftY_mm, BOOL ShiftApo, int Index);

	/// \fn void zoomScanPattern(ScanPatternHandle Pattern, double Factor)
	/// \ingroup ScanPattern
	/// \brief Zooms the specified pattern (#ScanPatternHandle) around the optical center that coincides with the center of the camera image and the physical coordinates (0 mm,0 mm). 
	///		The apodization position will not be modified.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[in] Factor The zoom factor.
	SPECTRALRADAR_API void zoomScanPattern(ScanPatternHandle Pattern, double Factor);

	/// \fn int getScanPatternLUTSize(ScanPatternHandle Pattern);
	/// \ingroup ScanPattern
	/// \brief Returns the number of points in the specified scan pattern (#ScanPatternHandle), including apodization and flyback.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \return The size of the look-up-table.
	///
	/// The look-up-table mentioned here is a table with the voltages that will be sent to the galvos. It is computed aforehand.
	SPECTRALRADAR_API int getScanPatternLUTSize(ScanPatternHandle Pattern);

	/// \fn void getScanPatternLUT(ScanPatternHandle Pattern, double* VoltsX, double* VoltsY);
	/// \ingroup ScanPattern
	/// \brief Returns the voltages that will be applied to reach the positions to be scanned, in the specified scan pattern (#ScanPatternHandle).
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[out] VoltsX A pointer to the array in which the voltage for the X-positions will be written. If a nullptr is passed,
	///                    nothing will be written. Otherwise it should have space for at least the size returned by #getScanPatternLUTSize().
	/// \param[out] VoltsY A pointer to the array in which the voltage for the Y-positions will be written. If a nullptr is passed,
	///                    nothing will be written. Otherwise it should have space for at least the size returned by #getScanPatternLUTSize().
	///
	/// The look-up-table mentioned here is a table with the voltages that will be sent to the galvos. It is computed aforehand.
	SPECTRALRADAR_API void getScanPatternLUT(ScanPatternHandle Pattern, double* VoltX, double* VoltY);

	/// \fn int getScanPointsSize(ScanPatternHandle Pattern);
	/// \ingroup ScanPattern
	/// \brief Returns the number of points in the specified scan pattern (#ScanPatternHandle), including apodization and flyback.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \return The number of points in the scan pattern, including apodization and flyback.
	SPECTRALRADAR_API int getScanPointsSize(ScanPatternHandle Pattern);

	/// \fn int getScanPoints(ScanPatternHandle Pattern, double* PosX_mm, double* PosY_mm);
	/// \ingroup ScanPattern
	/// \brief Returns the position coordinates (in mm) of the points that in the specified scan pattern (#ScanPatternHandle).
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	/// \param[out] PosX_mm A pointer to the array in which the X-positions (in mm) will be written. If a nullptr is passed,
	///                     nothing will be written. Otherwise it should have space for at least the size returned by #getScanPointsSize().
	/// \param[out] PosY_mm A pointer to the array in which the Y-positions (in mm) will be written. If a nullptr is passed,
	///                     nothing will be written. Otherwise it should have space for at least the size returned by #getScanPointsSize().
	SPECTRALRADAR_API void getScanPoints(ScanPatternHandle Pattern, double* PosX_mm, double* PosY_mm);

	/// \fn void clearScanPattern(ScanPatternHandle Pattern);
	/// \ingroup ScanPattern
	/// \brief Clears the specified scan pattern (#ScanPatternHandle).
	/// \param[in] Pattern A handle of a scan pattern (#ScanPatternHandle). If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void clearScanPattern(ScanPatternHandle Pattern);

	/// \defgroup Math Mathematical manipulations
	/// \brief Functions for pure mathematical manipulations (i.e. no physics involved).

	/// \enum InterpolationMethod
	/// \ingroup Math
	/// \brief Selects the interpolation method.
	typedef enum InterpolationMethod_{
		/// Linear interpolation.
		Interpolation_Linear,
		/// Cubic B-Spline interpolation.
		Interpolation_Spline,
	} InterpolationMethod;

	/// \enum BoundaryCondition
	/// \ingroup Math
	/// \brief Selects the boundary conditions for the interpolation.
	typedef enum BoundaryCondition_{
		/// Matches the slope of the interpolated function at starting/end point to the following/previous points.
		BoundaryCondition_Standard,
		/// Natural boundary considtions used for interpolation which means the interpolated spline will turn into a straight line at the start/end.
		BoundaryCondition_Natural,
		/// Peridoc boundary conditions used for interpolation which means that the interpolated function will interpret the points as a closed loop 
		/// and use therefore the points from the start/end for interpolation of the end/start.
		BoundaryCondition_Periodic,
	} BoundaryCondition;

	/// \enum ScanPointsDataFormat
	/// \ingroup ScanPattern
	/// \brief Selects format with the functions #loadScanPointsFromFile or #saveScanPointsToFile to import or export data points.
	typedef enum ScanPointsDataFormat_{
		/// Data format txt
		ScanPoints_DataFormat_TXT,
		/// Data format raw/srm pair
		ScanPoints_DataFormat_RAWandSRM,
	} ScanPointsDataFormat;

	/// \fn ScanPatternHandle createFreeformScanPattern2D(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int Size, int AScans, InterpolationMethod InterpolationMethod, BOOL CloseScanPattern);
	/// \ingroup ScanPattern
	/// \brief Creates a B-scan scan pattern of arbitrary form with equidistant sampled scan points.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  or #initDefaultProbe.
	/// \param[in] PosX_mm A pointer to the double array of x-positions (in mm) of the scan pattern with length \p Size
	/// \param[in] PosY_mm A pointer to the double array of y-positions (in mm) of the scan pattern with length \p Size
	/// \param[in] Size The length of the arrays \p PosX_mm and \p PosY_mm.
	/// \param[in] AScans The number of A-scans in the scan pattern that will be created. The number of A-scans should be greater than Size.
	/// \param[in] InterpolationMethod The interpolation method used to fill up the specified points by \p PosX_mm and \p PosY_mm to create a pattern
	///                                with evenly spaced sampled points.
	/// \param[in] CloseScanPattern Specifies whether the scan pattern should be closed (TRUE) or not (FALSE). Closing the scan pattern will lead to
	///                             the same start and end point of each B-scan.
	SPECTRALRADAR_API ScanPatternHandle createFreeformScanPattern2D(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int Size, int AScans,
		InterpolationMethod InterpolationMethod, BOOL CloseScanPattern);
	
	/// \fn ScanPatternHandle createFreeformScanPattern2DFromLUT(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int Size, BOOL ClosedScanPattern);
	/// \ingroup ScanPattern
	/// \brief Creates a B-scan scan pattern of arbitrary form with the specified scan points. 
	///        The voltages array is taken as-is, so care must be taken to use sensible values with regard to the capabilities of the utilized scanner
	///        system and to the resolution of the system resp. the desired resolution of your scan pattern.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  or #initDefaultProbe.
	/// \param[in] PosX_mm A pointer to the double array of X-positions (in mm) of the scan pattern with length \p Size
	/// \param[in] PosY_mm A pointer to the double array of Y-positions (in mm) of the scan pattern with length \p Size
	/// \param[in] Size The length of the arrays PositionsX and PositionsY.
	/// \param[in] ClosedScanPattern Specifies whether the scan pattern should be closed (TRUE) or not (FALSE). Closing the scan pattern will lead to
	///                             the same start and end point of each B-scan.
	///
	/// With this function the definition of every single scan point is required. In order to create a scan pattern specifying only some "edge" points of the pattern,
	/// please consider #createFreeformScanPattern2D.
	SPECTRALRADAR_API ScanPatternHandle createFreeformScanPattern2DFromLUT(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int Size, BOOL ClosedScanPattern);
	
	/// \fn ScanPatternHandle createFreeformScanPattern3DFromLUT(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int AScansPerBScan, int NumberOfBScans, BOOL ClosedScanPattern, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);
	/// \ingroup ScanPattern
	/// \brief Creates a volume scan pattern of arbitrary form with the specified scan voltages. 
	/// The voltages array is taken as-is, so care must be taken to use sensible values with regard to the capabilities of the utilized scanner
	/// system and to the resolution of the system resp. the desired resolution of your scan pattern.
	/// With this function the definition of each single scan point is required.  In order to create a scan pattern specifying only the end coordinates,
	/// please consider #createFreeformScanPattern3D.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  or #initDefaultProbe.
	/// \param[in] PosX_mm A pointer to the array of X-positions (in mm) of the scan pattern whose length is the product of \p AScansPerBScan and \p NumberOfBScans.
	/// \param[in] PosY_mm A pointer to the array of Y-positions (in mm) of the scan pattern whose length is the product of \p AScansPerBScan and \p NumberOfBScans.
	/// \param[in] AScansPerBScan The desired number of A-scans in each B-scan of the volume pattern. All B-scans will have the same size.
	/// \param[in] NumberOfBScans The desired number of B-scans in the volume pattern. 
	/// \param[in] ClosedScanPattern Specifies whether the scan pattern should be closed or not.
	///                             Closing the scan pattern will lead to the same start and end point of each B-scan.
	/// \param[in] ApoType The specified method used for apodization in a volume pattern. \sa #ScanPatternApodizationType.
	/// \param[in] AcqOrder The specified method used for the acquisition order in a volume pattern. \sa #ScanPatternAcquisitionOrder.
	/// \return A scan pattern handle containing the created 3D-freeform scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createFreeformScanPattern3DFromLUT(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int AScansPerBScan, int NumberOfBScans,
		BOOL ClosedScanPattern, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);
	
	/// \fn ScanPatternHandle createFreeformScanPattern3D(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int* ScanIndices, int Size, int NumberOfAScansPerBScan, InterpolationMethod InterpolationMethod, BOOL CloseScanPattern, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder)
	/// \ingroup ScanPattern
	/// \brief Creates a volume scan pattern of arbitrary form with equidistant sampled scan points.
	/// \param[in] Probe A valid (non null) handle of a probe (#ProbeHandle), previously generated by one of the functions #initProbe,
	///                  or #initDefaultProbe.
	/// \param[in] PosX_mm The pointer to the array of x-positions of the scan pattern with length Size.
	/// \param[in] PosY_mm The pointer to the array of y-positions of the scan pattern with length Size.
	/// \param[in] ScanIndices The array specifies the assignment of each point to its B-scan. It needs to have the length Size.
	///                        The entries need to go from 0 to number of (B-scans - 1). The number of B-scans is defined with the entries
	///                        of \p ScanIndices. For example, if the minimum entry is 0 (cannot be negative!) and the maximum entry is 2,
	///                        there will be three B-scans in the pattern.
	/// \param[in] Size The length of the arrays \p PosX_mm, \p PosY_mm, and \p ScanIndices.
	/// \param[in] NumberOfAScansPerBScan The number of A-scans in each B-scan of the created scan pattern. The number of B-scans
	///                                   will be defined with the entries in the \p ScanIndices.
	/// \param[in] InterpolationMethod The interpolation method used to fill up the specified points by \p PositionsX and \p PositionsY
	///                                to create a pattern with evenly-spaced sampled points.
	/// \param[in] CloseScanPattern Specifies whether the scan pattern should be closed or not. Closing the scan pattern means that each
	///                             B-scan starts and stops at the same point.
	/// \param[in] ApoType The specified method used for apodization in a volume pattern. Please see
	///                                       #ScanPatternApodizationType for more information.
	/// \param[in] AcqOrder The specified method used for the acquisition order in a volume pattern. Please see
	///                                        #ScanPatternAcquisitionOrder for more information.
	/// \return A scan pattern handle containing the created 3D-freeform scan pattern.
	SPECTRALRADAR_API ScanPatternHandle createFreeformScanPattern3D(ProbeHandle Probe, double* PosX_mm, double* PosY_mm, int* ScanIndices, int Size, int NumberOfAScansPerBScan,
		InterpolationMethod InterpolationMethod, BOOL CloseScanPattern, ScanPatternApodizationType ApoType, ScanPatternAcquisitionOrder AcqOrder);
	
	/// \fn void interpolatePoints2D(double* OrigPosX, double* OrigPosY, int Size, double* InterpPosX, double* InterpPosY, int NewSize, InterpolationMethod InterpolationMet, BoundaryCondition BoundaryCond)
	/// \ingroup Math
	/// \brief Interpolates the imaginary curve defined by the given sequence of points with the specified #InterpolationMethod.
	///        The coordinates are abstract and this funcion has no sideffects that could affect any physical property.
	///        The original and the interpolated coordinates have a meaning for the user, but no consequence for SpectralRadar.
	/// \param[in] OrigPosX A pointer to the array of x-coords with length \p Size.
	/// \param[in] OrigPosY A pointer to the array of y-coords with length \p Size.
	/// \param[in] Size The length of the arrays \p PositionsX, \p PositionsY and \p ScanIndices.
	/// \param[out] InterpPosX A pointer to the array of x-coords whose length should be \p NewSize.
	/// \param[out] InterpPosY A pointer to the array of y-coords whose length should be \p NewSize.
	/// \param[in] NewSize The number of interpolated points.
	/// \param[in] InterpolationMet The desired #InterpolationMethod.
	/// \param[in] BoundaryCond The desired #BoundaryCondition.
	SPECTRALRADAR_API void interpolatePoints2D(double* OrigPosX, double* OrigPosY, int Size, double* InterpPosX, double* InterpPosY, int NewSize,
		InterpolationMethod InterpolationMet, BoundaryCondition BoundaryCond);
	
	/// \fn void inflatePoints(double* PosX, double* PosY, int Size, double* InflatedPosX, double* InflatedPosY, int NumberOfInflationLines, double RangeOfInflation, InflationMethod Method)
	/// \ingroup Math
	/// \brief Inflates the provided curve in space with the specified #InflationMethod. 
	/// It can be used to create scan patterns of arbitrary forms with #createFreeformScanPattern3DFromLUT if the used positions correspond to coordinates of the valid scan field in mm.
	/// \param[in] PosX The pointer to the double array of x-positions of the scan pattern with length \p Size.
	/// \param[in] PosY The pointer to the double array of y-positions of the scan pattern with length \p Size.
	/// \param[in] Size The length of the arrays PositionsX, PositionsY and ScanIndices.
	/// \param[out] InflatedPosX The pointer to the double array of x-positions of the scan pattern with length Size * NumberOfInflationLines
	/// \param[out] InflatedPosY The pointer to the double array of y-positions of the scan pattern with length Size * NumberOfInflationLines
	/// \param[in] NumberOfInflationLines The number of inflation lines. Please note that the length of the arrays InflatedPointsX and InflatedPointsY need to match.
	/// \param[in] RangeOfInflation The range of inflation which results in the width of the created data object.
	/// \param[in] Method The specified #InflationMethod.
	SPECTRALRADAR_API void inflatePoints(double* PosX, double* PosY, int Size, double* InflatedPosX, double* InflatedPosY, int NumberOfInflationLines,
		double RangeOfInflation, InflationMethod Method);

	/// \fn void saveScanPointsToFile(double* ScanPosX_mm, double* ScanPosY_mm, int* ScanIndices, int Size, const char* Filename, ScanPointsDataFormat DataFormat)
	/// \ingroup ScanPattern
	/// \brief Saves the scan points and scan indices to a file with the specified #ScanPointsDataFormat.
	/// \param[in] ScanPosX_mm The pointer to the double array of x-positions of the scan pattern with length Size in mm
	/// \param[in] ScanPosY_mm The pointer to the double array of y-positions of the scan pattern with length Size in mm
	/// \param[in] ScanIndices The array specifies the assignment of each point to its B-scan. It needs to have the length Size with entries from 0 to number of (B-scans - 1). 
	/// The number of B-scans is defined with the entries of \p ScanIndices. To save scan points for a 2D-pattern set all entries to zero.   
	/// \param[in] Size The length of the arrays PositionsX, PositionsY and ScanIndices.
	/// \param[in] Filename Path and name of the file containing the scan points and indices.
	/// \param[in] DataFormat The specified #ScanPointsDataFormat.
	SPECTRALRADAR_API void saveScanPointsToFile(double* ScanPosX_mm, double* ScanPosY_mm, int* ScanIndices, int Size, const char* Filename, ScanPointsDataFormat DataFormat);
	
	/// \fn int getSizeOfScanPointsFromFile(const char* Filename, ScanPointsDataFormat DataFormat)
	/// \ingroup ScanPattern
	/// \brief Returns the number of scan points in the specified file.
	/// \param[in] Filename (including path) of the file that contains the scan points and indices.
	/// \param[in] DataFormat The desired #ScanPointsDataFormat. 
	/// \return The number of scan points in the give file.
	SPECTRALRADAR_API int getSizeOfScanPointsFromFile(const char* Filename, ScanPointsDataFormat DataFormat);
	
	/// \fn void loadScanPointsFromFile(double* ScanPosX_mm, double* ScanPosY_mm, int* ScanIndices, int Size, const char* Filename, ScanPointsDataFormat DataFormat)
	/// \ingroup ScanPattern
	/// \brief Copies the scan points and scan indices from the file to the provided arrays.
	/// \param[out] ScanPosX_mm The pointer to the double array of x-positions of the scan pattern with length Size in mm
	/// \param[out] ScanPosY_mm The pointer to the double array of y-positions of the scan pattern with length Size in mm
	/// \param[out] ScanIndices The array specifies the assignment of each point to its B-scan. It has the length Size with entries from 0 to number of (B-scans - 1). 
	/// The number of B-scans is defined with the entries of \p ScanIndices. To save scan points for a 2D-pattern set all entries to zero.   
	/// \param[in] Size The length of the arrays PositionsX, PositionsY and ScanIndices.
	/// \param[in] Filename Path and name of the file containing the scan points and indices.
	/// \param[in] DataFormat The selected #ScanPointsDataFormat.
	SPECTRALRADAR_API void loadScanPointsFromFile(double* ScanPosX_mm, double* ScanPosY_mm, int* ScanIndices, int Size, const char* Filename, ScanPointsDataFormat DataFormat);

	/// \fn int getSizeOfScanPointsFromDataHandle(DataHandle ScanPoints)
	/// \ingroup ScanPattern
	/// \brief Returns the size of the scan points and scan indices in the #DataHandle.
	/// \param[in] ScanPoints The #DataHandle containing the provided points and scan indices.
	/// \return The number of scan points.
	///
	/// Notice that in this case a data structure is used to hold data other than spectra or A-scans. 
	SPECTRALRADAR_API int getSizeOfScanPointsFromDataHandle(DataHandle ScanPoints);
	
	/// \fn void getScanPointsFromDataHandle(DataHandle ScanPoints, double* PosX_mm, double* PosY_mm, int* ScanIndices, int Length)
	/// \ingroup ScanPattern
	/// \brief Copies the scan points and scan indices from the #DataHandle to the provided arrays.
	/// \param[in] ScanPoints The created #DataHandle containing the provided points and scan indices.
	/// \param[out] PosX_mm The pointer to the array of X-coords of the scan pattern, with length \p Size
	/// \param[out] PosY_mm The pointer to the array of Y-coords of the scan pattern, with length \p Size
	/// \param[out] ScanIndices The array specifies the assignment of each point to its B-scan, with length \p Size. The entries will go from 0 to number of B-scans - 1).
	/// The number of B-scans is defined with the entries of \p ScanIndices. To save scan points for a 2D-pattern set all entries to zero.  
	/// \param[in] Length The length of the arrays \p FreeformCoordsX, \p FreeformCoordsY, and \p ScanIndices.
	SPECTRALRADAR_API void getScanPointsFromDataHandle(DataHandle ScanPoints, double* PosX_mm, double* PosY_mm, int* ScanIndices, int Length);
	
	/// \fn DataHandle createDataHandleFromScanPoints(double* PosX_mm, double* PosY_mm, int* ScanIndices, int Length)
	/// \ingroup ScanPattern
	/// \brief Creates a #DataHandle from the specified scan points and corresponding indices.
	/// \param[in] PosX_mm A pointer to the array of X-coords of the scan pattern, with length \p Size
	/// \param[in] PosY_mm A pointer to the array of Y-coords of the scan pattern, with length \p Size
	/// \param[in] ScanIndices The array specifies the assignment of each point to its B-scan, with length \p Size. The entries need to go from 0 to number of B-scans - 1) 
	/// The number of B-scans is defined with the entries of \p ScanIndices. To save scan points for a 2D-pattern set all entries to zero.  
	/// \param[in] Length The length of the arrays \p FreeFromCoordsX, \p FreeformCoordsY, and \p ScanIndices.
	/// \return A #DataHandle containing the scan points and indices.
	SPECTRALRADAR_API DataHandle createDataHandleFromScanPoints(double* PosX_mm, double* PosY_mm, int* ScanIndices, int Length);

	/// \defgroup Acquisition Acquisition
	/// \brief Functions for acquisition.
	///
	/// \enum AcquisitionType 
	/// \ingroup Acquisition
	/// \brief Determines the kind of acquisition process. The type of acquisition process affects e.g. whether consecutive B-scans are acquired or if it is possible to lose some data.
	typedef enum AcquisitionType_ {
		/// Specifies an asynchronous infinite/continuous measurement. 
		/// With this acquisition type an infinite loop to acquire the specified scan pattern will be started and stopped with the call of #stopMeasurement. 
		/// Several buffers will be created internally to hold the data of the specified scan pattern several times. 
		/// With this acquisiton mode it is possible to lose data if the acquisition is faster than the copying from the framegrabber with #getRawData.
		/// If you lose data you will always lose a whole frame, e.g. a whole B-scan. 
		/// The acquisiton thread runs independently from the thread for grabbing the data to acquire the data as fast as possible.
		/// To get the information whether the data of a whole scan pattern got lost please use #getRawDataPropertyInt with #RawData_LostFrames when grabbing the data.
		Acquisition_AsyncContinuous,
		/// Specifies an asynchronous finite measurement. With this acquisitions type enough memory is created internally to hold the data for the whole scan pattern once. 
		/// Therefore it is guaranteed to grab all the data and not losing frames. Please note that it is possible to acquire the scan pattern once only with this acquisition mode.
		Acquisition_AsyncFinite,
		/// Specfies a synchronous measurement. With this acquisition mode the acquisition of the specified scan pattern will be started with the call of #getRawData.
		/// You can interpret this acquisition type as a software trigger to start the measurement. 
		/// To start the data acquisition externally please see the chapter in the software manual about external triggering.
		Acquisition_Sync
	} AcquisitionType;

	/// \fn size_t projectMemoryRequirement(OCTDeviceHandle Handle, ScanPatternHandle Pattern, AcquisitionType type)
	/// \ingroup Acquisition
	/// \brief Returns the size of the required memory, e.g. for a raw data object, in bytes to acquire the scan pattern once. 
	SPECTRALRADAR_API size_t projectMemoryRequirement(OCTDeviceHandle Handle, ScanPatternHandle Pattern, AcquisitionType type);

	/// \fn void startMeasurement(OCTDeviceHandle Dev, ScanPatternHandle Pattern, AcquisitionType Type)
	/// \ingroup Acquisition
	/// \brief starts a continuous measurement BScans. 
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Pattern A valid (non null) scan pattern handle (#ScanPatternHandle).
	/// \param[in] Type This parameter (#AcquisitionType) decides whether the acquisition proceeds asynchronic (continuous or finite) or
	///            synchronic.
	///
	/// Scanning proceeds according to the specified scan pattern handle. In order to retrieve the acquired data, refer to the
	/// #getRawData() function. To stop the measuring process, invoke #stopMeasurement().\n
	/// Synchronic measurements get triggered when the user invokes function that retrieves the data. Asynchronic measurements proceed
	/// in background, and the retrieving function returns the last available buffer that has been filled with fresh data. Asynchronic
	/// measurements can acquire a pre-specified number of buffers (finite) or continue indefinetely (continuous). If it is not possible
	/// to retrieve acquired data for a while, intermediate buffers might be skipped.
	SPECTRALRADAR_API void startMeasurement(OCTDeviceHandle Dev, ScanPatternHandle Pattern, AcquisitionType Type);

	/// \fn void getRawData(OCTDeviceHandle Dev, RawDataHandle RawData);
	/// \ingroup Acquisition
	/// \brief Acquires data and stores the data unprocessed.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] RawData A valid (non null) raw data handle (#RawDataHandle).
    ///
	/// In case of a synchronic measurement, this function will trigger the data acquisition. Otherwise it will return the latest acquired
	/// data buffer. In any case, this function will block until a data buffer is available (asynchronic measurements may satistfy this
	/// requirement immediately if a previously acquired buffer has not already been consumed).\n
	/// This function is equivalent to \code{.cpp}getRawDataEx(Dev, RawData, 0);\endcode. In other words, in systems with more than just
	/// one camera, this function retrieves the raw data of the first camera.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void getRawData(OCTDeviceHandle Dev, RawDataHandle RawData);

	/// \fn void getRawDataEx(OCTDeviceHandle Dev, RawDataHandle RawData, int CameraIdx);
	/// \ingroup Acquisition
	/// \brief Acquires data with the specific camera given with camera index and stores the data unprocessed. \warning{Unless the program
	///        divides the acquistion in different threads, this function should be invoked first for the master camera (\p CameraIdx = 0)
	///        and only then for the slaves. Otherwise it will block for ever.}
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] RawData A valid (non null) raw data handle (#RawDataHandle).
	/// \param[in] CameraIdx The camera index (0-based, i.e. zero for the first = master, one for the second, and so on).
    ///
	/// In case of a synchronic measurement, this function will trigger the data acquisition. Otherwise it will return the latest acquired
	/// data buffer. In any case, this function will block until a data buffer is available (asynchronic measurements may satistfy this
	/// requirement immediately if a previously acquired buffer has not already been consumed).\n
	/// In systems with more than one camera, the hardware connections ensure that all cameras measure simultaneoulsy. That is, they have 
	/// a common trigger. The master camera (index 0) will actually trigger the measurement of all slaves. for this reason, this function should
	/// be invoked first for the master (index 0) and only afterwards for the slaves (index greater than 0). If a slave triggers first, it will
	/// wait for the master (that is, this function call will block the current execution thread). If the master triggers first, the buffer
	/// for the slave will be ready for pick up by the time the slave retrieves (without blocking).\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void getRawDataEx(OCTDeviceHandle Dev, RawDataHandle RawData, int CameraIdx);

	/// \fn void getAnalogInputData(OCTDeviceHandle Dev, DataHandle Output);
	/// \ingroup Acquisition
	/// \brief Acquires analog data and stores the result floating point voltages. Data from apodisation and flyback regions will 
	///		   be discarded.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Output A valid (non null) data handle (#DataHandle).
	///
	/// #getRawData needs to be called before each call to getAnalogInputData
	SPECTRALRADAR_API void getAnalogInputData(OCTDeviceHandle Dev, DataHandle Output);

	/// \fn void getAnalogInputDataEx(OCTDeviceHandle Dev, DataHandle Data, DataHandle ApoData);
	/// \ingroup Acquisition
	/// \brief Acquires analog data and stores the result floating point voltages. Data from flyback regions will be discarded,
	///		   data from apodisation regions will be stored in ApoData.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Data A valid (non null) data handle (#DataHandle).
	/// \param[in] ApoData A valid (non null) data handle (#DataHandle).
	///
	/// #getRawData needs to be called before each call to getAnalogInputData
	SPECTRALRADAR_API void getAnalogInputDataEx(OCTDeviceHandle Dev, DataHandle Data, DataHandle ApoData);

	/// \fn void setAnalogInputChannelEnabled(OCTDeviceHandle Dev, int ChannelIndex, bool Enable);
	/// \ingroup Acquisition
	/// \brief Enables or disables an analog input channel. Use #getDevicePropertyInt with flag Device_NumOfAnalogInputChannels to determine the number of available channels.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] ChannelIndex Index of the analog input channel to activate or deactivate
	/// \param[in] Enable True to enable channel, false to disable
	///
	SPECTRALRADAR_API void setAnalogInputChannelEnabled(OCTDeviceHandle Dev, int ChannelIndex, bool Enable);

	/// \fn bool getAnalogInputChannelEnabled(OCTDeviceHandle Dev, int ChannelIndex);
	/// \ingroup Acquisition
	/// \brief Returns the current status (enabled/disabled) of an analog input channel. Use #getDevicePropertyInt with flag Device_NumOfAnalogInputChannels to determine the number of available channels.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] ChannelIndex Index of the analog input channel to query	///
	/// \return True if channel is enabled, false otherwise
	SPECTRALRADAR_API bool getAnalogInputChannelEnabled(OCTDeviceHandle Dev, int ChannelIndex);

	/// \fn const char* getAnalogInputChannelName(OCTDeviceHandle Dev, int ChannelIndex);
	/// \ingroup Acquisition
	/// \brief Returns the name of an analog input channel. Use #getDevicePropertyInt with flag Device_NumOfAnalogInputChannels to determine the number of available channels.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] ChannelIndex Index of the analog input channel to query
	/// \return String containing the descriptive name of the channel 
	SPECTRALRADAR_API const char* getAnalogInputChannelName(OCTDeviceHandle Dev, int ChannelIndex);

	/// \fn void stopMeasurement(OCTDeviceHandle);
	/// \ingroup Acquisition
	/// \brief stops the current measurement.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	SPECTRALRADAR_API void stopMeasurement(OCTDeviceHandle Dev);

	/// \fn void measureSpectra(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw)
	/// \ingroup Acquisition
	/// \brief Acquires the desired number of spectra (raw data without processing) without moving galvo scanners.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] NumberOfSpectra The desired number of spectra.
	/// \param[out] Raw A valid (non null) handle of raw data (#RawDataHandle), where the acquired spectra will be stored. The
	///                 meta data (dimensions, sizes, bytes per pixel, etc.) will be adjusted automatically.
	///
	/// This procedure assumes that there is no any ongoing measurement process (started with the function #startMeasurement). The indicated number
	/// of measurements will be carried out. The user should not stop the measurement (this function will block till the whole data is ready).\n
	/// If the hardware contains more than one camera, all cameras will be triggered, because the hardware has been setup to do so. This function
	/// will return raw data only for the first camera (the master). The raw data for the slaves, acquired simultaneously, will be available for
	/// retrieval any time afterwards (the function #measureSpectraEx should be used).\n
	/// This function blocks till the desired number of spectra get written in the indicated buffer (\p Raw).\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void measureSpectra(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw);

	/// \fn void measureSpectraEx(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw, int CameraIndex)
	/// \ingroup Acquisition
	/// \brief Acquires the desired number of spectra (raw data without processing) without moving galvo scanners, for the desired camera.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] NumberOfSpectra The desired number of spectra.
	/// \param[out] Raw A valid (non null) handle of raw data (#RawDataHandle), where the acquired spectra will be stored. The
	///                 meta data (dimensions, sizes, bytes per pixel, etc.) will be adjusted automatically.
	/// \param[in] CameraIndex The camera index (0-based, i.e. zero for the first = master, one for the second, and so on). \warning{Unless the program
	///                        divides the acquistion in different threads, this function should be invoked first for the master camera
	///                        (\p CameraIdx = 0) and only then for the slaves. Otherwise it will block for ever.}
	///
	/// This procedure assumes that there is no any ongoing measurement process (started with the function #startMeasurement). The indicated number
	/// of measurements will be carried out. The user should not stop the measurement (this function will block till the whole data is ready).\n
	/// If the hardware contains more than one camera, all cameras will be triggered together with the first one (the master), because the hardware has
	/// been setup to do so. If \p CameraIdx is different from zero, i.e. a slave is meant, this function will retrieve the spectra measured
	/// together with the master. If those data happen to be already consumed, this function will block until the master triggers. Notice that in
	/// a single thread programming model, the program would stop execution for ever. For this reason, it is strongly adviced to invoke this function
	/// first for the master (\p CameraIdx = 0) and only then for the slaves.\n
	/// This function will retrieve raw data only for the selected camera. The user must invoke this function for each camera separately, but in 
	/// judicious order, as explained before.\n
	/// This function blocks till the desired number of spectra get written in the indicated buffer (\p Raw).\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void measureSpectraEx(OCTDeviceHandle Dev, int NumberOfSpectra, RawDataHandle Raw, int CameraIndex);

	/// \defgroup Processing Processing
	/// \brief Standard Processing Routines.
	///
	/// \enum Processing_FFTType
	/// \ingroup Processing
	/// \brief defindes the algorithm used for dechirping the input signal and Fourier transformation
	typedef enum {
		/// FFT with no dehchirp algorithm applied.
		Processing_StandardFFT,
		/// Full matrix multiplication ("filter bank"). Mathematical precise dechirp, but rather slow.
		Processing_StandardNDFT,
		/// Linear interpolation prior to FFT.
		Processing_iFFT,
		/// NFFT algorithm with parameter m=1.
		Processing_NFFT1,
		/// NFFT algorithm with parameter m=2.
		Processing_NFFT2,
		/// NFFT algorithm with parameter m=3.
		Processing_NFFT3,
		/// NFFT algorithm with parameter m=4.
		Processing_NFFT4,
	} Processing_FFTType;

	/// \enum DispersionCorrectionType
	/// \ingroup Processing
	/// \brief To select the dispersion correction algorithm.
	typedef enum {
		/// No software dispersion correction is used.
		Dispersion_None,
		/// Quadratic dispersion correction is used with the specified factor in #setDispersionQuadraticCoeff.
		Dispersion_QuadraticCoeff,
		/// The specified dispersion preset from #setDispersionPresets is used. For more information please see the documentation of #setDispersionPresets.
		Dispersion_Preset,
		/// Vector for dispersion correction needs to be supplied manually. Please use the #setCalibration function.
		Dispersion_Manual
	} DispersionCorrectionType;

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
		Processing_MinElectrons,
		Processing_FFTOversampling,
		/// Largest (absolute) value that the processing will expect for raw samples.
		Processing_MaxSensorValue
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
		Processing_RemoveFixedPattern, // 14
		/// Flag to calculate sensor saturation, used in swept source OCT systems. This flag is deactivated by default.
		Processing_CalculateSaturation //15
	} ProcessingFlag;

	/// \enum ProcessingAveragingAlgorithm
	/// \ingroup Processing
	/// \brief This sets the averaging algorithm to be used for processing. \warning{This features is still experimental and might contain bugs.}
	typedef enum {
		Processing_Averaging_Min,
		/// Default.
		Processing_Averaging_Mean,
		Processing_Averaging_Median,
		Processing_Averaging_Norm2,
		Processing_Averaging_Max,
		Processing_Averaging_Fourier_Min,
		Processing_Averaging_Fourier_Norm4,
		Processing_Averaging_Fourier_Max,
		Processing_Averaging_StandardDeviationAbs,
		Processing_Averaging_PhaseMatched,
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

	/// \fn ProcessingHandle createProcessing(int SpectrumSize, int BytesPerRawPixel, BOOL Signed, float ScalingFactor, float MinElectrons, Processing_FFTType Type, float FFTOversampling);
	/// \ingroup Processing
	/// \brief Creates processing routines with the desired properties. 
	/// \param[in] SpectrumSize The number of pixels in each spectrum.
	/// \param[in] BytesPerRawPixel The number of bytes in each pixel (e.g. two for a 12-bit resolution). Currently, 1, 2, and 4-bytes per pixel are
	///								supported. 1 and 2-bytes per pixel assume an integer representation, whereas 4-bytes per pixel assumes a single
	///								precision floating point representation.
	/// \param[in] Signed Indicates whether the value of each pixel is signed or not. This parameter is ignored in case of floating point representations.
	/// \param[in] ScalingFactor A multiplicative constant to transform digital levels into the number of electrons actually freed.
	/// \param[in] MinElectrons A threshold. This value is used to identify the portions of the measured spectra (close to the edges) where the signal-to-noise
	///                         ratio is too poor for any practical purposes. After the \p ScalingFactor has been applied to the digitized data (i.e. a spectrum
	///                         has been measured), this threshold can be used to identify the portions near the edges that can be regarded as "near zero".
	/// \param[in] Type Specifies the FFT algorithm (#Processing_FFTType) that will combine the dechirping with the Fourier transform.
	/// \param[in] FFTOversampling In case the selected FFT algorithm bases on oversampling, this parameter gives the factor.
	/// \return A handle of the newly created processing routines (#ProcessingHandle).
	SPECTRALRADAR_API ProcessingHandle createProcessing(int SpectrumSize, int BytesPerRawPixel, BOOL Signed, float ScalingFactor, float MinElectrons, Processing_FFTType Type, float FFTOversampling);

	/// \fn ProcessingHandle createProcessingForDevice(OCTDeviceHandle Dev);
	/// \ingroup Processing
	/// \brief Creates processing routines for the specified device (#OCTDeviceHandle).
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \return A handle of the newly created processing routines (#ProcessingHandle).
	///
	/// In systems containing several cameras, there should be one set of processing routines for each camera. The reason is that each
	/// camera has its own calibration, and the calibration is an integral part of the computations. This function creates and returns
	/// a handle only for the first camera. Thus, this function is intended for systems contining a single camera. In case of systems
	/// containing several cameras, the function #createProcessingForDeviceEx should be used instead.
	SPECTRALRADAR_API ProcessingHandle createProcessingForDevice(OCTDeviceHandle Dev);

	/// \fn ProcessingHandle createProcessingForDeviceEx(OCTDeviceHandle Dev, int CameraIndex)
	/// \ingroup Processing
	/// \brief Creates processing routines for the specified device (#OCTDeviceHandle) with camera index.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] CameraIndex The camera index (0-based, i.e. zero for the first, one for the second, and so on).
	/// \return A handle of the newly created processing routines (#ProcessingHandle).
	///
	/// In systems containing several cameras, there should be one set of processing routines for each camera. The reason is that each
	/// camera has its own calibration, and the calibration is an integral part of the computations. This function creates and returns
	/// a handle only for the first camera. Thus, this function is intented for systems contining more than one camera. In case the
	/// second parameter (\p CameraIndex) is zero, this function is equivalent to #createProcessingForDevice.
	SPECTRALRADAR_API ProcessingHandle createProcessingForDeviceEx(OCTDeviceHandle Dev, int CameraIndex);

	/// \fn ProcessingHandle createProcessingForOCTFile(OCTFileHandle File)
	/// \ingroup Processing
	/// \brief Creates processing routines for the specified OCT file (#OCTFileHandle), such that the processing conditions are exactly
	///        the same as those when the file had been saved.
	/// \param[in] File A valid (non null) OCT file handle (#OCTFileHandle).
	/// \return A handle of the newly created processing routines (#ProcessingHandle).
	SPECTRALRADAR_API ProcessingHandle createProcessingForOCTFile(OCTFileHandle File);

	/// \fn ProcessingHandle createProcessingForOCTFileEx(OCTFileHandle File, const int CameraIndex)
	/// \ingroup Processing
	/// \brief Creates processing routines for the specified OCT file (#OCTFileHandle), such that the processing conditions are exactly
	///        the same as those when the file had been saved.
	/// \param[in] File A valid (non null) OCT file handle (#OCTFileHandle).
	/// \param[in] CameraIndex The detector index (first camera has zero index).
	/// \return A handle of the newly created processing routines (#ProcessingHandle).
	///
	/// For systems with one camera, this function fall backs to #createProcessingForOCTFile.
	SPECTRALRADAR_API ProcessingHandle createProcessingForOCTFileEx(OCTFileHandle File, const int CameraIndex);

	/// \fn int getInputSize(ProcessingHandle);
	/// \ingroup Processing
	/// \brief Returns the expected input size (pixels per spectrum) of the processing algorithms. 
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return The number of pixels per spectrum.
	///
	/// This function is provided for convenience as processing routines can be used independently of the device.
	SPECTRALRADAR_API int getInputSize(ProcessingHandle Proc);

	/// \fn int getAScanSize(ProcessingHandle Proc);
	/// \ingroup Processing
	/// \brief Returns the number of pixels in an A-Scan that can be obtained (computed) with the given processing routines.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return The number of pixels in an A-Scan that can be obtained (computed) with the given processing routines.
	///
	/// The returned number is identical to the number of rows in a finished B-Scan, that can also be retrieved (after the 
	/// processing has been executed) by invoking one of the functions #getDataPropertyInt, #getComplexDataPropertyInt,
	/// or #getColoredDataPropertyInt, passing the enumeration item #Data_Size1 as the second parameter, and passing
	/// the respective data object (#DataHandle, #ComplexDataHandle, #ColoredDataHandle) as the first parameter.
	SPECTRALRADAR_API int getAScanSize(ProcessingHandle Proc);

	/// \fn void setApodizationWindow(ProcessingHandle Proc, ApodizationWindow Window)
	/// \ingroup Processing
	/// \brief Sets the windowing function that will be used for apodization (this apodization has nothing to do with the reference spectra
	///        measured without a sample!). The selected windowing function will be used in all subsequent processings right before the fast
	///        Fourier transformation.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Window The desired apodization window to be used for apodizations right before Fourier transformations.
	/// 
	/// The selection of a windowing function is a balance between the acceptable width of the main lobe (that is, how many "frequency bins"
	/// does it take the response to reach half maximum power) and the attenuation of the side lobes (that is, what level of artifacts caused
	/// by the spectral leakage can be tolerated). As such, it depends on the paticular experiment. The default selection (Hann windowing)
	/// cannot be expected to fit everyone's needs.\n
	/// If this function is not explicitly called, a Hann window will be assumed (#Apodization_Hann).
	SPECTRALRADAR_API void setApodizationWindow(ProcessingHandle Proc, ApodizationWindow Window);

	/// \fn ApodizationWindow getApodizationWindow(ProcessingHandle Proc);
	/// \ingroup Processing
	/// \brief Returns the current windowing function that is being used for apodization, #ApodizationWindow (this apodization
	///        is not the reference spectrum measured without a sample!).
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return The current windowing function that is being used for apodization (#ApodizationWindow) right before Fourier transformations.
	SPECTRALRADAR_API ApodizationWindow getApodizationWindow(ProcessingHandle Proc);

	/// \fn void setApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection, double Value);
	/// \ingroup Processing
	/// \brief Sets the apodization window parameter, such as window width or ratio between constant and cosine part. Notice that
	///        this apodization is unrelated to the reference spectrum measured without a sample!.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection The desired parameter whose value will be changed (#ApodizationWindowParameter).
	/// \param[in] Value The desired value for the parameter.
	SPECTRALRADAR_API void setApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection, double Value);

	/// \fn double getApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection);
	/// \ingroup Processing
	/// \brief Gets the apodization window parameter, such as window width or ratio between constant and cosine part. Notice that
	///        this apodization is unrelated to the reference spectrum measured without a sample!.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection The desired parameter whose value shall be be retrieved (#ApodizationWindowParameter).
	/// \return The current value of the parameter.
	SPECTRALRADAR_API double getApodizationWindowParameter(ProcessingHandle Proc, ApodizationWindowParameter Selection);

	/// \fn SPECTRALRADAR_API void getCurrentApodizationEdgeChannels(ProcessingHandle Proc, int* LeftPix, int* RightPix);
	/// \ingroup Processing
	/// \brief Returns the pixel positions of the left/right edge channels of the current apodization. Here apodization
	///        refers to the reference spectra measured without sample.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[out] LeftPix The address to store the position of the last pixel position, starting from the left, at which
	///                     the intensity is too low for reliable computations. If a nullptr is given, nothing will be
	///                     written on it.
	/// \param[out] RightPix The address to store the position of the last pixel position, starting from the right, at which
	///                      the intensity is too low for reliable computations. If a nullptr is given, nothing will be
	///                     written on it.
	///
	/// The apodization spectra (i.e. the spectra measured without a sample) have regions, at their left and right edges,
	/// where the signal to noise ratio is too low for practical purposes. This function returns the position of the last
	/// pixel position (or channel) at which the measured intensity is insufficient for reliable computations.\n
	/// Notice that the camera is upside down. Hence the right-most pixel refers to the shortest measured wavelength,
	/// and the left-most pixel refers to the longest measured wavelength.\n
	/// The second and third pointers are addresses in memory managed by the user, not by SpectralRadar. 
	SPECTRALRADAR_API void getCurrentApodizationEdgeChannels(ProcessingHandle Proc, int* LeftPix, int* RightPix);

	/// \fn void setProcessingDechirpAlgorithm(ProcessingHandle Proc, Processing_FFTType Type, float Oversampling)
	/// \ingroup Processing
	/// \brief Sets the algorithm to be used for dechirping the input spectra.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Type Specifies the FFT algorithm (#Processing_FFTType) that will combine the dechirping with the Fourier transform.
	/// \param[in] Oversampling In case the selected FFT algorithm bases on oversampling, this parameter gives the factor.
	SPECTRALRADAR_API void setProcessingDechirpAlgorithm(ProcessingHandle Proc, Processing_FFTType Type, float Oversampling);

	/// \fn void setProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection, int Value);
	/// \ingroup Processing
	/// \brief Sets the specified integer value processing parameter.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection The parameter whose value will be modified.
	/// \param[in] Value The desired value for the integer parameter.
	SPECTRALRADAR_API void setProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection, int Value);

	/// \fn int getProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection);
	/// \ingroup Processing
	/// \brief Returns the specified integer value processing parameter.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection The parameter whose value will be retrieved.
	/// \return The current value of the integer parameter.
	SPECTRALRADAR_API int getProcessingParameterInt(ProcessingHandle Proc, ProcessingParameterInt Selection);

	/// \fn  void setProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection, double Value);
	/// \ingroup Processing
	/// \brief Sets the specified floating point processing parameter. 
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection The floating point parameter whose value will be modified.
	/// \param[in] Value The desired value for the floating point parameter.
	SPECTRALRADAR_API void setProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection, double Value);

	/// \fn  double getProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection)
	/// \ingroup Processing
	/// \brief Gets the specified floating point processing parameter. 
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection The floating point parameter whose value will be retrieved.
	/// \return The current value of the floating point parameter.
	SPECTRALRADAR_API double getProcessingParameterFloat(ProcessingHandle Proc, ProcessingParameterFloat Selection);

	/// \fn void setProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag, BOOL Value);
	/// \ingroup Processing
	/// \brief Sets the specified processing flag.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Flag The flag whose value will be modified.
	/// \param[in] Value The desired value for the flag.
	SPECTRALRADAR_API void setProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag, BOOL Value);

	/// \fn BOOL getProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag);
	/// \ingroup Processing
	/// \brief Returns TRUE if the specified processing flag is set, FALSE otherwise.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Flag The flag whose value will be retrieved.
	/// \return The current value of the flag.
	SPECTRALRADAR_API BOOL getProcessingFlag(ProcessingHandle Proc, ProcessingFlag Flag);

	/// \fn void setProcessingAveragingAlgorithm(ProcessingHandle Proc, ProcessingAveragingAlgorithm Algorithm);
	/// \ingroup Processing
	/// \brief Sets the algorithm that will be used for averaging during the processing.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Algorithm The averaging algorithm (#ProcessingAveragingAlgorithm). If this function is not explicetely
	///            invoked, the value #Processing_Averaging_Mean can be assumed.
	SPECTRALRADAR_API void setProcessingAveragingAlgorithm(ProcessingHandle Proc, ProcessingAveragingAlgorithm Algorithm);

	/// \fn void setCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);
	/// \ingroup Processing
	/// \brief Sets the calibration data.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be set (#CalibrationData).
	/// \param[in] Data A valid handle (#DataHandle) of the calibration data that will be set. 
	SPECTRALRADAR_API void setCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);

	/// \fn void getCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);
	/// \ingroup Processing
	/// \brief Retrieves the desired calibration vector.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be set (#CalibrationData).
	/// \param[out] Data A valid handle (#DataHandle) of the calibration data that will be retrieved. \p Data will be
	///                  automatically resized for the data to fit in the structure.
	SPECTRALRADAR_API void getCalibration(ProcessingHandle Proc, CalibrationData Selection, DataHandle Data);

	/// \fn void measureCalibration(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection);
	/// \ingroup Processing
	/// \brief Measures the specified calibration parameters and uses them in subsequent processing.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be measured (#CalibrationData).
	///
	/// If the hardware contains more than one camera, all cameras will be triggered, because the hardware has been setup to do so. This function
	/// will return raw data only for the first camera (the master). The raw data for the slaves, acquired simultaneously, will be available for
	/// retrieval any time afterwards (the function #measureCalibrationEx should be used).\n
	/// Using the parameters #Calibration_ApodizationSpectrum or #Calibration_ApodizationVector will acquire the apodization spectra without
	/// moving the mirrors to the apodization position. To acquire the spectra used for the processing in the apodization position use
	/// #measureApodizationSpectra.
	/// Please note that the apodization spectra will not be acquired in thespecified apodization position from the #ProbeHandle.
	SPECTRALRADAR_API void measureCalibration(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection);

	/// \fn void measureCalibrationEx(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection, int CameraIndex)
	/// \ingroup Processing
	/// \brief Measures the specified calibration parameters and uses them in subsequent processing with specified camera index.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be measured (#CalibrationData).
	/// \param[in] CameraIndex The camera index (0-based, i.e. zero for the first = master, one for the second, and so on). \warning{Unless the program
	///                        divides the acquistion in different threads, this function should be invoked first for the master camera
	///                        (\p CameraIdx = 0) and only then for the slaves. Otherwise it will block for ever.}
	///
	/// If the hardware contains more than one camera, all cameras will be triggered together with the first one (the master), because the hardware has
	/// been setup to do so. If \p CameraIdx is different from zero, i.e. a slave is meant, this function will retrieve the spectra measured
	/// together with the master. If those data happen to be already consumed, this function will block until the master triggers. Notice that in
	/// a single thread programming model, the program would stop execution for ever. For this reason, it is strongly adviced to invoke this function
	/// first for the master (\p CameraIdx = 0) and only then for the slaves.\n
	/// Using the parameters #Calibration_ApodizationSpectrum or #Calibration_ApodizationVector will acquire the apodization spectra without moving the
	/// mirrors to the apodization position. To acquire the spectra used for the processing in the apodization position use #measureApodizationSpectra.
	SPECTRALRADAR_API void measureCalibrationEx(OCTDeviceHandle Dev, ProcessingHandle Proc, CalibrationData Selection, int CameraIndex);

	/// \fn void measureApodizationSpectra(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc)
	/// \ingroup Processing
	/// \brief Measures the apodization spectra in the defined apodization position and size and uses them in subsequent processing.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Probe A valid (non null) probe handle (#ProbeHandle), previously generated with the function #initProbe.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///
	/// If the hardware contains more than one camera, all cameras will be triggered, because the hardware has been setup to do so. This function
	/// will return raw data only for the first camera (the master). The raw data for the slaves, acquired simultaneously, will be available for
	/// retrieval any time afterwards (the function #measureCalibrationEx should be used).\n
	SPECTRALRADAR_API void measureApodizationSpectra(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc);

	/// \fn void saveCalibrationDefault(ProcessingHandle Proc, CalibrationData Selection);
	/// \ingroup Processing
	/// \brief Saves the selected calibration in its default path. This same default path will be used by SpectralRadar in subsequent executions to
	///        retrieve the calibration data.
	/// \warning This will override your default calibration of the device.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be saved (#CalibrationData).
	///
	/// In systems with more than one camera, this function will only save calibration data pertaining to the first camera. For the other cameras
	/// use function #saveCalibrationDefaultEx.
	SPECTRALRADAR_API void saveCalibrationDefault(ProcessingHandle Proc, CalibrationData Selection);

    /// \fn void saveCalibrationDefaultEx(ProcessingHandle Proc, CalibrationData Selection, int CameraIndex);
    /// \ingroup Processing
    /// \brief Saves the selected calibration in its default path, for the selected camera. This same default path will be used by SpectralRadar in
	///        subsequent executions to retrieve the calibration data. \warning This will override your default calibration of the device.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be saved (#CalibrationData).
	/// \param[in] CameraIndex The camera index (0-based, i.e. zero for the first, one for the second, and so on).
	///
	/// This function will only save calibration data pertaining to the selected camera. To save the calibration of all cameras, multiple invokations
	/// are needed. the order plays no role.
	SPECTRALRADAR_API void saveCalibrationDefaultEx(ProcessingHandle Proc, CalibrationData Selection, int CameraIndex);

	/// \fn void saveCalibration(ProcessingHandle Proc, CalibrationData Selection, const char* Path);
	/// \ingroup Processing
	/// \brief Saves the selected calibration in the specified path.
	///        \warning This will override your default calibration of the device if you specifiy the default path. 
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be saved (#CalibrationData).
	/// \param[in] Path A zero terminated string specifying the filename, including full path.
	SPECTRALRADAR_API void saveCalibration(ProcessingHandle Proc, CalibrationData Selection, const char* Path);

	/// \fn void loadCalibration(ProcessingHandle Proc, CalibrationData Selection, const char* Path);
	/// \ingroup Processing
	/// \brief Will load a specified calibration file and its content will be used for subsequent processing. 
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Selection Indicates the calibration that will be saved (#CalibrationData).
	/// \param[in] Path A zero terminated string specifying the filename, including full path.
	SPECTRALRADAR_API void loadCalibration(ProcessingHandle Proc, CalibrationData Selection, const char* Path);

	/// \fn void setSpectrumOutput(ProcessingHandle Proc, DataHandle Spectrum);
	/// \ingroup Processing
	/// \brief Sets the location for the resulting spectral data.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Spectrum A valid (non null) data handle (#DataHandle). Suitable sizes and ranges will be automatically set
	///                     during the processing (#executeProcessing).
	SPECTRALRADAR_API void setSpectrumOutput(ProcessingHandle Proc, DataHandle Spectrum);

	/// \fn void setOffsetCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle OffsetCorrectedSpectrum);
	/// \ingroup Processing
	/// \brief Sets the location for the resulting offset corrected spectral data.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] OffsetCorrectedSpectrum A valid (non null) data handle (#DataHandle). Suitable sizes and ranges will be
	///                                    automatically set during the processing (#executeProcessing).
	SPECTRALRADAR_API void setOffsetCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle OffsetCorrectedSpectrum);

	/// \fn void setDCCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle DCCorrectedSpectrum)
	/// \ingroup Processing
	/// \brief Sets the location for the resulting DC removed spectral data.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] DCCorrectedSpectrum A valid (non null) data handle (#DataHandle). Suitable sizes and ranges will be
	///                                automatically set during the processing (#executeProcessing).
	SPECTRALRADAR_API void setDCCorrectedSpectrumOutput(ProcessingHandle Proc, DataHandle DCCorrectedSpectrum);

	/// \fn void setApodizedSpectrumOutput(ProcessingHandle Proc, DataHandle ApodizedSpectrum);
	/// \ingroup Processing
	/// \brief Sets the location for the resulting apodized spectral data.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] ApodizedSpectrum A valid (non null) data handle (#DataHandle). Suitable sizes and ranges will be
	///                             automatically set during the processing (#executeProcessing).
	SPECTRALRADAR_API void setApodizedSpectrumOutput(ProcessingHandle Proc, DataHandle ApodizedSpectrum);

	/// \fn void setComplexDataOutput(ProcessingHandle Proc, ComplexDataHandle ComplexScan)
	/// \ingroup Processing
	/// \brief Sets the pointer to the resulting complex scans that will be written after subsequent processing executions.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] ComplexScan A valid (non null) complex data handle (#ComplexDataHandle). Suitable sizes and ranges will be
	///                         automatically set during the processing (#executeProcessing).
	///
	/// After the next completion of the function #executeProcessing(), this complex data object will contain the real and imaginary
	/// parts of the scans.\n
	/// If set to a nullptr, no complex data result will be written in the subsequent processing executions.
	SPECTRALRADAR_API void setComplexDataOutput(ProcessingHandle Proc, ComplexDataHandle ComplexScan);

	/// \fn void setProcessedDataOutput(ProcessingHandle Proc, DataHandle Scan)
	/// \ingroup Processing
	/// \brief Sets the pointer to the resulting scans that will be written after subsequent processing executions.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Scan A valid (non null) data handle (#DataHandle). Suitable sizes and ranges will be automatically set
	///                 during the processing (#executeProcessing).
	///
	/// After the next completion of the function #executeProcessing(), this data object will contain the amplitude (in dB)
	/// of the scans.\n
	/// If set to nullptr no processed floating point data in dB will be written in the subsequent processing executions.
	SPECTRALRADAR_API void setProcessedDataOutput(ProcessingHandle Proc, DataHandle Scan);

	/// \fn void setColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle Scan, ColoringHandle Color)
	/// \ingroup Processing
	/// \brief Sets the pointer to the resulting colored scans that will be written after subsequent processing executions.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Scan A valid (non null) colored data handle (#ColoredDataHandle). Suitable sizes and ranges will be
	///                 automatically set during the processing (#executeProcessing).
	/// \param[in] Color A valid (non null) coloring handle (#ColoringHandle) as created, for example, with the functions
	///                  #createColoring32Bit() or #createCustomColoring32Bit().
    ///
	/// After the next completion of the function #executeProcessing(), this data object will contain the colored amplitude
	/// of the scans.\n
	/// If set to nullptr no colored data will be written in the subsequent processing executions.
	SPECTRALRADAR_API void setColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle Scan, ColoringHandle Color);

	/// \fn void setTransposedColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle Scan, ColoringHandle Color);
	/// \ingroup Processing
	/// \brief Sets the pointer to the resulting colored scans that will be written after subsequent processing executions.
	///        The orientation of the colored data will be transposed in such a way that the first axis (normally z-axis) will be
	///        the x-axis (the depth of each individual A-scan) and the second axis (normally x-axis) will be the z-axis. 
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Scan A valid (non null) colored data handle (#ColoredDataHandle). Suitable sizes and ranges will be
	///                 automatically set during the processing (#executeProcessing).
	/// \param[in] Color A valid (non null) coloring handle (#ColoringHandle) as created, for example, with the functions
	///                  #createColoring32Bit() or #createCustomColoring32Bit().
    ///
	/// After the next completion of the function #executeProcessing(), this data object will contain the transposed colored
	/// amplitude of the scans.\n
	/// If set to nullptr no colored data will be written in the subsequent processing executions.
	SPECTRALRADAR_API void setTransposedColoredDataOutput(ProcessingHandle Proc, ColoredDataHandle Scan, ColoringHandle Color);

	/// \fn void executeProcessing(ProcessingHandle Proc, RawDataHandle RawData)
	/// \ingroup Processing
	/// \brief Executes the processing. The results will be stored as requested through the functions #setProcessedDataOutput(),
	///        #setComplexDataOutput(), #setColoredDataOutput() (including coloring properties) and similar ones. In all cases,
	///        sizes and ranges will be adjusted automatically to the right values.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] RawData A valid (non null) handle of raw data (#RawDataHandle) with fresh measured data (e.g. acquired with the
	///                    #getRawData() function.
	SPECTRALRADAR_API void executeProcessing(ProcessingHandle Proc, RawDataHandle RawData);

	/// \fn void clearProcessing(ProcessingHandle Proc);
	/// \ingroup Processing
	/// \brief Clears the processing instance and frees all temporary memory that was associated with it. Processing threads will be stopped.
	/// \param[in] Proc A handle of the processing routines (#ProcessingHandle). If the handle is a nullptr, this function does nothing.
	///                 In most cases this handle will have been previously obtained through one of the functions #createProcessing,
	///                 #createProcessingForDevice, #createProcessingForDeviceEx or #createProcessingForOCTFile.
	SPECTRALRADAR_API void clearProcessing(ProcessingHandle Proc);

	/// \fn void computeDispersion(DataHandle Spectrum1, DataHandle Spectrum2, DataHandle Chirp, DataHandle Disp);
	/// \ingroup Processing
	/// \brief Computes the dispersion and chirp of the two provided spectra, where both spectra need to have been subjected 
	///        to same dispersion mismatch. Both spectra need to have been acquired for different path length differences.
	/// \param[in] Spectrum1 A valid (non null) handle of data (#DataHandle) with an apodized spectrum, with the functions
	///                      #setApodizedSpectrumOutput() followed by #executeProcessing(), measuring a test reflector positioned at a
	///                      a distance different from the one used for the second parameter.
	/// \param[in] Spectrum2 A valid (non null) handle of data (#DataHandle) with an apodized spectrum, with the functions
	///                      #setApodizedSpectrumOutput() followed by #executeProcessing(), measuring a test reflector positioned at a
	///                      a distance different from the one used for the first parameter.
	/// \param[out] Chirp A valid (non null) handle of data (#DataHandle) where the calculated chirp curve will be written.
	/// \param[out] Disp A valid (non null) handle of data (#DataHandle) where the calculated dispersion curve will be written.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API void computeDispersion(DataHandle Spectrum1, DataHandle Spectrum2, DataHandle Chirp, DataHandle Disp);

	/// \fn void computeDispersionByCoeff(double Quadratic, DataHandle Chirp, DataHandle Disp);
	/// \ingroup Processing
	/// \brief Computes dispersion by a quadratic approximation specified by the quadratic factor.
    /// \param[in] Quadratic The leading coefficient of the second order polynomia that will define the dispersion curve.
	/// \param[in] Chirp A valid (non null) handle of data (#DataHandle) where a valid chirp curve has been stored.
	/// \param[out] Disp A valid (non null) handle of data (#DataHandle) where the calculated dispersion curve will be written.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API void computeDispersionByCoeff(double Quadratic, DataHandle Chirp, DataHandle Disp);

	/// \fn void computeDispersionByImage(DataHandle LinearKSpectra, DataHandle Chirp, DataHandle Disp);
	/// \ingroup Processing
	/// \brief Guesses the dispersion based on the spectral data specified. The spectral data needs to be linearized in wavenumber before using this function.
	/// \param[in] LinearKSpectra A valid (non null) handle of data (#DataHandle) where the input spectra is stored.
	///                           The spectral data needs to be linearized in wavenumber (not wavelength) before using this function.
	/// \param[in] Chirp A valid (non null) handle of data (#DataHandle) where a valid chirp curve has been stored.
	/// \param[out] Disp A valid (non null) handle of data (#DataHandle) where the calculated dispersion curve will be written.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API void computeDispersionByImage(DataHandle LinearKSpectra, DataHandle Chirp, DataHandle Disp);

	/// \fn int getNumberOfDispersionPresets(ProcessingHandle Proc)
	/// \ingroup Processing
	/// \brief Gets the number of dispersion presets.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return The number of dispersion presets.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API int getNumberOfDispersionPresets(ProcessingHandle Proc);

	/// \fn const char* getDispersionPresetName(ProcessingHandle Proc, int Index)
	/// \ingroup Processing
	/// \brief Gets the name of the dispersion preset specified with index.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Index The index of the desired dispersion preset.
	/// \return A zero terminated string with the name of the dispersion preset associated with the given index.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API const char* getDispersionPresetName(ProcessingHandle Proc, int Index);

	/// \fn void setDispersionPresetByName(ProcessingHandle Proc, const char* Name)
	/// \ingroup Processing
	/// \brief Sets the dispersion preset specified with name.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Name A zero terminated string with the name of the desired dispersion preset that has to be set.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API void setDispersionPresetByName(ProcessingHandle Proc, const char* Name);

	/// \fn void setDispersionPresetByIndex(ProcessingHandle Proc, int Index)
	/// \ingroup Processing
	/// \brief Sets the dispersion preset specified with index.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Index An index specifying the desired dispersion preset that has to be set.
	///
	/// For a detailed explanation, do please refer to the documentation of #setDispersionPresets.
	SPECTRALRADAR_API void setDispersionPresetByIndex(ProcessingHandle Proc, int Index);

	/// \fn void setDispersionPresets(ProcessingHandle Proc, ProbeHandle Probe)
	/// \ingroup Processing
	/// \brief Sets the dispersion presets for the probe.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Probe A valid (non null) probe handle (#ProbeHandle), previously generated with the function #initProbe.
	///
	/// Unfortunately no really good and easy method to predict the dispersion coefficient(s) is offered here. The coefficients
	/// (currently) used in the software do not correspond to physically meaningful parameters, but are rather given in arbitrary
	/// units.\n
    /// Hence it is suggested to use image quality as of criterion to set the coefficients, because this criterion usually works
	/// quite well. The quadratic coefficient can be easily found by using the ThorImage OCT software, and using the built-in
	/// quadratic slider and simultaneously looking for image quality and axial sharpness. Usually, the quadratic parameter alone
	/// gives rather good quality and dispersion correction and only for very broadband sources and strong dispersion higher
	/// coefficients are required. \n
	/// To set higher coefficients, either this SDK is required or an entry "Dispersion_NameOfPreset" has to be added to the
	/// respective probe.ini file being used (see Settings Dialog of ThorImage OCT). This file is default located in
    /// C:\\Program Files\\Thorlabs\\SpectralRadar\\Config (or equivalent, if the software has been installed to another location).
	/// In probe.ini, the relevant entry looks like:\n
    /// Dispersion_Probe = 10.0, 2.0, -1.0\n
    /// and this particular example would set three dispersion factors, the quadratic one being 10, the third degree 2, and
	/// fourth degree -1. Again, unfortunately, there is no option to set these automatically. The user has to experiment
	/// with different parameters and iteratively optimize for a sharp signal. After this entry has been added to the file
	/// probe.ini, the Preset dispersions menu will contain a new entry on the next software start of ThorImageOCT.\n
	/// Of course, the presets added to probe.ini can also be used by the functions in this SDK. Each line should give a 
	/// different preset-name, and after this function is invoked, all of them will be available through indeces
	/// (#setDispersionPresetByIndex, #getNumberOfDispersionPresets) or names (#getDispersionPresetName, #setDispersionPresetByName).
	SPECTRALRADAR_API void setDispersionPresets(ProcessingHandle Proc, ProbeHandle Probe);

	/// \fn Processing_FFTType getProcessing_FFTType(ProcessingHandle Proc)
	/// \ingroup Processing
	/// \brief Retrieve the active FFT Type.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return the current FFT algorithm type (#Processing_FFTType) that will combine the dechirping with the Fourier transform.
	SPECTRALRADAR_API Processing_FFTType getProcessing_FFTType(ProcessingHandle Proc);

	/// \fn void setDispersionCorrectionType(ProcessingHandle Proc, DispersionCorrectionType Type);
	/// \ingroup Processing
	/// \brief Sets the active dispersion correction type.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Type The specification of the dispersion correction algorithm (#DispersionCorrectionType).
	SPECTRALRADAR_API void setDispersionCorrectionType(ProcessingHandle Proc, DispersionCorrectionType Type);

	/// \fn DispersionCorrectionType getDispersionCorrectionType(ProcessingHandle Proc);
	/// \ingroup Processing
	/// \brief Sets the active dispersion correction type.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return The currently active dispersion correction algorithm (#DispersionCorrectionType).
	SPECTRALRADAR_API DispersionCorrectionType getDispersionCorrectionType(ProcessingHandle Proc);

	/// \fn void setDispersionQuadraticCoeff(ProcessingHandle Proc, double Coeff)
	/// \ingroup Processing
	/// \brief Sets the coefficient for the quadratic correction of the dispersion.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Coeff The desired coefficient.
	SPECTRALRADAR_API void setDispersionQuadraticCoeff(ProcessingHandle Proc, double Coeff);

	/// \fn double getDispersionQuadraticCoeff(ProcessingHandle Proc)
	/// \ingroup Processing
	/// \brief Sets the coefficient for the quadratic correction of the dispersion.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return The coefficient currently used for the quadratic correction of the dispersion.
	SPECTRALRADAR_API double getDispersionQuadraticCoeff(ProcessingHandle Proc);

	/// \fn const char* getCurrentDispersionPresetName(ProcessingHandle Proc)
	/// \ingroup Processing
	/// \brief Gets the name of the active dispersion preset.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \return A zero terminated string with the name of the active dispersion preset.
	SPECTRALRADAR_API const char* getCurrentDispersionPresetName(ProcessingHandle Proc);
	
	/// \defgroup ImportExport Export and Import
	/// \brief Functionality to store data to disk and load it from there. 
	///
	/// \enum DataExportFormat 
	/// \ingroup ImportExport
	/// \brief Export format for any data represented by a #DataHandle.
	typedef enum DataExportFormat_ {
		/// Spectral Radar Metaformat, containing no data but many additinal parameters, such as spacing, size, etc.
		DataExport_SRM,
		/// RAW data format containing the data of the object as binary, single precision floating point values, little endian.
		DataExport_RAW,
		/// CSV (Comma Seperated Values) is a text file having all values stored, comma seperated and human readable.
		DataExport_CSV,
		/// TXT is a text file having all values stored space seperated and human readable.
		DataExport_TXT,
		/// TableTXT is a human readable text-file in a table like format, having the physical 1- and 2-axis as first two columns and the data value as third. Currently only works for 1D- and 2D-Data.
		DataExport_TableTXT,
		/// FITS Data format
		DataExport_Fits,
		/// VFF data format.
		DataExport_VFF,
		/// VTK data format.
		DataExport_VTK,
		/// TIFF Data format as 32-bit floating point numbers
		DataExport_TIFF
	} DataExportFormat;

	/// \enum ComplexDataExportFormat
	/// \ingroup ImportExport
	/// \brief Export format for complex data
	typedef enum {
		/// RAW data format containg binary data.
		ComplexDataExport_RAW
	} ComplexDataExportFormat;

	/// \enum ColoredDataExportFormat
	/// \ingroup ImportExport
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
	/// \brief Specifies a direction. In the default orientation, the first orientation is the Z-axis (parallel to the
	///        illumination-ray during the measurement), the second is the X-axis, and the third is the Y-axis.
	typedef enum {
		/// The 1-axis direction.
		Direction_1,
		/// The 2-axis direction.
		Direction_2,
		/// The 3-axis direction.
		Direction_3
	} Direction;

	/// \enum DataImportFormat
	/// \ingroup ImportExport
	/// \brief Supported import format to load data from disk.
	typedef enum {
		/// Spectral Radar Metaformat, containing no data but all additinal parameters, such as spacing, size, etc. 
		/// It is searched for an appropriate file with same name but different extension containg the according data.
		DataImport_SRM
	} DataImportFormat;

	/// \enum RawDataExportFormat
	/// \ingroup ImportExport
	/// \brief Supported raw data export formats to store data to disk.
	typedef enum {
		/// Single precision floating point raw data
		RawDataExport_RAW,
		/// Spectral Radar raw data format, specified additional information such as apodization scans, scan range, etc.
		RawDataExport_SRR
	} RawDataExportFormat;

	/// \enum RawDataImportFormat
	/// \ingroup ImportExport
	/// \brief Supported raw data import formats to load data from disk.
	typedef enum {
		/// Spectral Radar raw data-format, specified additional information such as apodization scans, scan range, etc.
		RawDataImport_SRR
	} RawDataImportFormat;

	/// \fn void exportData(DataHandle Data, DataExportFormat Format, const char* FileName);
	/// \ingroup ImportExport
	/// \brief Exports data (#DataHandle) to a file. The number of dimensions is handled automatically upon analysis of the first argument.
	/// \param[in] Data A valid (non null) data handle of the data (#DataHandle). These data may be multi-dimensional.
	/// \param[in] Format The desired data-format, to be selected among those in #DataExportFormat.
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                 should be escaped with an additional backslash.
	SPECTRALRADAR_API void exportData(DataHandle Data, DataExportFormat Format, const char* FileName);

	/// \fn void exportDataAsImage(DataHandle Data, ColoringHandle Color, ColoredDataExportFormat Format, Direction SliceNormalDirection, const char* FileName, int ExportOptionMask)
	/// \ingroup ImportExport
	/// \brief Exports 2-dimensional and 3-dimensional data (#DataHandle) as image data (such as BMP, PNG, JPEG, ...). 
	/// \param[in] Data A valid (non null) data handle of the data (#DataHandle). These data may be multi-dimensional.
	/// \param[in] Color A valid (non null) coloring handle (#ColoringHandle) as created, for example, with the functions
	///                  #createColoring32Bit() or #createCustomColoring32Bit().
	/// \param[in] Format The desired data-format, to be selected among those in #ColoredDataExportFormat.
	/// \param[in] SliceNormalDirection Specifies the direction normal to the generated pictures (to be chosen among those elements in #Direction).
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	/// \param[in] ExportOptionMask An OR-ed combination of the flags #ExportOption_None, #ExportOption_DrawScaleBar, #ExportOption_DrawMarkers,
	///                             and #ExportOption_UsePhysicalAspectRatio.
	SPECTRALRADAR_API void exportDataAsImage(DataHandle Data, ColoringHandle Color, ColoredDataExportFormat Format, Direction SliceNormalDirection, const char* FileName, int ExportOptionMask);

	/// \fn void exportComplexData(ComplexDataHandle Data, ComplexDataExportFormat Format, const char* FileName)
	/// \ingroup ImportExport
	/// \brief Exports 1-, 2- and 3-dimensional complex data (#ComplexDataHandle)
	/// \param[in] Data A valid (non null) complex-data handle of the data (#ComplexDataHandle). These data may be multi-dimensional.
	/// \param[in] Format The desired data-format, to be selected among those in #ComplexDataExportFormat.
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	SPECTRALRADAR_API void exportComplexData(ComplexDataHandle Data, ComplexDataExportFormat Format, const char* FileName);

	/// \fn void exportColoredData(ColoredDataHandle Data, ColoredDataExportFormat Format, Direction SliceNormalDirection, const char* FileName, int ExportOptionMask)
	/// \ingroup ImportExport
	/// \brief Exports colored data (#ColoredDataHandle).
	/// \param[in] Data A valid (non null) colored-data handle of the data (#ColoredDataHandle). These data may be multi-dimensional.
	/// \param[in] Format The desired data-format, to be selected among those in #ColoredDataExportFormat.
	/// \param[in] SliceNormalDirection Specifies the direction normal to the generated pictures (to be chosen among those elements in #Direction).
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	/// \param[in] ExportOptionMask An OR-ed combination of the flags #ExportOption_None, #ExportOption_DrawScaleBar, #ExportOption_DrawMarkers,
	///                             and #ExportOption_UsePhysicalAspectRatio.
	SPECTRALRADAR_API void exportColoredData(ColoredDataHandle Data, ColoredDataExportFormat Format, Direction SliceNormalDirection, const char* FileName, int ExportOptionMask);

	/// \fn void importColoredData(ColoredDataHandle ColoredData, DataImportFormat Format, const char* FileName)
	/// \ingroup ImportExport
	/// \brief Imports colored data (#ColoredDataHandle) with the specified format and copied it into a data object (#ColoredDataHandle)
	/// \param[out] ColoredData A valid (non null) colored-data handle of the data (#ColoredDataHandle). These data may be multi-dimensional.
	/// \param[in] Format The data-format stored in the file (the supported ones are the items of #DataImportFormat).
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	SPECTRALRADAR_API void importColoredData(ColoredDataHandle ColoredData, DataImportFormat Format, const char* FileName);

	/// \fn void importData(DataHandle Data, DataImportFormat Format, const char* FileName);
	/// \ingroup ImportExport
	/// \brief Imports data with the specified format and copies it into a data object (#DataHandle).
	/// \param[out] Data A valid (non null) data handle of the data (#DataHandle). These data may be multi-dimensional.
	/// \param[in] Format The data-format stored in the file (the supported ones are the items of #DataImportFormat).
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	SPECTRALRADAR_API void importData(DataHandle Data, DataImportFormat Format, const char* FileName);

	/// \fn  void exportRawData(RawDataHandle Raw, RawDataExportFormat Format, const char* FileName);
	/// \ingroup ImportExport
	/// \brief Exports the specified data to disk. 
	/// \param[in] Raw A valid (non null) raw-data handle of the data (#RawDataHandle).
	/// \param[in] Format The desired data-format to be stored in the file (the supported ones are the items of #RawDataExportFormat).
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void exportRawData(RawDataHandle Raw, RawDataExportFormat Format, const char* FileName);

	/// \fn void importRawData(RawDataHandle Raw, RawDataImportFormat Format, const char* FileName);
	/// \ingroup ImportExport
	/// \brief Imports the specified data from disk.
	/// \param[out] Raw A valid (non null) raw-data handle of the data (#RawDataHandle).
	/// \param[in] Format The data-format stored in the file (the supported ones are the items of #RawDataImportFormat).
	/// \param[in] FileName A zero-terminated string specifying the full pathname to the file to be written. Notice that backslashes
	///                     should be escaped with an additional backslash.
	///
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void importRawData(RawDataHandle Raw, RawDataImportFormat Format, const char* FileName);

	/*!
		\name ExportOptions
		Specifies additional export options to be used with functions such as #exportDataAsImage().
		Multiple options can be combined by bit-wise or ("|").
		Different options can be used for different export format. If an option is not supported by an export format, it is ignored. 
	*/
	///@{
	/// \ingroup ImportExport
	/// For default or no specific export options.
	const int ExportOption_None = 0x00000000;
	/// Draw scale bar on exported image.
	const int ExportOption_DrawScaleBar = 0x00000001;
	/// Draw markers on exported image.
	const int ExportOption_DrawMarkers = 0x00000002;
	/// Honor physical aspect ratio when exporting data (width and height of each pixel will have the same physical dimensions).
	const int ExportOption_UsePhysicalAspectRatio = 0x00000004;
	/// Flip X-axis
	const int ExportOption_Flip_X_Axis = 0x00000008;
	/// Flip Y-axis
	const int ExportOption_Flip_Y_Axis = 0x00000010;
	/// Flip Z-axis
	const int ExportOption_Flip_Z_Axis = 0x00000020;
	///@}

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

	/// \fn void appendRawData(RawDataHandle Raw, RawDataHandle DataToAppend, Direction Dir)
	/// \ingroup Volume
	/// \brief Appends the new raw data to the old raw data perpendicular to the specified direction.
	/// \param[in,out] Raw A valid (non null) raw-data handle of the existing data (#RawDataHandle), that will be expanded.
	/// \param[in] DataToAppend A valid (non null) raw-data handle of the new data (#RawDataHandle).  These raw-data will not be modified.
	/// \param[in] Dir The physical direction (#Direction) in which the new data will be appended.
	///                Currently the #Direction_1 (usually the Z-axis) is not supported and should not be specified.
	///
	/// Appending data implies expanding the number of data and also their physical range. These expansions are carried out
	/// automatically before the function returns.\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void appendRawData(RawDataHandle Raw, RawDataHandle DataToAppend, Direction Dir);

	/// \fn void getRawDataSliceAtIndex(RawDataHandle Raw, RawDataHandle Slice, Direction SliceNormalDirection, int Index)
	/// \ingroup Volume
	/// \brief Returns a slice of raw data perpendicular to the specified direction at the specified index.
	/// \param[in] Raw A valid (non null) raw-data handle of the existing, three-dimensional raw data (#RawDataHandle).
	///                These data will not be modified.
	/// \param[out] Slice A valid (non null) raw-data handle (#RawDataHandle), where the raw data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Index The desired slice number in the direction \p Dir.
	///
	/// The raw data that will be sliced (\p Raw) should be three-dimensional, that is, a sequence of B-scans. A slice is one of
	/// the B-scans, perpendicular to the third direction (usually the Y-axis).\n
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void getRawDataSliceAtIndex(RawDataHandle Raw, RawDataHandle Slice, Direction SliceNormalDirection, int Index);

	/// \fn double analyzeData(DataHandle Data, DataAnalyzation Selection);
	/// \ingroup Volume
	/// \brief Analyzes the given data, extracts the selected feature, and returns the computed value.
	/// \param[in] Data A valid (non null) data handle of the data (#DataHandle). These data may be multi-dimensional.
	/// \param[in] Selection The desired feature that should be computed (#DataAnalyzation).
	/// \return The value of the desired feature.
	SPECTRALRADAR_API double analyzeData(DataHandle Data, DataAnalyzation Selection);

	/// \fn double analyzeAScan(DataHandle Data, AScanAnalyzation Selection);
	/// \ingroup Volume
	/// \brief Analyzes the given A-scan data, extracts the selected feature, and returns the computed value.
	/// \param[in] Data A valid (non null) data handle of the A-scan (#DataHandle).
	/// \param[in] Selection The desired feature that should be computed (#AScanAnalyzation).
	/// \return The computed feature.
	///
	/// If the given data is multi-dimensional, only the first A-scan will be analyzed.
	SPECTRALRADAR_API double analyzeAScan(DataHandle Data, AScanAnalyzation Selection);

	/// \fn void analyzePeaksInAScan(DataHandle Data, AScanAnalyzation Selection, int NumberOfPeaksToAnalyze, int MinDistBetweenPeaks, double* Result)
	/// \ingroup Volume
	/// \brief Analyzes the given A-scan data, extracts the selected feature, and returns the computed value. It returns the result of multiple peaks compared to the function #analyzeAScan.
	/// \param[in] Data A valid (non null) data handle of the A-scan (#DataHandle).
	/// \param[in] Selection The desired feature that should be computed (#AScanAnalyzation).
	/// \param[in] NumberOfPeaksToAnalyze The number of peaks which should be analyzed. 
	/// \param[in] MinDistBetweenPeaks The minimum distance between the peaks in pixel.
	/// \param[out] Result An array of size NumberOfPeaksToAnalyze which contains the computed result. 
	///
	/// If the given data is multi-dimensional, only the first A-scan will be analyzed.
	SPECTRALRADAR_API void analyzePeaksInAScan(DataHandle Data, AScanAnalyzation Selection, int NumberOfPeaksToAnalyze, int MinDistBetweenPeaks, double* Result);

	/// \fn void transposeData(DataHandle DataIn, DataHandle DataOut)
	/// \ingroup Volume
	/// \brief Transposes the given data and writes the result to DataOut. First and second axes will be swaped.
	/// \param[in] DataIn A valid (non null) data handle of the input data (#DataHandle). These data should be multi-dimensional.
	///                   These data will not be modified.
	/// \param[out] DataOut A valid (non null) data handle of the output data (#DataHandle). These data will be a copy
	///                     of the input data, except that the first and the second axes will be swaped (usually Z- and X-axes).
	SPECTRALRADAR_API void transposeData(DataHandle DataIn, DataHandle DataOut);

	/// \fn void transposeDataInplace(DataHandle Data)
	/// \ingroup Volume
	/// \brief Transposes the given Data. First and second axes will be swaped.
	/// \param[in,out] Data A valid (non null) data handle of the data (#DataHandle). These data will be modified:
	///                     the first and the second axes will be swaped (usually Z- and X-axes).
	SPECTRALRADAR_API void transposeDataInplace(DataHandle Data);

	/// \fn void transposeAndScaleData(DataHandle DataIn, DataHandle DataOut, float Min, float Max)
	/// \ingroup Volume
	/// \brief Transposes the given data and writes the result to DataOut. First and second axes will be swaped, and the range
	///        of the entries will be scaled in such a way, that the range [Min,Max] will be mapped onto the range [0,1].
	/// \param[in] DataIn A valid (non null) data handle of the input data (#DataHandle). These data should be multi-dimensional.
	///                   These data will not be modified.
	/// \param[out] DataOut A valid (non null) data handle of the output data (#DataHandle). These data will be a scaled and 
	///                     transposed copy of the input data, that is, the first and the second axes will be swaped (usually
	///                     Z- and X-axes).
	/// \param[in] Min The lower bound of the data that will be mapped to 0 in \p DataOut.
	/// \param[in] Max The upper bound of the data that will be mapped to 1 in \p DataOut.
	SPECTRALRADAR_API void transposeAndScaleData(DataHandle DataIn, DataHandle DataOut, float Min, float Max);

	/// \fn void normalizeData(DataHandle Data, float Min, float Max)
	/// \ingroup Volume
	/// \brief Scales the given data in such a way, that the range [Min, Max] is mapped onto the range [0,1].
	/// \param[in,out] Data A valid (non null) data handle of the data (#DataHandle). These data will be scaled.
	/// \param[in] Min The lower bound of the data that will be mapped to 0 in \p DataOut.
	/// \param[in] Max The upper bound of the data that will be mapped to 1 in \p DataOut.
	SPECTRALRADAR_API void normalizeData(DataHandle Data, float Min, float Max);

	/// \fn void getDataSliceAtPos(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, double Pos_mm)
	/// \ingroup Volume
	/// \brief Returns a slice of data perpendicular to the specified direction at the specified position.
	/// \param[in] Data A valid (non null) data handle of the existing, three-dimensional data (#DataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) data handle (#DataHandle), where the data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                                 Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Pos_mm The position of the desired slice along the direction \p Dir, expressed in millimeter. The total
	///                   range of positions can be inquired with the function #getDataPropertyFloat, specifying the 
	///                   property #Data_Range2 or #Data_Range3 (depending on the \p Dir specified). If the scan pattern has
	///                   not been manipulated (e.g. shifted), the center position is 0 mm.
	///
	/// The data that will be sliced (\p Data) should be three-dimensional, e.g., a sequence of B-scans. A slice is one of the
	/// B-scans, perpendicular to the specified direction \p Dir. If a position intermediate between two measured B-scans is given,
	/// this function will pick the closest; no interpolation will take place.
	SPECTRALRADAR_API void getDataSliceAtPos(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, double Pos_mm);

	/// \fn void getComplexDataSlicePos(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, double Pos_mm);
	/// \ingroup Volume
	/// \brief Returns a slice of complex data perpendicular to the specified direction at the specified position.
	/// \param[in] Data A valid (non null) complex data handle of the existing, three-dimensional data (#ComplexDataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) complex data handle (#ComplexDataHandle), where the data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                                 Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Pos_mm The position of the desired slice along the direction \p Dir, expressed in millimeter. The total
	///                   range of positions can be inquired with the function #getDataPropertyFloat, specifying the 
	///                   property #Data_Range2 or #Data_Range3 (depending on the \p Dir specified). If the scan pattern has
	///                   not been manipulated (e.g. shifted), the center position is 0 mm.
	///
	/// The complex data that will be sliced (\p Data) should be three-dimensional, e.g., a sequence of B-scans. A slice is one of the
	/// B-scans, perpendicular to the specified direction \p Dir. If a position intermediate between two measured B-scans is given,
	/// this function will pick the closest; no interpolation will take place.
	SPECTRALRADAR_API void getComplexDataSlicePos(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, double Pos_mm);

	/// \fn void getColoredDataSlicePos(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, double Pos_mm)
	/// \ingroup Volume
	/// \brief Returns a slice of colored data perpendicular to the specified direction at the specified position.
	/// \param[in] Data A valid (non null) colored data handle of the existing, three-dimensional data (#ColoredDataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) complex data handle (#ComplexDataHandle), where the data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                                 Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Pos_mm The position of the desired slice along the direction \p Dir, expressed in millimeter. The total
	///                   range of positions can be inquired with the function #getDataPropertyFloat, specifying the 
	///                   property #Data_Range2 or #Data_Range3 (depending on the \p Dir specified). If the scan pattern has
	///                   not been manipulated (e.g. shifted), the center position is 0 mm.
	///
	/// The colored data that will be sliced (\p Data) should be three-dimensional, e.g., a sequence of B-scans. A slice is one of the
	/// B-scans, perpendicular to the specified direction \p Dir. If a position intermediate between two measured B-scans is given,
	/// this function will pick the closest; no interpolation will take place.
	SPECTRALRADAR_API void getColoredDataSlicePos(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, double Pos_mm);

	/// \fn void getDataSliceAtIndex(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, int Index)
	/// \ingroup Volume
	/// \brief Returns a slice of data perpendicular to the specified direction at the specified index.
	/// \param[in] Data A valid (non null) data handle of the existing, three-dimensional data (#DataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) data handle (#DataHandle), where the data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                                 Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Index The index of the desired slice along the direction \p Dir (zero-based, that is, the first slice is
	///                  0). The total number of slices can be obtained with the function #getDataPropertyInt, and specifying
	///                  the property #Data_Size2 or #Data_Size3 (depending on the \p Dir specified).
	///
	/// The data that will be sliced (\p data) should be three-dimensional, e.g., a sequence of B-scans. A slice is one of the
	/// B-scans, perpendicular to the specified direction \p Dir.
	SPECTRALRADAR_API void getDataSliceAtIndex(DataHandle Data, DataHandle Slice, Direction SliceNormalDirection, int Index);

	/// \fn void getComplexDataSliceIndex(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, int Index);
	/// \ingroup Volume
	/// \brief Returns a slice of complex data perpendicular to the specified direction at the specified index.
	/// \param[in] Data A valid (non null) complex data handle of the existing, three-dimensional data (#ComplexDataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) complex data handle (#ComplexDataHandle), where the data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                                 Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Index The index of the desired slice along the direction \p Dir (zero-based, that is, the first slice is
	///                  0). The total number of slices can be obtained with the function #getDataPropertyInt, and specifying
	///                  the property #Data_Size2 or #Data_Size3 (depending on the \p Dir specified).
	///
	/// The complex data that will be sliced (\p data) should be three-dimensional, e.g., a sequence of B-scans. A slice is one of the
	/// B-scans, perpendicular to the specified direction \p Dir.
	SPECTRALRADAR_API void getComplexDataSliceIndex(ComplexDataHandle Data, ComplexDataHandle Slice, Direction SliceNormalDirection, int Index);

	/// \fn void getColoredDataSliceIndex(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, int Index)
	/// \ingroup Volume
	/// \brief Returns a slice of colored data perpendicular to the specified direction at the specified index.
	/// \param[in] Data A valid (non null) colored data handle of the existing, three-dimensional data (#ColoredDataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) complex data handle (#ColoredDataHandle), where the data of the slice will be written.
	/// \param[in] SliceNormalDirection The physical direction (#Direction) in which the existing data will be sliced.
	///                                 Currently only the #Direction_3 (usually the Y-axis) is supported.
	/// \param[in] Index The index of the desired slice along the direction \p Dir (zero-based, that is, the first slice is
	///                  0). The total number of slices can be obtained with the function #getDataPropertyInt, and specifying
	///                  the property #Data_Size2 or #Data_Size3 (depending on the \p Dir specified).
	///
	/// The colored data that will be sliced (\p data) should be three-dimensional, e.g., a sequence of B-scans. A slice is one of the
	/// B-scans, perpendicular to the specified direction \p Dir.
	SPECTRALRADAR_API void getColoredDataSliceIndex(ColoredDataHandle Data, ColoredDataHandle Slice, Direction SliceNormalDirection, int Index);

	/// \fn void computeDataProjection(DataHandle Data, DataHandle Slice, Direction ProjectionDirection, DataAnalyzation Selection)
	/// \ingroup Volume
	/// \brief Returns a single slice of data, in which each pixel value is the feature extracted through an analysis along the specified
	///        direction.
	/// \param[in] Data A valid (non null) data handle of the existing, three-dimensional data (#DataHandle).
	///                 These data will not be modified.
	/// \param[out] Slice A valid (non null) data handle (#DataHandle), where the data of the slice will be written. Geometrically, this
	///                   slice is situated perpendicular to the specified #Direction.
	/// \param[in] ProjectionDirection The physical direction (#Direction) along which the provided data will be analyzed.
	/// \param[in] Selection The desired feature that should be extracted from the data along the specified direction.
	SPECTRALRADAR_API void computeDataProjection(DataHandle Data, DataHandle Slice, Direction ProjectionDirection, DataAnalyzation Selection);

	/// \fn void appendData(DataHandle Data, DataHandle DataToAppend, Direction Dir)
	/// \ingroup Volume
	/// \brief Appends the new data to the provided data, perpendicular to the specified direction.
	/// \param[in,out] Data A valid (non null) data handle of the existing data (#DataHandle), that will be expanded.
	/// \param[in] DataToAppend A valid (non null) data handle of the new data (#DataHandle). These data will not be modified.
	/// \param[in] Dir The physical direction (#Direction) along which the existing data will be expanded in order to accomodate
	///                the new data. Currently the #Direction_1 (usually the Z-axis) is not supported and should not be specified.
	///
	/// Appending data implies expanding the number of data and also their physical range. These expansions are carried out
	/// automatically before the function returns.
	SPECTRALRADAR_API void appendData(DataHandle Data, DataHandle DataToAppend, Direction Dir);

	/// \fn void appendComplexData(ComplexDataHandle Data, ComplexDataHandle DataToAppend, Direction Dir);
	/// \ingroup Volume
	/// \brief Appends the new data to the provided data, perpendicular to the specified direction.
	/// \param[in,out] Data A valid (non null) complex data handle of the existing data (#ComplexDataHandle), that will be expanded.
	/// \param[in] DataToAppend A valid (non null) complex data handle of the new data (#ComplexDataHandle). These data will not be modified.
	/// \param[in] Dir The physical direction (#Direction) along which the existing data will be expanded in order to accomodate
	///                the new data. Currently the #Direction_1 (usually the Z-axis) is not supported and should not be specified.
	///
	/// Appending data implies expanding the number of data and also their physical range. These expansions are carried out
	/// automatically before the function returns.
	SPECTRALRADAR_API void appendComplexData(ComplexDataHandle Data, ComplexDataHandle DataToAppend, Direction Dir);

	/// \fn void appendColoredData(ColoredDataHandle Data, ColoredDataHandle DataToAppend, Direction Dir)
	/// \ingroup Volume
	/// \brief Appends the new data to the provided data, perpendicular to the specified direction.
	/// \param[in,out] Data A valid (non null) colored data handle of the existing data (#ColoredDataHandle), that will be expanded.
	/// \param[in] DataToAppend A valid (non null) colored data handle of the new data (#ColoredDataHandle). These data will not be modified.
	/// \param[in] Dir The physical direction (#Direction) along which the existing data will be expanded in order to accomodate
	///                the new data. Currently the #Direction_1 (usually the Z-axis) is not supported and should not be specified.
	///
	/// Appending data implies expanding the number of data and also their physical range. These expansions are carried out
	/// automatically before the function returns.
	SPECTRALRADAR_API void appendColoredData(ColoredDataHandle Data, ColoredDataHandle DataToAppend, Direction Dir);

	/// \fn void cropData(DataHandle Data, Direction Dir, int IndexMax, int IndexMin)
	/// \ingroup Volume
	/// \brief Crops the data along the desired direction at the given indices. Upon return the data will only contain those slices
	///        whose indices where in the interval [IndexMin, IndexMax), counted along the cropping direction.
	/// \param[in,out] Data A valid (non null) data handle of the data (#DataHandle). These data will be cropped.
	/// \param[in] Dir The physical direction (#Direction) along which the existing data will be cropped.
	/// \param[in] IndexMax One-past-the-last slice that will be kept. This index is zero-based.
	/// \param[in] IndexMin The first slice that will be kept. This index is zero-based.
	SPECTRALRADAR_API void cropData(DataHandle Data, Direction Dir, int IndexMax, int IndexMin);

	/// \fn void cropComplexData(ComplexDataHandle Data, Direction Dir, int IndexMax, int IndexMin)
	/// \ingroup Volume
	/// \brief Crops the complex data along the desired direction at the given indices. Upon return the data will only contain those slices
	///        whose indices where in the interval [IndexMin, IndexMax), counted along the cropping direction.
	/// \param[in,out] Data A valid (non null) complex data handle of the data (#ComplexDataHandle). These data will be cropped.
	/// \param[in] Dir The physical direction (#Direction) along which the existing data will be cropped.
	/// \param[in] IndexMax One-past-the-last slice that will be kept. This index is zero-based.
	/// \param[in] IndexMin The first slice that will be kept. This index is zero-based.
	SPECTRALRADAR_API void cropComplexData(ComplexDataHandle Data, Direction Dir, int IndexMax, int IndexMin);

	/// \fn void cropColoredData(ColoredDataHandle Data, Direction Dir, int IndexMax, int IndexMin);
	/// \ingroup Volume
	/// \brief Crops the colored data along the desired direction at the given indices. Upon return the data will only contain those slices
	///        whose indices where in the interval [IndexMin, IndexMax), counted along the cropping direction.
	/// \param[in,out] Data A valid (non null) colored data handle of the data (#ColoredDataHandle). These data will be cropped.
	/// \param[in] Dir The physical direction (#Direction) along which the existing data will be cropped.
	/// \param[in] IndexMax One-past-the-last slice that will be kept. This index is zero-based.
	/// \param[in] IndexMin The first slice that will be kept. This index is zero-based.
	SPECTRALRADAR_API void cropColoredData(ColoredDataHandle Data, Direction Dir, int IndexMax, int IndexMin);

	/// \fn void separateData(DataHandle Data1, DataHandle Data2, int SeparationIndex, Direction Dir);
	/// \ingroup Volume
	/// \brief Separates the data at the given index at specific separation direction. The first part of the separated data will remain in Data1,
	///        the second separated in Data2.
	/// \param[in,out] Data1 A valid (non null) data handle of the data (#DataHandle). Upon return, only the first part will remain
	///                      in this container.
	/// \param[out] Data2 A valid (non null) data handle to the second part of the data (#DataHandle).
	/// \param[in] SeparationIndex The first slice of the second part, or one-past-the-last slice kept in the first part.
	/// \param[in] Dir The physical direction (#Direction) along which the separation will take place.
	SPECTRALRADAR_API void separateData(DataHandle Data1, DataHandle Data2, int SeparationIndex, Direction Dir);

	/// \fn void separateComplexData(ComplexDataHandle Data1, ComplexDataHandle Data2, int SeparationIndex, Direction Dir);
	/// \ingroup Volume
	/// \brief Separates the data at the given index at specific separation direction. The first part of the separated data will remain in Data1,
	///        the second separated in Data2.
	/// \param[in,out] Data1 A valid (non null) complex data handle of the data (#ComplexDataHandle). Upon return, only the first part will remain
	///                      in this container.
	/// \param[out] Data2 A valid (non null) complex data handle to the second part of the data (#ComplexDataHandle).
	/// \param[in] SeparationIndex The first slice of the second part, or one-past-the-last slice kept in the first part.
	/// \param[in] Dir The physical direction (#Direction) along which the separation will take place.
	SPECTRALRADAR_API void separateComplexData(ComplexDataHandle Data1, ComplexDataHandle Data2, int SeparationIndex, Direction Dir);

	/// \fn void separateColoredData(ColoredDataHandle Data1, ColoredDataHandle Data2, int SeparationIndex, Direction Dir);
	/// \ingroup Volume
	/// \brief Separates the data at the given index at specific separation direction. The first part of the separated data will remain in Data1,
	///        the second separated in Data2.
	/// \param[in,out] Data1 A valid (non null) colored data handle of the data (#ColoredDataHandle). Upon return, only the first part will remain
	///                      in this container.
	/// \param[out] Data2 A valid (non null) colored data handle to the second part of the data (#ColoredDataHandle).
	/// \param[in] SeparationIndex The first slice of the second part, or one-past-the-last slice kept in the first part.
	/// \param[in] Dir The physical direction (#Direction) along which the separation will take place.
	SPECTRALRADAR_API void separateColoredData(ColoredDataHandle Data1, ColoredDataHandle Data2, int SeparationIndex, Direction Dir);

	/// \fn void flipData(DataHandle Data, Direction FlippingDir);
	/// \ingroup Volume
	/// \brief Mirrors the data across a plane perpendicular to the given direction.
	/// \param[in,out] Data A valid (non null) data handle of the data (#DataHandle). These data will be flipped.
	/// \param[in] FlippingDir The physical direction (#Direction) along which the existing data will be flipped.
	SPECTRALRADAR_API void flipData(DataHandle Data, Direction FlippingDir);

	/// \fn void flipComplexData(ComplexDataHandle Data, Direction FlippingDir);
	/// \ingroup Volume
	/// \brief Mirrors the data across a plane perpendicular to the given direction.
	/// \param[in,out] Data A valid (non null) complex data handle of the data (#ComplexDataHandle). These data will be flipped.
	/// \param[in] FlippingDir The physical direction (#Direction) along which the existing data will be flipped.
	SPECTRALRADAR_API void flipComplexData(ComplexDataHandle Data, Direction FlippingDir);

	/// \fn void flipColoredData(ColoredDataHandle Data, Direction FlippingDir);
	/// \ingroup Volume
	/// \brief Mirrors the data across a plane perpendicular to the given direction.
	/// \param[in,out] Data A valid (non null) complex data handle of the data (#ComplexDataHandle). These data will be flipped.
	/// \param[in] FlippingDir The physical direction (#Direction) along which the existing data will be flipped.
	SPECTRALRADAR_API void flipColoredData(ColoredDataHandle Data, Direction FlippingDir);

	/// \fn ImageFieldHandle createImageField(void);
	/// \ingroup Volume
	/// \brief Creates an object holding image field data.
	/// \return A valid handle of the newly created image field (#ImageFieldHandle).
	SPECTRALRADAR_API ImageFieldHandle createImageField(void);

	/// \fn ImageFieldHandle createImageFieldFromProbe(ProbeHandle Probe);
	/// \ingroup Volume
	/// \brief Creates an object holding image field data from the specified Probe Handle.
	/// \param[in] Probe A valid (non null) probe handle (#ProbeHandle), previously generated with the function #initProbe.
	/// \return A valid handle of the newly created image field (#ImageFieldHandle).
	SPECTRALRADAR_API ImageFieldHandle createImageFieldFromProbe(ProbeHandle Probe);

	/// \fn void clearImageField(ImageFieldHandle ImageField);
	/// \ingroup Volume
	/// \brief Frees an object holding image field data.
	/// \param[in] ImageField A handle of the image field (#ImageFieldHandle). If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void clearImageField(ImageFieldHandle ImageField);

	/// \fn void saveImageField(ImageFieldHandle ImageField, const char* Path);
	/// \ingroup Volume
	/// \brief Saves data containing image field data.
	/// \param[in] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///            #createImageField or #createImageFieldFromProbe.
	/// \param[in] Path Filename (including path), where the data will be saved. If the file exists, it will be (merciless) overwritten.
	SPECTRALRADAR_API void saveImageField(ImageFieldHandle ImageField, const char* Path);

	/// \fn void loadImageField(ImageFieldHandle ImageField, const char* Path);
	/// \ingroup Volume
	/// \brief Loads data containing image field data.
	/// \param[out] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///             #createImageField or #createImageFieldFromProbe.
	/// \param[in] Path Filename (including path), where the data will be read from.
	SPECTRALRADAR_API void loadImageField(ImageFieldHandle ImageField, const char* Path);

	/// \fn void determineImageField(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Surface);
	/// \ingroup Volume
	/// \brief Determines the image field correction for the given surface data, previously measured with the given scan pattern.
	/// \param[out] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///                        #createImageField or #createImageFieldFromProbe.
	/// \param[in] Pattern A valid (non null) handle of a volume scan pattern (#ScanPatternHandle), created with one of the functions
	///                    #createVolumePattern, #createFreeformScanPattern3DFromLUT, or #createFreeformScanPattern3D. The scan pattern
	///                    should uniformly cover the whole field of view. \warning If the scan pattern is non uniform, or fails to cover some
	///                    areas, the resulting image field corrections will be impaired.
	///                    The scan pattern enables the conversion between index coordinates (i,j) and physical coordinates (in millimeter).
	///                    Hence it should be a scan pattern that covers the coordinates of the \p Surface (third parameter).
	/// \param[in] Surface A 2D data array, in a #DataHandle structure, whose entries are the depth of the surface at each (x,y) coordinate,
	///                    expressed in millimeter. The surface can be calculated from a volume scan using the function #determineSurface.
	///                    Notice that, unlike scans, the first coordinate is the x-axis and the second coordinate is the y-axis.
	///
	/// The purpose of the image field is to compensate the deformations introduced by the optical elements (e.g. lenses). To that end,
	/// a measurement of the substrate surface is carried out, and the geometric correction is computed. The default calibration of
	/// an instrument needs not be re-computed, unless a new objective is installed, or the objective is the same but the desired reference
	/// surface is non planar (the user must supply the desired surface, which should actually be measured).
	SPECTRALRADAR_API void determineImageField(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Surface);

	/// \fn void determineImageFieldWithMask(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Surface, DataHandle Mask);
	/// \ingroup Volume
	/// \brief Determines the image field correction for the given surface data, previously measured with the given scan pattern. The positive
	///        entries of the mask determine the points that actually enter in the computation.
	/// \param[out] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///                        #createImageField or #createImageFieldFromProbe.
	/// \param[in] Pattern A valid (non null) handle of a volume scan pattern (#ScanPatternHandle), created with one of the functions
	///                    #createVolumePattern, #createFreeformScanPattern3DFromLUT, or #createFreeformScanPattern3D. The scan pattern
	///                    should uniformly cover the whole field of view. \warning If the scan pattern is non uniform, or fails to cover some
	///                    areas, the resulting image field corrections will be impaired.
	///                    The scan pattern enables the conversion between index coordinates (i,j) and physical coordinates (in millimeter).
	///                    Hence it should be a scan pattern that covers the coordinates of the \p Surface (third parameter).
	/// \param[in] Surface A 2D data array, in a #DataHandle structure, whose entries are the depth of the surface at each (x,y) coordinate,
	///                    expressed in millimeter. The surface can be calculated from a volume scan using the function #determineSurface.
	///                    Notice that, unlike scans, the first coordinate is the x-axis and the second coordinate is the y-axis.
	/// \param[in] Mask A 2D array, in stored a #DataHandle structure, indicating which points of the Surface should be taken into account
	///                 (positive entries in \p Mask). Negative entries in \p Mask identify points of the \p Surface which should not be
	///                 considered in the computation. Notice that the entries are single-precision floating-point numbers. In case a 3D data
	///                 structure is passed, only the first slice will be used (index zero along the third #Direction).
	///
	/// The purpose of the image field is to compensate the deformations introduced by the optical elements (e.g. lenses). To that end,
	/// a measurement of the substrate surface is carried out, and the geometric correction is computed. The default calibration of
	/// an instrument needs not be re-computed, unless a new objective is installed, or the objective is the same but the desired reference
	/// surface is non planar (the user must supply the desired surface, which should actually be measured).\n
	/// This function checks that the first two dimensions (in pixels) of \p Surface and \p Mask match each other.
	SPECTRALRADAR_API void determineImageFieldWithMask(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Surface, DataHandle Mask);

	/// \fn void correctImageField(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Data)
	/// \ingroup Volume
	/// \brief Applies the image field correction to the given B-Scan or volume data.
	/// \param[in] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///                       #createImageField or #createImageFieldFromProbe. Besides, the image field has already been determined with the
	///                       help of the function #determineImageField or #determineImageFieldWithMask.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern (#ScanPatternHandle). This scan pattern should be the one used to
	///                    acquire the \p Data (third parameter), because the correction depends on the measurement coordinates.
	///                    The scan pattern enables the conversion between index coordinates (i,j) and physical coordinates (in millimeter).
	///                    Hence it should be a scan pattern that covers the coordinates of the \p Data (third parameter).
	/// \param[in,out] Data A valid (non null) handle of data (#DataHandle) pointing to data measured (acquired and processed) in a B-scan or in 
	///                     a volume scan.
	SPECTRALRADAR_API void correctImageField(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Data);

	/// \fn void correctImageFieldComplex(ImageFieldHandle ImageField, ScanPatternHandle Pattern, ComplexDataHandle Data)
	/// \ingroup Volume
	/// \brief Applies the image field correction to the complex B-Scan or volume complex data.
	/// \param[in] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///                       #createImageField or #createImageFieldFromProbe. Besides, the image field has already been determined with the
	///                       help of the function #determineImageField or #determineImageFieldWithMask.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern (#ScanPatternHandle). This scan pattern should be the one used to
	///                    acquire the \p Data (third parameter), because the correction depends on the measurement coordinates.
	///                    The scan pattern enables the conversion between index coordinates (i,j) and physical coordinates (in millimeter).
	///                    Hence it should be a scan pattern that covers the coordinates of the \p Data (third parameter).
	/// \param[in,out] Data A valid (non null) handle of complex data (#ComplexDataHandle) pointing to data measured (acquired and processed) in a
	///                     B-scan or in a volume scan.
	SPECTRALRADAR_API void correctImageFieldComplex(ImageFieldHandle ImageField, ScanPatternHandle Pattern, ComplexDataHandle Data);

	/// \fn void correctSurface(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Surface);
	/// \ingroup Volume
	/// \brief Applies the image field correction to the given Surface. Surface must contain depth values as a function of x/y coordinates.
	/// \param[in] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///                       #createImageField or #createImageFieldFromProbe. Besides, the image field has already been determined with the
	///                       help of the function #determineImageField or #determineImageFieldWithMask.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern (#ScanPatternHandle). The scan pattern enables the conversion between
	///                    index coordinates (i,j) and physical coordinates (in millimeter). Hence it should be a scan pattern that covers
	///                    the coordinates of the \p Surface (third parameter).
	/// \param[in,out] Surface A 2D data array, in a #DataHandle structure, whose entries are the depth of the surface at each (x,y) coordinate,
	///                        expressed in millimeter. This surface will be corrected. Notice that, unlike scans, the first coordinate is
	///                        the x-axis and the second coordinate is the y-axis.
	SPECTRALRADAR_API void correctSurface(ImageFieldHandle ImageField, ScanPatternHandle Pattern, DataHandle Surface);

	/// \fn void setImageFieldInProbe(ImageFieldHandle ImageField, ProbeHandle Probe);
	/// \ingroup Volume
	/// \brief Sets the specified image field to the specified Probe handle. Notice that no probe file will be automatically saved. 
	/// \param[in] ImageField A valid (non null) handle of the image field (#ImageFieldHandle), previously created with one of the functions
	///                       #createImageField or #createImageFieldFromProbe. Besides, the image field has already been determined with the
	///                       help of the function #determineImageField or #determineImageFieldWithMask.
	/// \param[in] Probe A valid (non null) probe handle (#ProbeHandle), previously generated with the function #initProbe.
	SPECTRALRADAR_API void setImageFieldInProbe(ImageFieldHandle ImageField, ProbeHandle Probe);

	/// \defgroup ProbeCalibration ProbeCalibration
	/// \brief Functionality to perform the probe calibration. Please use the ThorImageOCT software to perform probe calibrations, if necessary.
	/// \warning ThorImageOCT uses these functions to calibrate the galvo, assuming a very specific sequence of actions and conditions, as explained
	/// in the ThorImageOCT. For these functions to properly work, the user need to re-create the same sequence of actions and conditions.\n
	/// The galvo offset/factor / Draw & Scan overlay calibration assumes a sample with a triangular dot pattern with a fixed edge length
	/// which must be aligned parallel to the video image egdes.

	/// \fn VisualCalibrationHandle createVisualCalibration(OCTDeviceHandle Device, double TargetCornerLength_mm, BOOL CheckAngle, BOOL SaveData);
	/// \brief Creates handle used for visual calibration.
	/// \param[in] Device A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] TargetCornerLength_mm The length of the edge
	/// \param[in] CheckAngle A flag stating if the the sample's position is in a right angle with respect to the camera image (TRUE) or not (FALSE).
	/// \param[in] SaveData If TRUE, debug information will be dumped. Kindly say FALSE.
	/// \return A valid handle of a visual calibration (#VisualCalibrationHandle).
	///
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API VisualCalibrationHandle createVisualCalibration(OCTDeviceHandle Device, double TargetCornerLength_mm, BOOL CheckAngle, BOOL SaveData);

	/// \fn void clearVisualCalibration(VisualCalibrationHandle Handle);
	/// \brief Clear handle and frees all related memory. 
	/// \param[in] Handle A handle of a visual calibration (#VisualCalibrationHandle). If the handle is a nullptr, this function does nothing.
	///                   In most cases this handle will have been previously created with the function #createVisualCalibration.
	///
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API void clearVisualCalibration(VisualCalibrationHandle Handle);

	/// \fn BOOL visualCalibrate_1st_CameraScaling(VisualCalibrationHandle Handle, ProbeHandle Probe, ColoredDataHandle Image);
	/// \brief This is the first step in visual calibration. For this, the calibration the target needs to be placed under the objective.
	///        Returns TRUE if the first step succeeds.
	/// \param[in] Handle A handle of a valid (non null) visual calibration (#VisualCalibrationHandle), previously created with the function
	///                   #createVisualCalibration.
	/// \param[in] Probe A valid (non null) probe handle (#ProbeHandle), previously generated with the function #initProbe.
	/// \param[in] Image Video snapshot to use for calibration
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API BOOL visualCalibrate_1st_CameraScaling(VisualCalibrationHandle Handle, ProbeHandle Probe, ColoredDataHandle Image);

	/// \fn BOOL visualCalibrate_2nd_Galvo(VisualCalibrationHandle Handle, ProbeHandle Probe, ColoredDataHandle Image);
	/// \brief This is the second step in visual calibration. For this, the calibration target or and infrared vieweing card needs to be placed
	///        under the objective. Returns TRUE if the second step succeeds.
	/// \param[in] Handle A handle of a valid (non null) visual calibration (#VisualCalibrationHandle), previously created with the function
	///                   #createVisualCalibration.
	/// \param[in] Probe A valid (non null) probe handle (#ProbeHandle), previously generated with the function #initProbe.
	/// \param[in] Image Video snapshot to use for calibration
	///
	/// It is assumed that the function #visualCalibrate_1st_CameraScaling has been previously successfully invoked.\n
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API BOOL visualCalibrate_2nd_Galvo(VisualCalibrationHandle Handle, ProbeHandle Probe, ColoredDataHandle Image);

	/// \fn BOOL visualCalibrate_previewImage(VisualCalibrationHandle Handle, ColoredDataHandle Image);
	/// \brief Provides a preview image for the current calibration.
	/// \param[in] Handle A handle of a valid (non null) visual calibration (#VisualCalibrationHandle), previously created with the function
	///                   #createVisualCalibration.
	/// \param[out] Image A valid (non null) handle of colored data (#ColoredDataHandle). The preview image will be written here.
	///
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API BOOL visualCalibrate_previewImage(VisualCalibrationHandle Handle, ColoredDataHandle Image);

	/// \fn void visualCalibration_getHoles(VisualCalibrationHandle Handle, int* x0, int* y0, int* x1, int* y1, int* x2, int* y2);
	/// \brief provides currently located hole positions of the three-hole target.
	/// \param[in] Handle A handle of a valid (non null) visual calibration (#VisualCalibrationHandle), previously created with the function
	///                   #createVisualCalibration.
	/// \param[out] x0 The x-coordinate of the first hole.
	/// \param[out] y0 The y-coordinate of the first hole.
	/// \param[out] x1 The x-coordinate of the second hole.
	/// \param[out] y1 The y-coordinate of the second hole.
	/// \param[out] x2 The x-coordinate of the third hole.
	/// \param[out] y2 The y-coordinate of the third hole.
	///
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API void visualCalibration_getHoles(VisualCalibrationHandle Handle, int* x0, int* y0, int* x1, int* y1, int* x2, int* y2);

	/// \fn const char* visualCalibrate_Status(VisualCalibrationHandle Handle);
	/// \brief Gives a status message of the currently executed visual calibration. 
	/// \param[in] Handle A handle of a valid (non null) visual calibration (#VisualCalibrationHandle), previously created with the function
	///                   #createVisualCalibration.
	/// \return The status message of the currently executed visual calibration.
	///
	/// \warning ThorImageOCT uses this and other functions to calibrate the galvo, assuming a very specific sequence of actions and conditions,
	/// as explained in the ThorImageOCT. For this function to properly work, the user need to re-create the same sequence of actions and
	/// conditions. Please use the ThorImageOCT software to perform probe calibrations, if at all necessary.
	SPECTRALRADAR_API const char* visualCalibrate_Status(VisualCalibrationHandle Handle);

	/// \defgroup Doppler Doppler 
	/// \brief Doppler Processing Routines.
	///
	/// \enum DopplerPropertyInt
	/// \ingroup Doppler
	/// \brief Values that determine the behaviour of the Doppler processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		Doppler_Averaging_1,
		/// Averaging along the first axis, usually the first transversal axis (x)
		Doppler_Averaging_2,
		/// Step size for calculating the doppler processing in the longitudinal axis (z). Stride needs to be smaller or equal to Doppler_Averaging_1 and larger or equal to 1.
		Doppler_Stride_1,
		/// Step size for calculating the doppler processing in the transversal axis (x). Stride needs to be smaller or equal to Doppler_Averaging_2 and larger or equal to 1.
		Doppler_Stride_2
	} DopplerPropertyInt;

	/// \enum DopplerPropertyFloat
	/// \ingroup Doppler
	/// \brief Values that determine the behaviour of the Doppler processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		Doppler_RefractiveIndex,
		/// Scan Rate (in Hz) that was used to acquire the Doppler data to be processed. This is only required for computing the actual velocity scaling
		Doppler_ScanRate_Hz,
		/// Center Wavelength (in nanometers) that was used to acquire the Doppler data to be processed. This is only required for computing the actual velocity scaling
		Doppler_CenterWavelength_nm,
		/// Angle of the Doppler detection beam to the normal. This is only required for computing the actual velocity scaling
		Doppler_DopplerAngle_Deg
	} DopplerPropertyFloat;

	/// \enum DopplerFlag
	/// \ingroup Doppler
	/// \brief Flats that determine the behaviour of the Doppler processing routines.
	typedef enum {
		/// Averaging along the first axis, usually the longitudinal axis (z)
		Doppler_VelocityScaling,
	} DopplerFlag;

	/// \fn DopplerProcessingHandle createDopplerProcessing(void);
	/// \ingroup Doppler
	/// \brief Returns a handle for the use of Doppler-computation routines.
	/// \return #DopplerProcessingHandle to the created Doppler routines.
	SPECTRALRADAR_API DopplerProcessingHandle createDopplerProcessing(void);


	/// \fn DopplerProcessingHandle createDopplerProcessingForFile(OCTFileHandle File);
	/// \ingroup Doppler
	/// \brief Returns a handle for the use of Doppler-computation routines. The handle is created based on a saved OCT file.
	/// \return #DopplerProcessingHandle to the created Doppler routines. 	
	SPECTRALRADAR_API DopplerProcessingHandle createDopplerProcessingForFile(OCTFileHandle File);


	/// \fn int getDopplerPropertyInt(DopplerProcessingHandle Handle, DopplerPropertyInt Property)
	/// \ingroup Doppler
	/// \brief Gets the value of the given Doppler processing property.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Property The desired integer property (#DopplerPropertyInt).
	/// \return The value of the desired property.
	SPECTRALRADAR_API int getDopplerPropertyInt(DopplerProcessingHandle Handle, DopplerPropertyInt Property);

	/// \fn void setDopplerPropertyInt(DopplerProcessingHandle Handle, DopplerPropertyInt Property, int Value)
	/// \ingroup Doppler
	/// \brief Sets the value of the given Doppler processing property.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Property The selected integer property (#DopplerPropertyInt).
	/// \param[in] Value The desired value for the selected property.
	SPECTRALRADAR_API void setDopplerPropertyInt(DopplerProcessingHandle Handle, DopplerPropertyInt Property, int Value);

	/// \fn double getDopplerPropertyFloat(DopplerProcessingHandle Handle, DopplerPropertyFloat Property);
	/// \ingroup Doppler
	/// \brief Gets the value of the given Doppler processing property.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Property The desired floating-point property (#DopplerPropertyFloat).
	/// \return The value of the desired property.
	SPECTRALRADAR_API double getDopplerPropertyFloat(DopplerProcessingHandle Doppler, DopplerPropertyFloat Property);

	/// \fn void setDopplerPropertyFloat(DopplerProcessingHandle Handle, DopplerPropertyFloat Property, float Value);
	/// \ingroup Doppler
	/// \brief Sets the value of the given Doppler processing property.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Property The selected floating-point property (#DopplerPropertyFloat).
	/// \param[in] Value The desired value for the selected property.
	SPECTRALRADAR_API void setDopplerPropertyFloat(DopplerProcessingHandle Handle, DopplerPropertyFloat Property, float Value);

	/// \fn void getDopplerFlag(DopplerProcessingHandle Handle, DopplerFlag Flag);
	/// \ingroup Doppler
	/// \brief Gets the given Doppler processing flag.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Flag The desired boolean flag (#DopplerFlag).
	/// \return The boolen value of the selected flag.
	SPECTRALRADAR_API BOOL getDopplerFlag(DopplerProcessingHandle Handle, DopplerFlag Flag);

	/// \fn void setDopplerFlag(DopplerProcessingHandle Handle, DopplerFlag Flag, BOOL OnOff);
	/// \ingroup Doppler
	/// \brief Sets the given Doppler processing flag. 
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Flag The selected boolean flag (#DopplerFlag).
	/// \param[in] OnOff The desired boolean value for the selected flag.
	SPECTRALRADAR_API void setDopplerFlag(DopplerProcessingHandle Handle, DopplerFlag Flag, BOOL OnOff);

	/// \fn void setDopplerAmplitudeOutput(DopplerProcessingHandle Handle, DataHandle AmpOut)
	/// \ingroup Doppler
	/// \brief Sets the location of the resulting Doppler amplitude output.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] AmpOut A valid (non null) handle of data (#DataHandle), where the resulting amplitudes of the Doppler computation
	///                   will be written. The right number of dimensions, sizes, and ranges will be automatically adjusted by
	///                   the function #executeDopplerProcessing.
	SPECTRALRADAR_API void setDopplerAmplitudeOutput(DopplerProcessingHandle Handle, DataHandle AmpOut);

	/// \fn void setDopplerPhaseOutput(DopplerProcessingHandle Handle, DataHandle PhasesOut)
	/// \ingroup Doppler
	/// \brief Sets the location of the resulting Doppler phase output.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] PhasesOut A valid (non null) handle of data (#DataHandle), where the resulting phases of the Doppler computation
	///                     will be written. The right number of dimensions, sizes, and ranges will be automatically adjusted by
	///                     the function #executeDopplerProcessing.
	SPECTRALRADAR_API void setDopplerPhaseOutput(DopplerProcessingHandle Handle, DataHandle PhasesOut);

	/// \fn void executeDopplerProcessing(DopplerProcessingHandle Handle, ComplexDataHandle Input)
	/// \ingroup Doppler
	/// \brief Executes the Doppler processing of the input data and returns phases and amplitudes.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Input A valid (non null) handle of complex data (#ComplexDataHandle). These data should have previously obtained
	///                  by invoking the functions #createComplexData, #setComplexDataOutput and #executeProcessing.
	///
	/// Doppler procesing takes place after the standard processing. It takes as input complex data computed by the standard
	/// processing, and during execution it writes amplitudes and phases, provided either 8or both) of the function 
	/// #setDopplerAmplitudeOutput or #setDopplerPhaseOutput have previously been invoked.
	SPECTRALRADAR_API void executeDopplerProcessing(DopplerProcessingHandle Handle, ComplexDataHandle Input);

	/// \fn void dopplerPhaseToVelocity(DopplerProcessingHandle Handle, DataHandle InOut)
	/// \ingroup Handle
	/// \brief Scales phases computed by Doppler OCT to actual flow velocities in scan direction
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in,out] InOut A handle of data representing first phase data that will then be modified to conttain velocity data.
	///
	/// This requires the Doppler scan rate, Doppler angle and center velocity of the Doppler object to be set correctly.
	SPECTRALRADAR_API void dopplerPhaseToVelocity(DopplerProcessingHandle Doppler, DataHandle InOut);

	/// \fn void dopplerVelocityToPhase(DopplerProcessingHandle Handle, DataHandle InOut);
	/// \ingroup Doppler
	/// \brief Scales flow velocities computed by Doppler OCT back to original phase differencees
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in,out] InOut A handle of data representing first velocity data that will then be modified to conttain velocity data.
	///
	/// This requires the Doppler scan rate, Doppler angle and center velocity of the Doppler object to be set correctly.
	SPECTRALRADAR_API void dopplerVelocityToPhase(DopplerProcessingHandle Doppler, DataHandle InOut);

	/// \fn void clearDopplerProcessing(DopplerProcessingHandle Handle)
	/// \ingroup Doppler
	/// \brief Closes the Doppler processing routines and frees the memory that has been allocated for these to work properly.
	/// \param[in] Handle A handle of Doppler processing routines (#DopplerProcessingHandle). If the handle is a nullptr, this function does nothing.
	///                   In most cases this handle will have been previously obtained with the function #createDopplerProcessing.
	SPECTRALRADAR_API void clearDopplerProcessing(DopplerProcessingHandle Handle);

	/// \fn void getDopplerOutputSize(DopplerProcessingHandle Handle, int Size1In, int Size2In, int* Size1Out, int* Size2Out);
	/// \ingroup Doppler
	/// \brief Returns the final size of the Doppler output if executeDopplerProcessing is executed using data of the specified input size.
	/// \param[in] Handle A valid (non null) handle of Doppler processing routines (#DopplerProcessingHandle), obtained with the
	///                   function #createDopplerProcessing.
	/// \param[in] Size1In The value of the #Data_Size1 property (#DataPropertyInt) of the complex-data that will be used as input.
	///                    In the default orientation, this is the number of pixels along the z-axis.
	/// \param[in] Size2In The value of the #Data_Size2 property (#DataPropertyInt) of the complex-data that will be used as input.
	///                    In the default orientation, this is the number of pixels along the x-axis.
	/// \param[out] Size1Out The value of the #Data_Size1 property (#DataPropertyInt) of the amplitude/phase data that will result
	///                      upon invokation of the function #executeDopplerProcessing. In the default orientation, this is the number
	///                      of pixels along the z-axis.
	/// \param[out] Size2Out The value of the #Data_Size2 property (#DataPropertyInt) of the amplitude/phase data that will result
	///                      upon invokation of the function #executeDopplerProcessing. In the default orientation, this is the number
	///                      of pixels along the x-axis.
	SPECTRALRADAR_API void getDopplerOutputSize(DopplerProcessingHandle Handle, int Size1In, int Size2In, int* Size1Out, int* Size2Out);

	/// \defgroup Service Service
	/// \brief Service functions for additional analyzing of OCT functionality.
	///
	/// \fn void calcContrast(DataHandle ApodizedSpectrum, DataHandle Contrast)
	/// \ingroup Service
	/// \brief Computes the contrast for the specified (apodized) spectrum.
	/// \param[in] ApodizedSpectrum The spectrum after offset substraction and apodization. This spectrum can be obtained
	///                             using the functions #setApodizedSpectrumOutput and #executeProcessing in sucession.
	/// \param[out] Contrast A valid (non null) data handle (#DataHandle). Its dimensions will be automatically be adjusted.
	///
	/// The contrast is a measure of the amount of information in the interference pattern as a fraction of the
	/// total signal. The computed values are expressed as percentage of the measured amplitudes, for each camera pixel.
	SPECTRALRADAR_API void calcContrast(DataHandle ApodizedSpectrum, DataHandle Contrast);

	/// \defgroup Settings Settings
	/// \brief Direct access to INI files and settings.
	///
	/// \typedef SettingsHandle
	/// \ingroup Settings
	/// \brief Handle for saving settings on disk.
	struct C_Settings;
	typedef struct C_Settings* SettingsHandle;

	/// \fn SettingsHandle initSettingsFile(const char*);
	/// \ingroup Settings
	/// \brief Loads a settings file (usually *.ini); and prepares its properties to be read.
	SPECTRALRADAR_API SettingsHandle initSettingsFile(const char* Path);

	/// \fn int getSettingsEntryInt(SettingsHandle, const char*, int);
	/// \ingroup Settings
	/// \brief Gets an integer number from the specified ini file (see #SettingsHandle and #initSettingsFile);.
	SPECTRALRADAR_API int getSettingsEntryInt(SettingsHandle SettingsFile, const char* Node, int DefaultValue);

	/// \fn double getSettingsEntryFloat(SettingsHandle, const char*, double);
	/// \ingroup Settings
	/// \brief Gets an floating point number from the specified ini file (see #SettingsHandle and #initSettingsFile);.
	SPECTRALRADAR_API double getSettingsEntryFloat(SettingsHandle SettingsFile, const char* Node, double DefaultValue);

	/// \fn void getSettingsEntryFloatArray(SettingsHandle SettingsFile, const char* Node, const double* DefaultValues, double* Values, int* Size);
	/// \ingroup Settings
	/// \brief Gets an array of floating point numbers from the specified ini file (see #SettingsHandle and #initSettingsFile);.
	SPECTRALRADAR_API void getSettingsEntryFloatArray(SettingsHandle SettingsFile, const char* Node, const double* DefaultValues, double* Values, int* Size);

	/// \fn const char* getSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Default);
	/// \ingroup Settings
	/// \brief Gets a string from the specified ini file (see #SettingsHandle and #initSettingsFile);. The resulting const char* ptr will be valid until the settings file is closed by #closeSettingsFile). 
	SPECTRALRADAR_API const char* getSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Default);

	/// \fn void setSettingsEntryInt(SettingsHandle SettingsFile, const char* Node, int Value);
	/// \ingroup Settings
	/// \brief Sets an integer entry in the specified ini file (see #SettingsHandle and #initSettingsFile);.
	SPECTRALRADAR_API void setSettingsEntryInt(SettingsHandle SettingsFile, const char* Node, int Value);

	/// \fn void setSettingsEntryFloat(SettingsHandle SettingsFile, const char* Node, double Value);
	/// \ingroup Settings
	/// \brief Sets a floating point entry in the specified ini file (see #SettingsHandle and #initSettingsFile);.
	SPECTRALRADAR_API void setSettingsEntryFloat(SettingsHandle SettingsFile, const char* Node, double Value);

	/// \fn void setSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Value);
	/// \ingroup Settings
	/// \brief Sets a string in the specified ini file (see #SettingsHandle and #initSettingsFile);.
	SPECTRALRADAR_API void setSettingsEntryString(SettingsHandle SettingsFile, const char* Node, const char* Value);

	/// \fn void saveSettings(SettingsHandle)
	/// \ingroup Settings
	/// \brief Saves the changes to the specified Settings file.
	SPECTRALRADAR_API void saveSettings(SettingsHandle SettingsFile);

	/// \fn void closeSettingsFile(SettingsHandle Handle);
	/// \ingroup Settings
	/// \brief Closes the specified ini file and stores the set entries (\sa #SettingsHandle, #initSettingsFile).
	/// \param[in] Handle A handle of settings (#SettingsHandle). If the handle is a nullptr, this function does nothing.
	///                   In most cases this handle will have been previously obtained with the function #initSettingsFile.
	SPECTRALRADAR_API void closeSettingsFile(SettingsHandle Handle);

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
		/// Green, blue and black is used as one half of a Doppler color scheme
		ColorScheme_GreenBlueAndBlack = 9,
		/// Black, red, and yellow  is used as one half of a Doppler color scheme
		ColorScheme_BlackAndRedYellow = 10,
		/// Transparent and white coloring for overlay and 3D volume rendering purposes
		ColorScheme_TransparentAndWhite = 11,
		/// Green, blue, White, Red, and Yellow for polarization sensitive measurements
		ColorScheme_GreenBlueWhiteRedYellow = 12,
		/// Blue, green, black, yellow, and red for polarization sensitive measurements
		ColorScheme_BlueGreenBlackYellowRed = 13,
		/// Red, green, and blue for polarization sensitive measurements
		ColorScheme_RedGreenBlue = 14,
		/// Green, blue and red for polarization sensitive measurements
		ColorScheme_GreenBlueRed = 15,
		/// Blue, red, and green for polarization sensitive measurements
		ColorScheme_BlueRedGreen = 16,
		/// Green, blue and red for polarization sensitive measurements
		ColorScheme_GreenBlueRedGreen = 17,
		/// Blue, red, and green for polarization sensitive measurements
		ColorScheme_BlueRedGreenBlue = 18,
		/// Red, green, and blue for polarization sensitive measurements
		ColorScheme_Inverse_RedGreenBlue = 19,
		/// Green, blue and red for polarization sensitive measurements
		ColorScheme_Inverse_GreenBlueRed = 20,
		/// Blue, red, and green for polarization sensitive measurements
		ColorScheme_Inverse_BlueRedGreen = 21,
		/// Green, blue and red for polarization sensitive measurements
		ColorScheme_Inverse_GreenBlueRedGreen = 22,
		/// Blue, red, and green for polarization sensitive measurements
		ColorScheme_Inverse_BlueRedGreenBlue = 23,
		/// Red, yellow, green, blue, and red for polarization sensitive measurements
		ColorScheme_RedYellowGreenBlueRed = 24,
		/// Red, green, blue, and red for polarization sensitive measurements
		ColorScheme_RedGreenBlueRed = 25,
		/// Red, green, blue, and red for polarization sensitive measurements
		ColorScheme_Inverse_RedGreenBlueRed = 26,
		/// Red, yellow, and blue
		ColorScheme_RedYellowBlue = 27,
		/// Red, yellow, and blue
		ColorScheme_Inverse_RedYellowBlue = 28,
		/// DEM
		ColorScheme_DEM_Normal = 29,
		ColorScheme_Inverse_DEM_Normal = 30,
		/// DEM
		ColorScheme_DEM_Blind = 31,
		ColorScheme_Inverse_DEM_Blind = 32,
		ColorScheme_WhiteBlackWhite = 33,
		ColorScheme_BlackWhiteBlack = 34
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

	/// \fn ColoringHandle createColoring32Bit(ColorScheme, ColoringByteOrder)
	/// \ingroup Coloring
	/// \brief Creates processing that can be used to color given floating point B-scans to 32 bit colored images.
	/// \param Color The color-table to be used
	/// \param ByteOrder The byte order the coloring is supposed to use. 
	/// \return The handle (#ColoringHandle) to the coloring algorithm. 
	SPECTRALRADAR_API ColoringHandle createColoring32Bit(ColorScheme Color, ColoringByteOrder ByteOrder);

	/// \fn ColoringHandle createCustomColoring32Bit(int LUTSize, unsigned long* LUT);
	/// \ingroup Coloring
	/// \brief Create custom coloring using the specified color look-up-table. 
	SPECTRALRADAR_API ColoringHandle createCustomColoring32Bit(int LUTSize, unsigned long* LUT);

	/// \fn void setColoringBoundaries(ColoringHandle, float, float)
	/// \ingroup Coloring
	/// \brief Sets the boundaries in dB which are used by the coloring algorithm to map colors to floating point values in dB.
	SPECTRALRADAR_API void setColoringBoundaries(ColoringHandle Colorng, float Min_dB, float Max_dB);

	/// \fn void setColoringEnhancement(ColoringHandle, ColorEnhancement)
	/// \ingroup Coloring
	/// \brief Selects a function for non-linear coloring to enhance (subjective) image impression.
	SPECTRALRADAR_API void setColoringEnhancement(ColoringHandle Coloring, ColorEnhancement Enhancement);

	/// \fn void colorizeData(ColoringHandle Coloring, DataHandle Data, ColoredDataHandle ColoredData, BOOL Transpose)
	/// \ingroup Coloring
	/// \brief Colors a given data object (#DataHandle) into a given colored object (#ColoredDataHandle).
	SPECTRALRADAR_API void colorizeData(ColoringHandle Coloring, DataHandle Data, ColoredDataHandle ColoredData, BOOL Transpose);

	/// \fn oid colorizeDopplerData(ColoringHandle AmpColoring, ColoringHandle PhaseColoring, DataHandle AmpData, 
	/// DataHandle PhaseData, ColoredDataHandle Output, double MinSignal_dB, BOOL Transpose); 
	/// \ingroup Coloring
	/// \brief Colors a two given data object (#DataHandle) using overlay and intensity to represent phase and amplitude data. Used for Doppler imaging.
	SPECTRALRADAR_API void colorizeDopplerData(ColoringHandle AmpColoring, ColoringHandle PhaseColoring, DataHandle AmpData, DataHandle PhaseData, ColoredDataHandle Output, double MinSignal_dB, BOOL Transpose);

	/// \fn oid colorizeDopplerDataEx(ColoringHandle AmpColoring, ColoringHandle PhaseColoring[2], DataHandle AmpData, 
	/// DataHandle PhaseData, ColoredDataHandle Output, double MinSignal_dB, BOOL Transpose); 
	/// \ingroup Coloring
	/// \brief Colors a two given data object (#DataHandle) using overlay and intensity to represent phase and amplitude data. Used for Doppler imaging. In the extended version, two ColoringHandles can be specified, two provide different coloring for increasing and decreasing phase, for example.
	SPECTRALRADAR_API void colorizeDopplerDataEx(ColoringHandle AmpColoring, ColoringHandle PhaseColoring[2], DataHandle AmpData, DataHandle PhaseData, ColoredDataHandle Output, double MinSignal_dB, BOOL Transpose);

	/// \fn void clearColoring(ColoringHandle Handle)
	/// \ingroup Coloring
	/// \brief Clears the coloring previously created by #createColoring32Bit.
	/// \param[in] Handle A handle of a coloring (#ColoringHandle). If the handle is a nullptr, this function does nothing.
	///                   In most cases this handle will have been previously obtained with the function #createColoring32Bit.
	SPECTRALRADAR_API void clearColoring(ColoringHandle Handle);

	/// \defgroup Camera Camera
	/// \brief Functions for acquiring camera video images.
	///

	/// \fn void getMaxCameraImageSize(OCTDeviceHandle, int*, int*)
	/// \ingroup Camera
	/// \brief Returns the maximum possible camera image size for the current device.
	SPECTRALRADAR_API void getMaxCameraImageSize(OCTDeviceHandle Dev, int* SizeX, int* SizeY);

	/// \fn void getCameraImage(OCTDeviceHandle, ColoredDataHandle)
	/// \ingroup Camera
	/// \brief Gets a camera image.
	SPECTRALRADAR_API void getCameraImage(OCTDeviceHandle Dev, ColoredDataHandle Image);


	// HELPER

	/// \defgroup Helper Helper function
	/// \brief Functions for chores common to many categories and scenarios.

	/// \fn unsigned long InterpretReferenceIntensity(float);
	/// \ingroup Helper
	/// \brief interprets the reference intensity and gives a color code that reflects its state. 
	/// 
	/// Possible colors include: 
	/// - red = 0x00FF0000 (bad intensity);
	/// - orange = 0x00FF7700 (okay intensity);
	/// - green = 0x0000FF00 (good intensity);
	/// \param intensity the current reference intensity as a value between 0.0 and 1.0
	/// \return the color code reflecting the state of the reference intensity	
	SPECTRALRADAR_API unsigned long InterpretReferenceIntensity(float intensity);

	/// \fn unsigned long InterpretReferenceIntensitySingleValue(float DesiredIntensity, float Tolerance, float CurrentIntensity)
	/// \ingroup Helper
	/// \brief interprets the reference intensity and gives green color code when the reference intensity is the DesiredIntensity plus/minus the Tolerance, otherwise red is returned. 
	/// 
	/// Possible colors include: 
	/// - red = 0x00FF0000 (bad intensity);
	/// - green = 0x0000FF00 (good intensity);
	/// \param[in] DesiredIntensity the desired reference intensity as a value between 0.0 and 1.0
	/// \param[in] Tolerance the specified reference intensity tolerance as a value between 0.0 and 1.0
	/// \param[in] CurrentIntensity the current reference intensity as a value between 0.0 and 1.0
	/// \return the color code reflecting the state of the reference intensity	
	SPECTRALRADAR_API unsigned long InterpretReferenceIntensitySingleValue(float DesiredIntensity, float Tolerance, float CurrentIntensity);

	/// \fn void getConfigPath(char* Path, int StrSize);
	/// \ingroup Helper
	/// \brief Returns the path that hold the config files.
	SPECTRALRADAR_API void getConfigPath(char* Path, int StrSize);

	/// \fn void getPluginPath(char* Path, int StrSize);
	/// \ingroup Helper
	/// \brief Returns the path that hold the plugins.
	SPECTRALRADAR_API void getPluginPath(char* Path, int StrSize);

	/// \fn void getInstallationPath(char* Path, int StrSize);
	/// \ingroup Helper
	/// \brief Returns the installation path.
	SPECTRALRADAR_API void getInstallationPath(char* Path, int StrSize);

	/// \fn double getReferenceIntensity(ProcessingHandle Proc)
	/// \ingroup Helper
	/// \brief Returns an absolute value that indicates the refernce intensity that was present when the currently used apodization was determined.
	SPECTRALRADAR_API double getReferenceIntensity(ProcessingHandle Proc);

	/// \fn double double getRelativeReferenceIntensity(OCTDeviceHandle Dev, ProcessingHandle Proc)
	/// \ingroup Helper
	/// \brief Returns a value larger than 0.0 and smaller than 1.0 that indicates the reference intensity (relative to saturation) that was present when the currently used apodization was determined.
	SPECTRALRADAR_API double getRelativeReferenceIntensity(OCTDeviceHandle Dev, ProcessingHandle Proc);

	/// \fn double double getRelativeSaturation(ProcessingHandle Proc)
	/// \ingroup Helper
	/// \brief Returns a value larger than 0.0 and smaller than 1.0 that indicates the saturation of the sensor that was present during the last processing cycle.
	SPECTRALRADAR_API double getRelativeSaturation(ProcessingHandle Proc);

	/// \defgroup Buffer Buffer
	/// \brief Functions for acquiring camera video images.
	///
	/// \fn BufferHandle createMemoryBuffer()
	/// \ingroup Buffer
	/// \brief Creates a buffer holding data and colored data.
	SPECTRALRADAR_API BufferHandle createMemoryBuffer(void);

	/// \fn void appendToBuffer(BufferHandle, DataHandle, ColoredDataHandle)
	/// \ingroup Buffer
	/// \brief Appends specified data and colored data to the requested buffer. 
	///
	/// If insufficient memory is availalbe the oldest items in the buffer will be freed automatically. 
	SPECTRALRADAR_API void appendToBuffer(BufferHandle, DataHandle, ColoredDataHandle);

	/// \fn void purgeBuffer(BufferHandle)
	/// \ingroup Buffer
	/// \brief Discards all data.
	SPECTRALRADAR_API void purgeBuffer(BufferHandle);

	/// \fn int getBufferSize(BufferHandle)
	/// \ingroup Buffer
	/// \brief Returns the currently avaiable data sets in the buffer.
	SPECTRALRADAR_API int getBufferSize(BufferHandle);

	/// \fn int getBufferFirstIndex(BufferHandle)
	/// \ingroup Buffer
	/// \brief Returns the index of the first data sets available in the buffer.
	SPECTRALRADAR_API int getBufferFirstIndex(BufferHandle);

	/// \fn int getBufferLastIndex(BufferHandle)
	/// \ingroup Buffer
	/// \brief Returns the index of one past the last data sets available in the buffer.
	SPECTRALRADAR_API int getBufferLastIndex(BufferHandle);

	/// \fn DataHandle getBufferData(BufferHandle, int Index)
	/// \ingroup Buffer
	/// \brief Returns the data in the buffer.
	SPECTRALRADAR_API DataHandle getBufferData(BufferHandle, int Index);

	/// \fn ColoredDataHandle getColoredBufferData(BufferHandle, int Index)
	/// \ingroup Buffer
	/// \brief Returns the colored data in the buffer.
	SPECTRALRADAR_API ColoredDataHandle getColoredBufferData(BufferHandle, int Index);

	/// \fn void clearBuffer(BufferHandle BufferHandle)
	/// \ingroup Buffer
	/// \brief Clears the buffer and frees all data and colored data objects in it. 
	/// \param[in] BufferHandle A handle of a buffer (#BufferHandle). If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void clearBuffer(BufferHandle BufferHandle);

	/// \defgroup OutputValues Output Values (digital or analog)
	/// \brief Functions to inquire, setup and generate output values. Whether this functionality is supported, and to what extent, depends on the hardware.

	/// \fn int getNumberOfOutputDeviceValues(OCTDeviceHandle Dev)
	/// \ingroup OutputValues
	/// \brief Returns the number of output values.
	SPECTRALRADAR_API int getNumberOfOutputDeviceValues(OCTDeviceHandle Dev);

	/// \fn void getOutputDeviceValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);
	/// \ingroup OutputValues
	/// \brief Returns names and units of the requested output values.
	SPECTRALRADAR_API void getOutputDeviceValueName(OCTDeviceHandle Dev, int Index, char* Name, int NameStringSize, char* Unit, int UnitStringSize);

	/// \fn BOOL doesOutputDeviceValueExist(OCTDeviceHandle Dev, const char* Name);
	/// \ingroup OutputValues
	/// \brief Returns whether the requested output device values exists or not.
	SPECTRALRADAR_API BOOL doesOutputDeviceValueExist(OCTDeviceHandle Dev, const char* Name);

	/// \fn void setOutputDeviceValueByName(OCTDeviceHandle Dev, const char* Name, double value);
	/// \ingroup OutputValues
	/// \brief Sets the specified output value.
	SPECTRALRADAR_API void setOutputDeviceValueByName(OCTDeviceHandle Dev, const char* Name, double value);

	/// \fn void setOutputValueByIndex(OCTDeviceHandle Dev, int Index, double Value);
	/// \ingroup OutputValues
	/// \brief Sets the specified output value.
	SPECTRALRADAR_API void setOutputValueByIndex(OCTDeviceHandle Dev, int Index, double Value);

	/// \fn void getOutputDeviceValueRangeByName(OCTDeviceHandle Dev, const char* Name, double* Min, double* Max);
	/// \ingroup OutputValues
	/// \brief Gives the range of the specified output value.
	SPECTRALRADAR_API void getOutputDeviceValueRangeByName(OCTDeviceHandle Dev, const char* Name, double* Min, double* Max);

	/// \fn void getOutputValueRangeByIndex(OCTDeviceHandle Dev, int Index, double* Min, double* Max);
	/// \ingroup OutputValues
	/// \brief Gives the range of the specified output value.
	SPECTRALRADAR_API void getOutputValueRangeByIndex(OCTDeviceHandle Dev, int Index, double* Min, double* Max);

	/// \fn void computeLinearKRawData(ComplexDataHandle ComplexDataAfterFFT, DataHandle LinearKData);
	/// \ingroup Processing
	/// \brief Computes the linear k raw data of the complex data after FFT by an inverse Fourier transform.
	SPECTRALRADAR_API void computeLinearKRawData(ComplexDataHandle ComplexDataAfterFFT, DataHandle LinearKData);

	/// \fn void linearizeSpectralData(DataHandle SpectraIn, DataHandle SpectraOut, DataHandle Chirp);
	/// \ingroup Processing
	/// \brief Linearizes the spectral data using the given chirp vector.
	SPECTRALRADAR_API void linearizeSpectralData(DataHandle SpectraIn, DataHandle SpectraOut, DataHandle Chirp);

	/// \defgroup filehandling File Handling

	/// \enum OCTFileFormat
	/// \ingroup filehandling
	/// \brief Enum identifying possible file formats
	typedef enum _OCTFileFormat {
		FileFormat_OCITY,
		FileFormat_IMG,
		FileFormat_SDR,
		FileFormat_SRM,
		FileFormat_TIFF32
	} OCTFileFormat;


	/// \enum DataObjectType
	/// \ingroup filehandling
	/// \brief Enum identifying 
	typedef enum _DataObjectType {
		DataObjectType_Real,
		DataObjectType_Colored,
		DataObjectType_Complex,
		DataObjectType_Raw,
		DataObjectType_Binary,
		DataObjectType_Text,
		DataObjectType_Unknown = 999
	} DataObjectType;


	/// \enum FileMetadataFloat
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of floating point type
	typedef enum {
		/// The refractive index applied to the whole image. 
		FileMetadata_RefractiveIndex, // = 0
		/// The FOV in axial direction (x) in mm.
		FileMetadata_RangeX,
		/// The FOV in axial direction (y) in mm.
		FileMetadata_RangeY,
		/// The FOV in longitudinal axis (z) in mm.
		FileMetadata_RangeZ,
		/// The center of the scan pattern in axial direction (x) in mm.
		FileMetadata_CenterX,
		/// The center of the scan pattern in axial direction (y) in mm.
		FileMetadata_CenterY,
		/// The angle betwenn the scanner and the video camera image.
		FileMetadata_Angle,
		/// Ratio between the binary value from the camera to the count of electrons.
		FileMetadata_BinToElectronScaling,
		/// Central wavelength of the device.
		FileMetadata_CentralWavelength_nm,
		/// Bandwidth of the light source.
		FileMetadata_SourceBandwidth_nm,
		/// Electron cut-off parameter used for processing
		FileMetadata_MinElectrons,
		/// Quadratic dispersion factor used for dispersion correction
		FileMetadata_QuadraticDispersionCorrectionFactor,
		/// Threshold for speckle variance mode.
		FileMetadata_SpeckleVarianceThreshold,
		/// Time needed for data acqusition. The processing and saving time is not included.
		FileMetadata_ScanTime_Sec,
		/// Value for the reference intensity.
		FileMetadata_ReferenceIntensity,
		/// Scan pause in between scans
		FileMetadata_ScanPause_Sec,
		/// Zooms the scan pattern.
		FileMetadata_Zoom,
		/// Minimum distance between two points of the scan pattern used for freeform scan patterns.
		FileMetadata_MinPointDistance,
		/// Maximum distance between two points of the scan pattern used for freeform scan patterns.
		FileMetadata_MaxPointDistance,
		/// FFT oversampling use for processing and chirp correction
		FileMetadata_FFTOversampling,
		FileMetadata_FullWellCapacity,
		FileMetadata_Saturation,
		FileMetadata_CameraLineRate_Hz,
		/// Polarization mode correction. This angle (expresssed in radians) is used to 
		/// compute a phasor (\f$\exp(i\alpha)\f$), that will be applied to the complex
		/// reflectivities vector associated with camera 0.
		FileMetadata_PMDCorrectionAngle_rad,
		/// In birefringent samples, this offset allows referring the angle of the fast axis
		/// to an axis in the sample holder.
		FileMetadata_OpticalAxisOffset_rad,
		/// Reference stage position, if adjustable reference stage is available
		FileMetadata_ReferenceLength_mm,
		/// Position of reference intensity control, if available
		FileMetadata_ReferenceIntensityControl_Value,
		/// Position of quarter wave polarization adjustment, if available
		FileMetadata_PolarizationAdjustment_QuarterWave,
		/// Position of half wave polarization adjustment, if available
		FileMetadata_PolarizationAdjustment_HalfWave, // 28
	} FileMetadataFloat;

	/// \enum FileMetadataInt
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of integral type
	typedef enum {
		/// Contains the specifif data format.
		FileMetadata_ProcessState, // = 0
		/// Number of pixels in x.
		FileMetadata_SizeX,
		/// Number of pixels in y.
		FileMetadata_SizeY,
		/// Number of pixels in z.
		FileMetadata_SizeZ,
		/// Oversampling parameter.
		FileMetadata_Oversampling,
		/// Spectrum averaging
		FileMetadata_IntensityAveragedSpectra,
		/// A-scan averaging
		FileMetadata_IntensityAveragedAScans,
		/// B-scan averaging
		FileMetadata_IntensityAveragedBScans,
		/// Averaging for doppler processing in x-direction.
		FileMetadata_DopplerAverageX,
		/// Averaging for doppler processing in z-direction.
		FileMetadata_DopplerAverageZ,
		/// Type of window used for apodization.
		FileMetadata_ApoWindow,
		/// Bits per pixel of the camera.
		FileMetadata_DeviceBitDepth,
		/// Number of elements of the spectrometer.
		FileMetadata_SpectrometerElements,
		/// Serial number of the dataset
		FileMetadata_ExperimentNumber,
		/// Bytes per pixel of the camera.
		FileMetadata_DeviceBytesPerPixel,
		/// Averaging parameter of the fast scan axis in speckle variance mode.
		FileMetadata_SpeckleAveragingFastAxis,
		/// Averaging parameter of the slow scan axis in speckle variance mode.
		FileMetadata_SpeckleAveragingSlowAxis,
		/// FFT algorithm used
		FileMetadata_Processing_FFTType,
		/// Number of cameras, or sensors, stored in the file. In case of legacy files, this property
		/// takes the default value "1".
		FileMetadata_NumOfCameras, // 18
		/// In devices with more than one camera, some modi need to know which camera is active, because they do
		/// not support work with multiple cameras. In case of legacy files, this property takes the default
		/// value "0".
		FileMetadata_SelectedCamera,
		FileMetadata_ApodizationType,
		FileMetadata_AcquisitionOrder,
		/// DOPU filter specification. See #PolarizationDOPUFilterType.
		FileMetadata_DOPUFilter,
		/// Number of pixels for DOPU averaging in the z-direction.
		FileMetadata_DOPUAverageZ,
		/// Number of pixels for DOPU averaging in the x-direction.
		FileMetadata_DOPUAverageX,
		/// Number of pixels for DOPU averaging in the y-direction.
		FileMetadata_DOPUAverageY,
		/// Number of pixels for averaging along the z axis.
		FileMetadata_PolarizationAverageZ,
		/// Number of pixels for averaging along the x axis.
		FileMetadata_PolarizationAverageX,
		/// Number of pixels for averaging along the y axis.
		FileMetadata_PolarizationAverageY,
		/// Sampling amplification for VEGA systems
		FileMetadata_SamplingAmplification,
		FileMetadata_SamplingAmplificationSteps,
		/// Number of channels provided by analog input
		FileMetadata_AnalogInputNumOfChannels // 31
	} FileMetadataInt;

	/// \enum FileMetadataString
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of character string type
	typedef enum {
		/// Name of the OCT device series.
		FileMetadata_DeviceSeries, // 0
		/// Name of the OCT device.
		FileMetadata_DeviceName,
		/// Serial number of the OCT device.
		FileMetadata_Serial,
		/// Comment of the OCT data file.
		FileMetadata_Comment,
		/// Additional, custom info
		FileMetadata_CustomInfo,
		/// Acquisition mode of the OCT data file.
		FileMetadata_AcquisitionMode,
		/// Study of the OCT data file.
		FileMetadata_Study,
		/// Dispersion Preset of the OCT data file.
		FileMetadata_DispersionPreset,
		/// Name of the probe.
		FileMetadata_ProbeName,
		FileMetadata_FreeformScanPatternInterpolation,
		FileMetadata_HardwareConfig,
		FileMetadata_OrigVersion,
		FileMetadata_LastModVersion,
		/// Comma separated list of active channels 
		FileMetadata_AnalogInputActiveChannels,
		/// Comma separated list of input channel descriptions
		FileMetadata_AnalogInputChannelNames, // 14
	
	} FileMetadataString;

	/// \enum FileMetadataFlag
	/// \ingroup filehandling
	/// \brief Enum identifying file metadata fields of bool type
	typedef enum {
		/// This field is the flag that can be accessed with the functions #getProcessingFlag / #setProcessingFlag and the
		/// constant #Processing_UseOffsetErrors.
		FileMetadata_OffsetApplied, // = 0
		/// This field is the flag that can be accessed with the functions #getProcessingFlag / #setProcessingFlag and the
		/// constant #Processing_RemoveDCSpectrum.
		FileMetadata_DCSubtracted,
		FileMetadata_ApoApplied,
		FileMetadata_DechirpApplied,
		/// This field is the flag that can be accessed with the functions #getProcessingFlag / #setProcessingFlag and the
		/// constant #Processing_UseUndersamplingFilter.
		FileMetadata_UndersamplingFilterApplied,
		FileMetadata_DispersionCompensationApplied,
		FileMetadata_QuadraticDispersionCorrectionUsed,
		FileMetadata_ImageFieldCorrectionApplied,
		FileMetadata_ScanLineShown,
		/// This field is the flag that can be accessed with the functions #getProcessingFlag / #setProcessingFlag and the
		/// constant #Processing_UseAutocorrCompensation.
		FileMetadata_AutoCorrCompensationUsed,
		FileMetadata_BScanCrossCorrelation,
		/// This field is the flag that can be accessed with the functions #getProcessingFlag / #setProcessingFlag and the
		/// constant #Processing_RemoveAdvancedDCSpectrum.
		FileMetadata_DCSubtractedAdvanced,
		/// This field is the flag that can be accessed with the functions #getProcessingFlag / #setProcessingFlag and the
		/// constant #Processing_OnlyWindowing.
		FileMetadata_OnlyWindowing,
		/// This field is the flag that can be retrieved with the function #getDevicePropertyInt and the
		/// constant #Device_DataIsSigned.
		FileMetadata_RawDataIsSigned,
		FileMetadata_FreeformScanPatternIsActive,
		FileMetadata_FreeformScanPatternCloseLoop,
		FileMetadata_IsSweptSource // 16
	} FileMetadataFlag;

	/// \enum FileMetadata_ProcessingState
	/// \ingroup filehandling
	/// \brief Enum to specify the processing state of the stored data.
	/// \deprecated The future SDK will deduce the processing state out fo the available data in the OCT file.
	typedef enum {
		/// Just the spectra, without any further processing. See also #saveCalibrationToFile and #addFileRawData.
		RawSpectra,
		/// Just the computed intensity. See also #addFileRealData.
		ProcessedIntensity,
		/// Both the spectra and the computed intensity get stored. In the case of polarization-sensitive instruments,
		/// the complex data \f$C_0\f$ and \f$C_1\f$ are saved.
		RawSpectraAndProcessedIntensity,
		ProcessedIntensityAndPhase,
		RawSpectraAndProcessedIntensityAndPhase,
		psSpeckleVariance,
		psRawSpectraAndSpeckleVariance,
		psColored,
		psUnknown = 999
	}FileMetadata_ProcessingState;

	/// \brief The following constant is intended to be used as a possible third agument when inkoking the function #addFileColoredData.
	///        It can also be used as the second argument in the functions #findFileDataObject and #containsFileDataObject.
	static const char* DataObjectName_VideoImage = "data\\VideoImage.data";
	/// \brief The following constant is intended to be used as a possible third agument when inkoking the function #addFileColoredData.
	///        It can also be used as the second argument in the functions #findFileDataObject and #containsFileDataObject.
	static const char* DataObjectName_PreviewImage = "data\\PreviewImage.data";
	/// \brief The following constant is intended to be used as a possible third agument when inkoking the function #addFileRealData.
	///        It can also be used as the second argument in the functions #findFileDataObject and #containsFileDataObject.
	static const char* DataObjectName_OCTData = "data\\Intensity.data";
	static const char* DataObjectName_VarianceData = "data\\Variance.data";
	static const char* DataObjectName_PhaseData = "data\\Phase.data";
	static const char* DataObjectName_Spectral1DData = "data\\SpectralFloat.data";
	static const char* DataObjectName_AnalogInputData = "data\\AnalogInput.data";
	/// \brief The following constant is intended to be used as the third agument when inkoking the function #addFileComplexData.
	///        It can also be used as the second argument in the functions #findFileDataObject and #containsFileDataObject.
	static const char* DataObjectName_ComplexOCTData = "data\\Complex.data";
	static const char* DataObjectName_FreeformScanPoints = "data\\CustomScanPoints.data";
	static const char* DataObjectName_FreeformScanPointsInterpolated = "data\\CustomScanPointsInterpolated.data";

	/// \fn const char* DataObjectName_SpectralData(int index);
	/// \ingroup filehandling
	/// \brief Returns the filename of the spectral-data object with the specified index.
	/// \param index Index of spectral-data object to return
	/// \return Filename of the specified data object
	SPECTRALRADAR_API const char* DataObjectName_SpectralData(int index);

	/// \brief The following constant identifies the one-dimensional measurement mode (A-Scan). It is intended to be used as a possible
	///        third argument when invoking the function #setFileMetadataString with the constant #FMD_AcquisitionMode as second argument.
	///        It may also be returned by the function #getFileMetadataString whenever the second argument is #FMD_AcquisitionMode.
	static const char* AcquisitionMode_1D = "Mode1D";
	/// \brief The following constant identifies the two-dimensional measurement mode (B-Scan). It is intended to be used as a possible
	///        third argument when invoking the function #setFileMetadataString with the constant #FMD_AcquisitionMode as second argument.
	///        It may also be returned by the function #getFileMetadataString whenever the second argument is #FMD_AcquisitionMode.
	static const char* AcquisitionMode_2D = "Mode2D";
	/// \brief The following constant identifies the three-dimensional measurement mode (Volume). It is intended to be used as a possible
	///        third argument when invoking the function #setFileMetadataString with the constant #FMD_AcquisitionMode as second argument.
	///        It may also be returned by the function #getFileMetadataString whenever the second argument is #FMD_AcquisitionMode.
	static const char* AcquisitionMode_3D = "Mode3D";
	static const char* AcquisitionMode_Doppler = "ModeDoppler";
	static const char* AcquisitionMode_Speckle = "ModeSpeckle";
	static const char* AcquisitionMode_PolarizationSensitive = "ModePolarization";
	static const char* AcquisitionMode_PolarizationSensitive_3D = "ModePolarization3D";

	/// \fn OCTFileHandle createOCTFile(OCTFileFormat format);
	/// \ingroup filehandling
	/// \brief Creates a handle to an OCT file of the given format.
	SPECTRALRADAR_API OCTFileHandle createOCTFile(OCTFileFormat format);

	/// \fn void clearOCTFile(OCTFileHandle Handle);
	/// \ingroup filehandling
	/// \brief Clears the given OCT file handle and frees its resources.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	SPECTRALRADAR_API void clearOCTFile(OCTFileHandle Handle);

	/// \fn int getFileDataObjectCount(OCTFileHandle Handle);
	/// \ingroup filehandling
	/// \brief Returns the number of data objects in the OCT file. This number will vary depending on the file's format and contents (Files with the .oct extension may contain multiple OCT data objects depending on their internal structure).
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	SPECTRALRADAR_API int getFileDataObjectCount(OCTFileHandle Handle);

	/// \fn void loadFile(OCTFileHandle Handle, const char* Filename);
	/// \ingroup filehandling
	/// \brief Loads the actual OCT data file from a file system. The file must have the format given in createOCTFile().
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Filename Name of the data file to load.
	SPECTRALRADAR_API void loadFile(OCTFileHandle Handle, const char* Filename);

	/// \fn void saveFile(OCTFileHandle Handle, const char* Filename);
	/// \ingroup filehandling
	/// \brief Saves the OCT data file in the given fully qualified path name.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Filename Name to which the OCT data file will be written.
	SPECTRALRADAR_API void saveFile(OCTFileHandle Handle, const char* Filename);

	/// \fn void saveChangesToFile(OCTFileHandle Handle)
	/// \ingroup filehandling
	/// \brief Saves the OCT data file in the file previously opened with #loadFile(). Only changes will be saved. 
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	SPECTRALRADAR_API void saveChangesToFile(OCTFileHandle Handle);

	/// \fn void copyFileMetadata(OCTFileHandle SrcHandle, OCTFileHandle DstHandle);
	/// \ingroup filehandling
	/// \brief Copies metadata from one OCT file to another.
	/// \param[in] SrcHandle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	///                      This is the source and will not be altered by this function in any way.
	/// \param[out] DstHandle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	///                       This is the destination and will be filled in using the information in the source.
	SPECTRALRADAR_API void copyFileMetadata(OCTFileHandle SrcHandle, OCTFileHandle DstHandle);

	/// \fn bool containsFileMetadataFloat(OCTFileHandle Handle, FileMetadataFloat Floatfield);
	/// \ingroup filehandling
	/// \brief	Returns true if the given metadata field is present in the file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Floatfield Metadata field to test.
	SPECTRALRADAR_API bool containsFileMetadataFloat(OCTFileHandle Handle, FileMetadataFloat Floatfield);

	/// \fn double getFileMetadataFloat(OCTFileHandle Handle, FileMetadataFloat Floatfield);
	/// \ingroup filehandling
	/// \brief	Returns the value of the given file metadata field as a floating point number if found.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Floatfield Metadata field to read.
	SPECTRALRADAR_API double getFileMetadataFloat(OCTFileHandle Handle, FileMetadataFloat Floatfield);

	/// \fn void setFileMetadataFloat(OCTFileHandle Handle, FileMetadataFloat Floatfield, double Value);
	/// \ingroup filehandling
	/// \brief Sets the value of the given file metadata field as a floating point number.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Floatfield Metadata field to set.
	/// \param[in] Value Double value to set on the field.
	SPECTRALRADAR_API void setFileMetadataFloat(OCTFileHandle Handle, FileMetadataFloat Floatfield, double Value);

	/// \fn bool containsFileMetadataInt(OCTFileHandle Handle, FileMetadataInt Intfield);
	/// \ingroup filehandling
	/// \brief	Returns true if the given metadata field is present in the file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Intfield Metadata field to test.
	SPECTRALRADAR_API bool containsFileMetadataInt(OCTFileHandle Handle, FileMetadataInt Intfield);

	/// \fn int getFileMetadataInt(OCTFileHandle Handle, FileMetadataInt Intfield);
	/// \ingroup filehandling
	/// \brief Returns the value of the given file metadata field as an integer if found.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Intfield Metadata field to read.
	SPECTRALRADAR_API int getFileMetadataInt(OCTFileHandle Handle, FileMetadataInt Intfield);

	/// \fn void setFileMetadataInt(OCTFileHandle Handle, FileMetadataInt Intfield, int Value);
	/// \ingroup filehandling
	/// \brief Sets the value of the given file metadata field as an integer.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Intfield Metadata field to set.
	/// \param[in] Value    Integer value to set on the field. If \p Intfield is #FileMetadata_ProcessState, this argument should
	///                     be one of #FileMetadata_ProcessingState.
	SPECTRALRADAR_API void setFileMetadataInt(OCTFileHandle Handle, FileMetadataInt Intfield, int Value);

	/// \fn bool containsFileMetadataString(OCTFileHandle Handle, FileMetadataString Stringfield);
	/// \ingroup filehandling
	/// \brief	Returns true if the given metadata field is present in the file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Stringfield Metadata field to test.
	SPECTRALRADAR_API bool containsFileMetadataString(OCTFileHandle Handle, FileMetadataString Stringfield);

	/// \fn const char* getFileMetadataString(OCTFileHandle Handle, FileMetadataString Stringfield);
	/// \ingroup filehandling
	/// \brief Returns the value of the given file metadata field as a string if found.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Stringfield Metadata field to read.
	SPECTRALRADAR_API const char* getFileMetadataString(OCTFileHandle Handle, FileMetadataString Stringfield);

	/// \fn void setFileMetadataString(OCTFileHandle Handle, FileMetadataString Stringfield, const char* Content);
	/// \ingroup filehandling
	/// \brief Sets the value of the given file metadata field as a string.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Stringfield Metadata field to set.
	/// \param[in] Content String value to set on the field.
	SPECTRALRADAR_API void setFileMetadataString(OCTFileHandle Handle, FileMetadataString Stringfield, const char* Content);

	/// \fn bool containsFileMetadataFlag(OCTFileHandle Handle, FileMetadataFlag Boolfield);
	/// \ingroup filehandling
	/// \brief	Returns true if the given metadata field is present in the file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Boolfield Metadata field to test.
	SPECTRALRADAR_API bool containsFileMetadataFlag(OCTFileHandle Handle, FileMetadataFlag Boolfield);

	/// \fn BOOL getFileMetadataFlag(OCTFileHandle Handle, FileMetadataFlag Boolfield);
	/// \ingroup filehandling
	/// \brief Gets the boolean value of the given file metadata field.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Boolfield Metadata field to read.
	SPECTRALRADAR_API BOOL getFileMetadataFlag(OCTFileHandle Handle, FileMetadataFlag Boolfield);

	/// \fn void setFileMetadataFlag(OCTFileHandle Handle, FileMetadataFlag Boolfield, BOOL Value);
	/// \ingroup filehandling
	/// \brief Sets the boolean value of the given file metadata field. 
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Boolfield Metadata field to set.
	/// \param[in] Value Boolean value to set on the field.
	SPECTRALRADAR_API void setFileMetadataFlag(OCTFileHandle Handle, FileMetadataFlag Boolfield, BOOL Value);

	/// \fn void saveFileMetadata(OCTFileHandle Handle, OCTDeviceHandle Dev, ProcessingHandle Proc, ProbeHandle Probe, ScanPatternHandle Pattern);
	/// \ingroup filehandling
	/// \brief Saves meta information from the given device, processing, probe and scan pattern instances in the metadata block of the given file handle. This information will be available in files of type FileFormat_OCITY; mileage on other formats may vary according to their description.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] Probe A valid (non null) handle of an initialized probem, obtained through #initProbe.
	/// \param[in] Pattern A valid (non null) handle of a scan pattern.
	SPECTRALRADAR_API void saveFileMetadata(OCTFileHandle Handle, OCTDeviceHandle Dev, ProcessingHandle Proc, ProbeHandle Probe, ScanPatternHandle Pattern);

	/// \fn void setFileMetadataTimestamp(OCTFileHandle File, time_t Timestamp)
	/// \ingroup filehandling
	/// \brief Saves provided timestamp to meta information to the given file handle. This information will be available in files of type FileFormat_OCITY; mileage on other formats may vary according to their description.
	/// \param[in] File A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Timestamp A valid (non null) timestamp.
	SPECTRALRADAR_API void setFileMetadataTimestamp(OCTFileHandle File, time_t Timestamp);
	
	/// \fn time_t getFileMetadataTimestamp(OCTFileHandle File)
	/// \ingroup filehandling
	/// \brief Returns the specified timestamp from the meta information of the given file handle. This information will be available in files of type FileFormat_OCITY; mileage on other formats may vary according to their description.
	/// \param[in] File A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	SPECTRALRADAR_API time_t getFileMetadataTimestamp(OCTFileHandle File);

	/// \fn void saveFileMetadataDoppler(OCTFileHandle Handle, DopplerProcessingHandle DopplerProc);
	/// \ingroup filehandling
	/// \brief Saves meta information from the given DopplerProcessingHandle. A corresponding DopplerProcessingHandle can then be recreated using createDopplerProcessingForFile.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile. This describes the files then handle data is stored to.
	/// \param[in] DopplerProc A valid (non null) handle of Doppler processing obtained by #createDopplerProcessing. This is the handle whose data is stored. 
	SPECTRALRADAR_API void saveFileMetadataDoppler(OCTFileHandle Handle, DopplerProcessingHandle DopplerProc);
	
	/// \fn void saveFileMetadataSpeckle(OCTFileHandle Handle, SpeckleVarianceHandle SpeckleVarianceProc);
	/// \ingroup filehandling
	/// \brief Saves meta information from the given SpeckleVarianceHandle. A corresponding SpeckleVarianceHandle can then be recreated using initSpeckleVarianceForFile.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile. This describes the files then handle data is stored to.
	/// \param[in] SpeckleVarianceProc A valid (non null) handle of speckle variance processing obtained by #initSpeckleVariance. This is the handle whose data is stored. 
	SPECTRALRADAR_API void saveFileMetadataSpeckle(OCTFileHandle Handle, SpeckleVarianceHandle SpeckleVarianceProc);


	/// \fn void loadCalibrationFromFile(OCTFileHandle Handle, ProcessingHandle Proc);
	/// \ingroup filehandling
	/// \brief Loads Chirp, Offset, and Apodization vectors from the given OCT file into the given processing object.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[out] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///				   	 one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					 #createProcessingForOCTFile.
	SPECTRALRADAR_API void loadCalibrationFromFile(OCTFileHandle Handle, ProcessingHandle Proc);

	/// \fn void loadCalibrationFromFileEx(OCTFileHandle Handle, ProcessingHandle Proc, const int CameraIndex);
	/// \ingroup filehandling
	/// \brief Loads Chirp, Offset, and Apodization vectors from the given OCT file into the given processing object.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] CameraIndex The camera index (0-based, i.e. zero for the first, one for the second, and so on).
	SPECTRALRADAR_API void loadCalibrationFromFileEx(OCTFileHandle Handle, ProcessingHandle Proc, const int CameraIndex);

	/// \fn void saveCalibrationToFile(OCTFileHandle Handle, ProcessingHandle Proc);
	/// \ingroup filehandling
	/// \brief Saves Chirp, Offset, and Apodization vectors from the given processing object into the given OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	SPECTRALRADAR_API void saveCalibrationToFile(OCTFileHandle Handle, ProcessingHandle Proc);

	/// \fn void saveCalibrationToFileEx(OCTFileHandle Handle, ProcessingHandle Proc, int CameraIndex);
	/// \ingroup filehandling
	/// \brief Saves Chirp, Offset, and Apodization vectors from the given processing object into the given OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Proc A valid (non null) handle of the processing routines (#ProcessingHandle), previously obtained through
	///					one of the functions #createProcessing, #createProcessingForDevice, #createProcessingForDeviceEx or
	///					#createProcessingForOCTFile.
	/// \param[in] CameraIndex The camera index (0-based, i.e. zero for the first, one for the second, and so on).
	SPECTRALRADAR_API void saveCalibrationToFileEx(OCTFileHandle Handle, ProcessingHandle Proc, int CameraIndex);

	/// \fn void getFileRealData(OCTFileHandle Handle, DataHandle Data, int Index);
	/// \ingroup filehandling
	/// \brief Retrieves a RealData object from the OCT file at the given index with 0 <= index < getFileDataObjectCount(OCTFileHandle handle). Users must ensure that the data handle is properly prepared and destroyed.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[out] Data A valid (non null) data handle of the existing data (#DataHandle), previously obtained with the function
	///                  #createData. It will be filled in with the data read from the OCT file at the given \a Index.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	SPECTRALRADAR_API void getFileRealData(OCTFileHandle Handle, DataHandle Data, int Index);

	/// \fn void getFileColoredData(OCTFileHandle Handle, ColoredDataHandle Data, size_t Index);
	/// \ingroup filehandling
	/// \brief Retrieves a ColoredData object from the OCT file at the given index with 0 <= index < getFileDataObjectCount(OCTFileHandle handle). Users must ensure that the data handle is properly prepared and destroyed.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[out] Data A valid (non null) colored data handle of the existing data (#ColoredDataHandle), previously obtained with the function
	///                  #createColoredData. It will be filled in with the data read from the OCT file at the given \a Index.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	SPECTRALRADAR_API void getFileColoredData(OCTFileHandle Handle, ColoredDataHandle Data, size_t Index);

	/// \fn void getFileComplexData(OCTFileHandle Handle, ComplexDataHandle Data, size_t Index);
	/// \ingroup filehandling
	/// \brief Retrieves a ComplexData object from the OCT file at the given index with 0 <= index < getFileDataObjectCount(OCTFileHandle handle). Users must ensure that the data handle is properly prepared and destroyed.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[out] Data A valid (non null) complex data handle of the existing data (#ComplexDataHandle), previously obtained with the function
	///                  #createComplexData. It will be filled in with the data read from the OCT file at the given \a Index.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	SPECTRALRADAR_API void getFileComplexData(OCTFileHandle Handle, ComplexDataHandle Data, size_t Index);

	/// \fn void getFileRawData(OCTFileHandle Handle, RawDataHandle Data, size_t Index);
	/// \ingroup filehandling
	/// \brief Retrieves a RawData object from the OCT file at the given index with 0 <= index < getFileDataObjectCount(OCTFileHandle handle). Users must ensure that the data handle is properly prepared and destroyed.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[out] Data A valid (non null) raw data handle of the existing data (#RawDataHandle), previously obtained with the function
	///                  #createRawData. It will be filled in with the data read from the OCT file at the given \a Index.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void getFileRawData(OCTFileHandle Handle, RawDataHandle Data, size_t Index);

	/// \fn void getFile(OCTFileHandle Handle, size_t Index, const char* FilenameOnDisk);
	/// \ingroup filehandling
	/// \brief Retrieves a data object of arbitrary type from the OCT file at the given index with 0 <= index < getFileDataObjectCount(OCTFileHandle handle) and stores it at the given fully qualified path.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the file inside the OCT file, e.g. returned by #findFileDataObject.
	/// \param[in] FilenameOnDisk Filename to which requested file will be written.
	SPECTRALRADAR_API void getFile(OCTFileHandle Handle, size_t Index, const char* FilenameOnDisk);
	
	/// \fn int findFileDataObject(OCTFileHandle Handle, const char* Search);
	/// \ingroup filehandling 
	/// \brief Searches for a data object the name of which contains the given string and returns its index, -1 if not found.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Search Data object name to find in OCT file.
	SPECTRALRADAR_API int findFileDataObject(OCTFileHandle Handle, const char* Search);

	/// \fn BOOL containsFileDataObject(OCTFileHandle Handle, const char* Search);
	/// \ingroup filehandling
	/// \brief Searches for a data object the name of which contains the given string and returns TRUE if at least one data object name matches.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Search Data object name to find in OCT file.
	SPECTRALRADAR_API BOOL containsFileDataObject(OCTFileHandle Handle, const char* Search);

	/// \fn BOOL containsFileRawData(OCTFileHandle Handle);
	/// \ingroup filehandling
	/// \brief Returns TRUE if the file contains raw data objects.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API BOOL containsFileRawData(OCTFileHandle Handle);

	/// \fn void addFileRealData(OCTFileHandle Handle, DataHandle Data, const char* DataObjectName);
	/// \ingroup filehandling
	/// \brief Adds a RealData object to the OCT file; dataObjectName will be its name inside the OCT file if applicable. The object that the DataHandle refers to must live until after saveFile() has been called.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Data A valid (non null) handle to the RealData object (#DataHandle) to add.
	/// \param[in] DataObjectName Name that will be assigned to the object in the OCT file.
	SPECTRALRADAR_API void addFileRealData(OCTFileHandle Handle, DataHandle Data, const char* DataObjectName);

	/// \fn void addFileColoredData(OCTFileHandle Handle, ColoredDataHandle Data, const char* DataObjectName);
	/// \ingroup filehandling
	/// \brief Adds a ColoredData object to the OCT file; dataObjectName will be its name inside the OCT file if applicable. The object that the ColoredDataHandle refers to must live until after saveFile() has been called.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Data A valid (non null) handle to the ColoredData object (#ColoredDataHandle) to add.
	/// \param[in] DataObjectName Name that will be assigned to the object in the OCT file.
	SPECTRALRADAR_API void addFileColoredData(OCTFileHandle Handle, ColoredDataHandle Data, const char* DataObjectName);

	/// \fn void addFileComplexData(OCTFileHandle Handle, ComplexDataHandle Data, const char* DataObjectName);
	/// \ingroup filehandling
	/// \brief Adds a ComplexData object to the OCT file; dataObjectName will be its name inside the OCT file if applicable. The object that the ComplexDataHandle refers to must live until after saveFile() has been called.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Data A valid (non null) handle to the ComplexData object (#ComplexDataHandle) to add.
	/// \param[in] DataObjectName Name that will be assigned to the object in the OCT file.
	SPECTRALRADAR_API void addFileComplexData(OCTFileHandle Handle, ComplexDataHandle Data, const char* DataObjectName);

	/// \fn void addFileRawData(OCTFileHandle Handle, RawDataHandle Data, const char* DataObjectName);
	/// \ingroup filehandling
	/// \brief Adds raw \a Data object to the OCT file; \a DataObjectName will be its name inside the OCT file if applicable.
	/// The object that the #RawDataHandle refers to must live until after #saveFile has been called.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Data A valid (non null) raw data handle of the existing data (#RawDataHandle), previously obtained with the function
	///                 #createRawData. It is assumed that these data have already been filled in with an appropiate data acquisition procedure.
	/// \param[in] DataObjectName Name that will be assigned to the object in the OCT file.
	/// Notice that raw data refers to the spectra as acquired, without processing of any kind.
	SPECTRALRADAR_API void addFileRawData(OCTFileHandle Handle, RawDataHandle Data, const char* DataObjectName);

	/// \fn void addFileText(OCTFileHandle Handle, const char* FilenameOnDisk, const char* DataObjectName);
	/// \ingroup filehandling
	/// \brief Adds a text object read from \a FilenameOnDisk to the OCT file; \a DataObjectName will be its name inside the OCT file if applicable.
	///        The file identified by filenameOnDisk must exist until after saveFile() has been called.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] FilenameOnDisk Filename from which text file will be read.
	/// \param[in] DataObjectName Name that will be assigned to the object in the OCT file.
	SPECTRALRADAR_API void addFileText(OCTFileHandle Handle, const char* FilenameOnDisk, const char* DataObjectName);

	/// \fn DataObjectType getFileDataObjectType(OCTFileHandle Handle, int Index);
	/// \ingroup filehandling
	/// \brief Returns the type of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return The type of the selected data object or DataObjectType_Unknown in case of an error.
	SPECTRALRADAR_API DataObjectType getFileDataObjectType(OCTFileHandle Handle, int Index);

	/// \fn void getFileDataObjectName(OCTFileHandle Handle, int Index, char* Filename, int Length);
	/// \ingroup filehandling
	/// \brief Returns the name of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, 0 <= Index < #getFileDataObjectCount()
	/// \param[out] Filename Name of the requested file
	/// \param[in] Length Length of the user-provided buffer at \a Filename
	SPECTRALRADAR_API void getFileDataObjectName(OCTFileHandle Handle, int Index, char* Filename, int Length);

	/// \fn int getFileDataSizeX(OCTFileHandle Handle, size_t Index);
	/// \ingroup filehandling 
	/// \brief Returns the pixel count in X of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return Pixel count in X of the data object or 0 in case of an error
	SPECTRALRADAR_API int getFileDataSizeX(OCTFileHandle Handle, size_t Index);

	/// \fn int getFileDataSizeY(OCTFileHandle Handle, size_t Index);
	/// \ingroup filehandling
	/// \brief Returns the pixel count in Y of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return Pixel count in Y of the data object or 0 in case of an error
	SPECTRALRADAR_API int getFileDataSizeY(OCTFileHandle Handle, size_t Index);
	
	/// \fn int getFileDataSizeZ(OCTFileHandle Handle, size_t Index);
	/// \ingroup filehandling
	/// \brief Returns the pixel count in Z of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return Pixel count in Z of the data object or 0 in case of an error
	SPECTRALRADAR_API int getFileDataSizeZ(OCTFileHandle Handle, size_t Index);
	
	/// \fn float getFileDataRangeX(OCTFileHandle Handle, size_t Index);
	/// \ingroup filehandling
	/// \brief Returns the range (usually in mm) in X of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return Range in X of the data object or 0.0f in case of an error
	SPECTRALRADAR_API float getFileDataRangeX(OCTFileHandle Handle, size_t Index);
	
	/// \fn float getFileDataRangeY(OCTFileHandle Handle, size_t Index);
	/// \ingroup filehandling
	/// \brief Returns the range (usually in mm) in Y of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return Range in Y of the data object or 0.0f in case of an error
	SPECTRALRADAR_API float getFileDataRangeY(OCTFileHandle Handle, size_t Index);
	
	/// \fn float getFileDataRangeZ(OCTFileHandle Handle, size_t Index);
	/// \ingroup filehandling
	/// \brief Returns the range (usually in mm) in Z of the data object at the given \a Index in the OCT file.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the data inside the OCT file, e.g. returned by #findFileDataObject.
	/// \return Range in Z of the data object or 0.0f in case of an error
	SPECTRALRADAR_API float getFileDataRangeZ(OCTFileHandle Handle, size_t Index);

	/// \fn void copyMarkerListFromRealData(OCTFileHandle Handle, DataHandle Data);
	/// \ingroup filehandling
	/// \brief Copies the marker list from the given data handle into the metadata block of the given OCT file handle.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Data A valid (non null) data handle of the existing data (#DataHandle), previously obtained with the function
	///                 #createData. It is assumed that this structure has already been filled with processed data. If no markers
	///                 are present, this function does nothing.
    /// Markers are a visual help, that can be created or manipulated by ThorImage-OCT. Markers are always expressed in physical
	/// coordinates, so re-use is possible.
	SPECTRALRADAR_API void copyMarkerListFromRealData(OCTFileHandle Handle, DataHandle Data);
	
	/// \fn void copyMarkerListToRealData(OCTFileHandle Handle, DataHandle Data);
	/// \ingroup filehandling
	/// \brief Copies the marker list from the metadata block of the given file handle to the given data handle.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[out] Data A valid (non null) data handle of the existing data (#DataHandle), previously obtained with the function
	///                  #createData. If no markers are present, this function does nothing.
    /// Markers are a visual help, that can be created or manipulated by ThorImage-OCT. Markers are always expressed in physical
	/// coordinates, so re-use is possible.
	SPECTRALRADAR_API void copyMarkerListToRealData(OCTFileHandle Handle, DataHandle Data); 

	/// \fn void addFileMetadataPreset(OCTFileHandle Handle, const char* Category, const char* PresetDescription);
	/// \ingroup filehandling
	/// \brief Adds one of the presets set during acquisition for the #OCTFileHandle
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Category Name of the category of the added preset.
	/// \param[in] PresetDescription Description for the added preset.
	SPECTRALRADAR_API void addFileMetadataPreset(OCTFileHandle Handle, const char* Category, const char* PresetDescription);

	/// \fn int getFileMetadataNumberOfPresets(OCTFileHandle Handle);
	/// \ingroup filehandling
	/// \brief Gets the number of presets that were set during the acquisition.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	SPECTRALRADAR_API int getFileMetadataNumberOfPresets(OCTFileHandle Handle);

	/// \fn const char* getFileMetadataPresetCategory(OCTFileHandle Handle, int Index);
	/// \ingroup filehandling
	/// \brief Gets the preset category belonging to the preset with given \a Index.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the preset inside the OCT file, 0 <= Index < #getFileMetadataNumberOfPresets
	SPECTRALRADAR_API const char* getFileMetadataPresetCategory(OCTFileHandle Handle, int Index);

	/// \fn const char* getFileMetadataPresetDescription(OCTFileHandle Handle, int Index);
	/// \ingroup filehandling
	/// \brief Gets the preset description belonging to the preset with given \a Index.
	/// \param[in] Handle A valid (non null) handle of OCTFile (#OCTFileHandle), obtained with the function #createOCTFile.
	/// \param[in] Index Index of the preset inside the OCT file, 0 <= Index < #getFileMetadataNumberOfPresets
	SPECTRALRADAR_API const char* getFileMetadataPresetDescription(OCTFileHandle Handle, int Index);

	/// \enum SpeckleVarianceType
	/// \ingroup specklevariance
	/// \brief Enum identifying different speckle variance processing types. 
	typedef enum {
		// Logscale speckle variance with linear output
		SpeckleVariance_LogscaleVariance_Linear,
		// Logscale speckle variance with logscale output
		SpeckleVariance_LogscaleVariance_Logscale,
		// Linear speckle variance with linear output
		SpeckleVariance_LinearVariance_Linear,
		// Linear speckle variance with logscale output
		SpeckleVariance_LinearVariance_Logscale,
		// Complex speckle variance with linear output
		SpeckleVariance_ComplexVariance_Linear,
		// Complex speckle variance with logscale output
		SpeckleVariance_ComplexVariance_Logscale
	} SpeckleVarianceType;

	/// \enum SpeckleVariancePropertyInt
	/// \ingroup specklevariance
	/// \brief Enum identifying different properties of typ int for speckle variance processing. 
	typedef enum {
		// Averaging on Z-Axis
		SpeckleVariance_Averaging_1,
		// Averaging on X-Axis
		SpeckleVariance_Averaging_2,
		// Averaging on Y-Axis
		SpeckleVariance_Averaging_3
	} SpeckleVariancePropertyInt;

	/// \enum SpeckleVariancePropertyFloat
	/// \ingroup specklevariance
	/// \brief Enum identifying different properties of typ float for speckle variance processing. 
	typedef enum {
		// Threshold used for processed data
		SpeckleVariance_Threshold
	} SpeckleVariancePropertyFloat;

	/// \fn SpeckleVarianceHandle initSpeckleVariance(void);
	/// \ingroup specklevariance
	/// \brief Initializes the speckle variance contrast processing instance.
	SPECTRALRADAR_API SpeckleVarianceHandle initSpeckleVariance(void);

	/// \fn SPECTRALRADAR_API SpeckleVarianceHandle initSpeckleVarianceForFile(OCTFileHandle File);
	/// \ingroup specklevariance
	/// \brief Initializes the speckle variance contrast processing instance, based on the parameters stored in an OCT file.
	/// \param[in] File A handle to the OCT-File used to create the speckle variance processing routines from. 
	SPECTRALRADAR_API SpeckleVarianceHandle initSpeckleVarianceForFile(OCTFileHandle File);
	
	/// \fn void closeSpeckleVariance(SpeckleVarianceHandle Handle);
	/// \ingroup specklevariance
	/// \brief Closes the speckle variance contrast processing instance and frees all used resources.
	/// \param[in] Handle A handle of speckle variance routines (#SpeckleVarianceHandle). If the handle is a nullptr, this function does nothing.
	SPECTRALRADAR_API void closeSpeckleVariance(SpeckleVarianceHandle Handle);
	// OBSOLETED
	//SPECTRALRADAR_API void setAveraging(SpeckleVarianceHandle SpeckleVar, int Av1, int Av2, int Av3);

	/// \fn void setSpeckleVariancePropertyInt(SpeckleVarianceHandle svh, SpeckleVariancePropertyInt prm, int value);
	/// \ingroup specklevariance
	/// \brief Sets the given integer property to the given value.
	SPECTRALRADAR_API void setSpeckleVariancePropertyInt(SpeckleVarianceHandle Handle, SpeckleVariancePropertyInt Property, int value);

	/// \fn int getSpeckleVariancePropertyInt(SpeckleVarianceHandle svh, SpeckleVariancePropertyInt prm);
	/// \ingroup specklevariance
	/// \brief Sets the given floating point property to the given value.
	SPECTRALRADAR_API int getSpeckleVariancePropertyInt(SpeckleVarianceHandle Handle, SpeckleVariancePropertyInt Property);

	/// \fn void setSpeckleVariancePropertyFloat(SpeckleVarianceHandle svh, SpeckleVariancePropertyFloat prm, double value);
	/// \ingroup specklevariance
	/// \brief Returns the value of the given integer property.
	SPECTRALRADAR_API void setSpeckleVariancePropertyFloat(SpeckleVarianceHandle Handle, SpeckleVariancePropertyFloat Property, double value);

	/// \fn double getSpeckleVariancePropertyFloat(SpeckleVarianceHandle svh, SpeckleVariancePropertyFloat prm);
	/// \ingroup specklevariance
	/// \brief Returns the value of the given floating point property
	SPECTRALRADAR_API double getSpeckleVariancePropertyFloat(SpeckleVarianceHandle Handle, SpeckleVariancePropertyFloat Property);

	/// \fn void setSpeckleVarianceType(SpeckleVarianceHandle SpeckleVar, SpeckleVarianceType Type);
	/// \ingroup specklevariance
	/// \brief Sets the speckle variance type to the given value.
	SPECTRALRADAR_API void setSpeckleVarianceType(SpeckleVarianceHandle SpeckleVar, SpeckleVarianceType Type);

	/// \fn void SpeckleVarianceType getSpeckleVarianceType(SpeckleVarianceHandle SpeckleVar);
	/// \ingroup specklevariance
	/// \brief Returns the speckle variance type the instance is using.
	SPECTRALRADAR_API SpeckleVarianceType getSpeckleVarianceType(SpeckleVarianceHandle SpeckleVar);

	/// \fn void computeSpeckleVariance(SpeckleVarianceHandle SpeckleVar, ComplexDataHandle CompDataIn, DataHandle DataOutMean, DataHandle DataOutVar);
	/// \ingroup specklevariance
	/// \brief Computes the speckle variance contrast and returns the mean and variance values in DataOutMean and DataOutVar.
	SPECTRALRADAR_API void computeSpeckleVariance(SpeckleVarianceHandle SpeckleVar, ComplexDataHandle CompDataIn, DataHandle DataOutMean, DataHandle DataOutVar);

	/// \enum DeviceTriggerType
	/// \ingroup Hardware
	/// \brief Enum identifying trigger types for the OCT system. 
	/// \warning Not all trigger types are available for all different systems. To check whether the specified trogger mode is available or not please use #isTriggerModeAvailable
	typedef enum DeviceTriggerType_ {
		/// Standard mode. 
		Trigger_FreeRunning,
		/// Used to trigger the start of an acquisition. Additional hardware is needed. 
		Trigger_TrigBoard_ExternalStart,
		/// Mode to trigger the acquisition of each A-scan. An external trigger signal is needed. Please see the software manual for detailed information.
		Trigger_External_AScan
	} DeviceTriggerType;

	/// \enum DeviceTriggerIOType
	/// \ingroup Hardware
	/// \brief Enum identifying trigger types for the secondary trigger IO channel.
	/// \warning Not all systems support trigger IO. To check if the system supports trigger IO, please use #isTriggerIOModeAvailable
	typedef enum DeviceTriggerIOType_ {
		/// Do not use trigger IO
		TriggerIO_Disabled,
		/// Output mode
		TriggerIO_Output,
		/// Input mode (uses trigger IO to start individual scans)
		TriggerIO_Input,
	} DeviceTriggerIOType;

	/// \enum ScanPatternPropertyInt
	/// \ingroup ScanPattern
	/// \brief Enum identifying different properties of typ int of the specified scan pattern. 
	typedef enum {
		/// Total count of trigger pulses needed for acquisition of the scan pattern once. The acquisition will start again after finishing for continuous acquisition mode. 
		ScanPattern_SizeTotal,
		/// Count of cycles for the scan pattern
		ScanPattern_Cycles,
		/// Count of trigger pulses needed to acquire one cycle, e.g. one B-scan in a volume scan. 
		ScanPattern_SizeCycle,
		/// Count of trigger pulses needed before the scanning of the sample starts. The OCT beam needs to be positioned and the apodization scans used for processing need to be acquired. 
		/// The flyback time is the time used to reach the position of apodization and start of scan pattern.
		ScanPattern_SizePreparationCycle,
		/// Count of trigger pulses to acquire the sample depending on averaging and size-x of the scan pattern.
		ScanPattern_SizeImagingCycle,
		/// Count of trigger pulses needed before the first scanning of the sample starts. Some scan
		/// patterns implement a dedicated apodisation scan before the first actual scan.
		ScanPattern_SizePreparationScan,
	} ScanPatternPropertyInt;

	/// \enum ScanPatternPropertyFloat
	/// \ingroup ScanPattern
	/// \brief Enum identifying different floating-type properties of the specified scan pattern. 
	typedef enum {
		/// The range of the scan pattern in mm for the x-direction
		ScanPattern_RangeX,
		/// The range of the scan pattern in mm for the y-direction
		ScanPattern_RangeY,
		/// the current x-center position in mm
		ScanPattern_CenterX,
		/// the current y-center position in mm
		ScanPattern_CenterY,
		/// the current scan pattern angle in radians
		ScanPattern_Angle,
		/// the mean of the B-scan lengths of the scan pattern in mm
		ScanPattern_MeanLength_mm
	} ScanPatternPropertyFloat;

	/// \defgroup ExternalTrigger External trigger
	/// \brief Functions to inquire, setup, and deal with an external trigger. Whether this functionality is supported, and to what extent, depends on the hardware.

	/// \fn void setTriggerMode(OCTDeviceHandle Dev, DeviceTriggerType TriggerMode);
	/// \ingroup ExternalTrigger
	/// \brief Sets the trigger mode for the OCT device used for acquisition. Additional hardware may be needed.
	/// \param[in] Dev Valid OCT device handle
	/// \param[in] TriggerMode Which trigger mode to use
	SPECTRALRADAR_API void setTriggerMode(OCTDeviceHandle Dev, DeviceTriggerType TriggerMode);

	/// \fn DeviceTriggerType getTriggerMode(OCTDeviceHandle Dev);
	/// \ingroup ExternalTrigger
	/// \brief Returns the trigger mode used for acquisition.
	/// \param[in] Dev Valid OCT device handle
	/// \return Current trigger mode
	SPECTRALRADAR_API DeviceTriggerType getTriggerMode(OCTDeviceHandle Dev);

	/// \fn BOOL isTriggerModeAvailable(OCTDeviceHandle Dev, DeviceTriggerType TriggerMode);
	/// \ingroup ExternalTrigger
	/// \brief Returns whether the specified trigger mode is possible or not for the used device. 
	/// \param[in] Dev Valid OCT device handle
	/// \param[in] TriggerMode Mode to query
	SPECTRALRADAR_API BOOL isTriggerModeAvailable(OCTDeviceHandle Dev, DeviceTriggerType TriggerMode);

	/// \fn BOOL isTriggerIOModeAvailable(OCTDeviceHandle Dev, DeviceTriggerIOType TriggerMode);
	/// \ingroup ExternalTrigger
	/// \brief Returns whether the specified trigger IO mode is possible or not for the used device. 
	/// \param[in] Dev Valid OCT device handle
	/// \param[in] TriggerMode Mode to query
	SPECTRALRADAR_API BOOL isTriggerIOModeAvailable(OCTDeviceHandle Dev, DeviceTriggerIOType TriggerMode);

	/// \fn void setTriggerIOMode(OCTDeviceHandle Dev, DeviceTriggerIOType TriggerMode);
	/// \ingroup ExternalTrigger
	/// \brief Sets the trigger mode for the trigger IO channel of the OCT device.
	/// \warning Not all systems support trigger IO. To check if the system supports trigger IO, please use #isTriggerIOModeAvailable
	/// \param[in] Dev Valid OCT device handle
	/// \param[in] TriggerMode Which trigger mode to use
	SPECTRALRADAR_API void setTriggerIOMode(OCTDeviceHandle Dev, DeviceTriggerIOType TriggerMode);

	/// \fn DeviceTriggerIOType getTriggerIOMode(OCTDeviceHandle Dev);
	/// \ingroup ExternalTrigger
	/// \brief Returns the trigger IO mode used for acquisition.
	/// \param[in] Dev Valid OCT device handle
	/// \return Current trigger mode
	SPECTRALRADAR_API DeviceTriggerIOType getTriggerIOMode(OCTDeviceHandle Dev);

	/// \fn void setTriggerIOConfiguration(OCTDeviceHandle Dev, size_t Offset, size_t Divider)
	/// \ingroup ExternalTrigger
	/// \brief Configures the parameters for trigger mode for the trigger IO channel of the OCT device. If the trigger IO channel is set to output, 
	/// it will start generating a pulses after n=Offset A-Scans. It will then generate a pulse every after d=Divider A-Scans.
	/// If the trigger IO channel is set to input, Offset is ignored. Whenever a trigger pulse is received on the trigger IO channel, the system
	/// will perform d=Divider A-Scans and wait for the next trigger.
	/// \param[in] Dev Valid OCT device handle
	/// \param[in] Offset Number of A-Scans to wait before first trigger IO pulse
	/// \param[in] Divider Number of A-Scans to wait before each subsequent trigger IO pulse
	SPECTRALRADAR_API void setTriggerIOConfiguration(OCTDeviceHandle Dev, size_t Offset, size_t Divider);

	/// \fn void setTriggerTimeout_s(OCTDeviceHandle Dev, int Timeout_s);
	/// \ingroup ExternalTrigger
	/// \brief Sets the timeout of the camera in seconds (useful in external trigger mode). 
	/// \param[in] Dev Valid OCT device handle
	/// \param[in] Timeout_s Timeout in external trigger mode in seconds
	SPECTRALRADAR_API void setTriggerTimeout_s(OCTDeviceHandle Dev, int Timeout_s);

	/// \fn int getTriggerTimeout_s(OCTDeviceHandle Dev);
	/// \ingroup ExternalTrigger
	/// \brief Returns the timeout of the camera in seconds (not used in trigger mode Trigger_FreeRunning). 
	/// \param[in] Dev Valid OCT device handle
	/// \return Timeout in external trigger mode in seconds
	SPECTRALRADAR_API int getTriggerTimeout_s(OCTDeviceHandle Dev);

	/// \fn int getScanPatternPropertyInt(ScanPatternHandle ScanPattern, ScanPatternPropertyInt Property);
	/// \ingroup ScanPattern
	/// \brief Returns the specified property of the scan pattern. 
	SPECTRALRADAR_API int getScanPatternPropertyInt(ScanPatternHandle ScanPattern, ScanPatternPropertyInt Property);

	/// \fn double getScanPatternPropertyFloat(ScanPatternHandle Pattern, ScanPatternPropertyFloat Selection);
	/// \ingroup ScanPattern
	/// \brief Returns the specified property of the scan pattern. 
	SPECTRALRADAR_API double getScanPatternPropertyFloat(ScanPatternHandle Pattern, ScanPatternPropertyFloat Selection);

	/// \fn double expectedAcquisitionTime_s(ScanPatternHandle ScanPattern, OCTDeviceHandle Dev);
	/// \ingroup ScanPattern
	/// \brief Returns the expected acquisition time of the scan pattern.
	SPECTRALRADAR_API double expectedAcquisitionTime_s(ScanPatternHandle ScanPattern, OCTDeviceHandle Dev);

	/// \fn ScanPatternAcquisitionOrder getScanPatternAcqOrder(ScanPatternHandle ScanPattern);
	/// \ingroup ScanPattern
	/// \brief Returns the acquisition order of the scan pattern. See definition of #ScanPatternAcquisitionOrder for detailed information.
	SPECTRALRADAR_API ScanPatternAcquisitionOrder getScanPatternAcqOrder(ScanPatternHandle ScanPattern);

	/// \fn BOOL isAcqTypeForScanPatternAvailable(ScanPatternHandle ScanPattern, AcquisitionType AcqType);
	/// \ingroup ScanPattern
	/// \brief Returns whether the acquisition type is available for the scan pattern.
	SPECTRALRADAR_API BOOL isAcqTypeForScanPatternAvailable(ScanPatternHandle ScanPattern, AcquisitionType AcqType);

	/// \fn BOOL checkAvailableMemoryForRawData(OCTDeviceHandle Dev, ScanPatternHandle Pattern, ptrdiff_t AdditionalMemory)
	/// \brief Checks whether sufficient memory is available for raw data acquired with the specified scan pattern.
	/// 
	/// \par AdditionalMemory The parameter specifies additional memory that will be required during the measurement (from startMeasurement() 
	/// to stopMeasruement()) unknown to the SDK and/or memory that will be freed/available prior to the call of startMeasurement(). 
	SPECTRALRADAR_API BOOL checkAvailableMemoryForRawData(OCTDeviceHandle Dev, ScanPatternHandle Pattern, ptrdiff_t AdditionalMemory);

	/// \fn double QuantumEfficiency(OCTDeviceHandle Dev, double CenterWavelength_nm, double PowerIntoSpectrometer_W, DataHandle Spectrum_e)
	/// \ingroup Hardware
	/// \brief Calculates the quantum efficiency from the processed input spectrum in the Data instance.
	SPECTRALRADAR_API double QuantumEfficiency(OCTDeviceHandle Dev, double CenterWavelength_nm, double PowerIntoSpectrometer_W, DataHandle Spectrum_e);

	/// \fn void determineSurface(DataHandle Volume, DataHandle Surface);
	/// \ingroup Data
	/// \brief Performs a minimal segmentation of the data, by finding a surface that is compromised of the highes signals from each A-scan.
	/// From the 3D input data, the output data will 2D data, where each data pixel contains the depth of the respective surface as a funciton of the x- and y-pixel position. 
	SPECTRALRADAR_API void determineSurface(DataHandle Volume, DataHandle Surface);

	/// \fn unsigned long long getFreeMemory();
	/// \brief Returns the amount of free system memory. Function is available for convenience. 
	SPECTRALRADAR_API unsigned long long getFreeMemory();

	/// \fn void absComplexData(ComplexDataHandle ComplexData, DataHandle Abs)
	/// \ingroup Data
	/// \brief Converts the complex values from the #ComplexDataHandle to its absolute values and writes them to #DataHandle.
	SPECTRALRADAR_API void absComplexData(ComplexDataHandle ComplexData, DataHandle Abs);

	/// \fn void logAbsComplexData(ComplexDataHandle ComplexData, DataHandle dB)
	/// \ingroup Data
	/// \brief Converts the complex values from the #ComplexDataHandle to its dB values and writes them to #DataHandle.
	SPECTRALRADAR_API void logAbsComplexData(ComplexDataHandle ComplexData, DataHandle dB);

	/// \fn void argComplexData(ComplexDataHandle ComplexData, DataHandle Arg)
	/// \ingroup Data
	/// \brief Converts the complex values from the #ComplexDataHandle to its phase angle values and writes them to #DataHandle.
	SPECTRALRADAR_API void argComplexData(ComplexDataHandle ComplexData, DataHandle Arg);

	/// \fn void realComplexData(ComplexDataHandle ComplexData, DataHandle Real)
	/// \ingroup Data
	/// \brief Writes the real part of the complex values from the #ComplexDataHandle to #DataHandle.
	SPECTRALRADAR_API void realComplexData(ComplexDataHandle ComplexData, DataHandle Real);

	/// \fn void imagComplexData(ComplexDataHandle ComplexData, DataHandle Imag)
	/// \ingroup Data
	/// \brief Writes the imaginary part of the complex values from the #ComplexDataHandle to #DataHandle.
	SPECTRALRADAR_API void imagComplexData(ComplexDataHandle ComplexData, DataHandle Imag);

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

	/// \enum ComplexFilterType2D
	/// \ingroup PostProcessing
	/// \brief Specifies the type of filter to be applied to complex data.
	typedef enum {
		/// A filter applied to complex data to get a phase contrast image.
		FilterComplex2D_PhaseContrast
	} ComplexFilterType2D;

	/// \enum FilterType1D
	/// \ingroup PostProcessing
	/// \brief Specifies the type of 1D-filter to be applied. All filters are normalized.
	typedef enum {
		/// A gaussian 1D-filter of size 5 to smooth the data 
		Filter1D_Gaussian_5
	} FilterType1D;

	/// \enum FilterType2D
	/// \ingroup PostProcessing
	/// \brief Specifies the type of 2D-filter to be applied. All filters are normalized.
	typedef enum {
		/// A gaussian filter of size 3x3 to smooth the data 
		Filter2D_Gaussian_3x3,
		/// A gaussian filter of size 5x5 to smooth the data
		Filter2D_Gaussian_5x5,
		/// Horizontal prewitt filter of size 3x3 to detect edges in horizontal direction
		Filter2D_Prewitt_Horizontal_3x3,
		/// Vertical prewitt filter of size 3x3 to detect edges in vertical direction
		Filter2D_Prewitt_Vertical_3x3,
		/// Maximum of horizontal and vertical prewitt filter each of size 3x3 to detect edges
		Filter2D_NonlinearPrewitt_3x3,
		/// Horizontal sobel filter of size 3x3 to detect edges in horizontal direction while smoothing in vertical direction
		Filter2D_Sobel_Horizontal_3x3,
		/// Vertical prewitt filter of size 3x3 to detect edges in vertical direction while smoothing in horizontal direction
		Filter2D_Sobel_Vertical_3x3,
		/// Maximum of horizontal and vertical sobel filter each of size 3x3 to detect edges while smoothing the data simultaneously
		Filter2D_NonlinearSobel_3x3,
		/// Laplacian filter of size 3x3 to detect horizontal and vertical edges, no diagonal egdes
		Filter2D_Laplacian_NoDiagonal_3x3,
		/// Laplacian filter of size 3x3 to detect horizontal, vertical and diagonal edges 
		Filter2D_Laplacian_3x3
	} FilterType2D;

	/// \enum FilterType3D
	/// \ingroup PostProcessing
	/// \brief Specifies the type of 3D-filter to be applied. All filters are normalized.
	typedef enum {
		/// A gaussian filter of size 3x3 to smooth the data 
		Filter3D_Gaussian_3x3x3
	} FilterType3D;

	/// This functions assumes that the data contains an A-scan and performs A-scan specific analysis on it.
	///
	/// \fn void determineDynamicRange_dB(DataHandle Data, double* MinRange_dB, double* MaxRange_dB);
	/// \ingroup PostProcessing
	/// \brief Gives a rough estimation of the dynamic range of the specified data object.
	/// \param Data The #DataHandle the filter will be applied to
	/// \param MinRange_dB Used to return the lower bound of the dynamic range
	/// \param MaxRange_dB Used to return the upper bound of the dynamic range
	SPECTRALRADAR_API void determineDynamicRange_dB(DataHandle Data, double* MinRange_dB, double* MaxRange_dB);

	/// \fn void determineDynamicRangeWithMinRange_dB(DataHandle Data, double* MinRange_dB, double* MaxRange_dB, double MinDynamicRange_dB);
	/// \ingroup PostProcessing
	/// \brief Gives a rough estimation of the dynamic range of the specified data object.
	/// \param Data The #DataHandle the filter will be applied to
	/// \param MinRange_dB Used to return the lower bound of the dynamic range
	/// \param MaxRange_dB Used to return the upper bound of the dynamic range
	/// \param MinDynamicRange_dB Minimal size of the returned dynamic range interval in dB
	SPECTRALRADAR_API void determineDynamicRangeWithMinRange_dB(DataHandle Data, double* MinRange_dB, double* MaxRange_dB, double MinDynamicRange_dB);

	/// \fn void medianFilter1D(DataHandle Data, int Rank, Direction FilterDirection)
	/// \ingroup PostProcessing
	/// \brief Computes a 1D-median filter on the specified data. 
	/// \param Data The #DataHandle the filter will be applied to
	/// \param Rank The size of the filter
	/// \param FilterDirection The direction the 1D-filter will be applied to the data.
	SPECTRALRADAR_API void medianFilter1D(DataHandle Data, int Rank, Direction FilterDirection);

	/// \fn void medianFilter2D(DataHandle Data, int Rank, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Computes a 2D-median filter on the specified data. 
	/// \param Data The #DataHandle the filter will be applied to
	/// \param Rank The size of the filter 
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the data.
	SPECTRALRADAR_API void medianFilter2D(DataHandle Data, int Rank, Direction FilterNormalDirection);

	/// \fn void pepperFilter2D(DataHandle Data, PepperFilterType Type, float Threshold, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Removes pepper-noise (very low values, i. e. dark spots in the data). This enhances the visual (colored) representation of the data.
	/// \param Data The #DataHandle the filter will be applied to
	/// \param Type The type of the pepper filter chosen from #PepperFilterType
	/// \param Threshold If the value is lower than the given value it will be replaced by the mean
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the data
	///
	/// The pepper filter compares all pixels to a mean of surrounding pixels. The surrounding pixels taking into account are specified by #PepperFilterType. 
	/// If the pixels is lower than specified by the Threshold the pixel will be replaced by the mean.
	SPECTRALRADAR_API void pepperFilter2D(DataHandle Data, PepperFilterType Type, float Threshold, Direction FilterNormalDirection);

	/// \fn void convolutionFilter1D(DataHandle Data, int Size, float* FilterKernel, Direction FilterDirection)
	/// \ingroup PostProcessing
	/// \brief Calculates a mathematical convolution of the Data and the 1D-FilterKernel
	/// \param Data The #DataHandle the filter will be applied to
	/// \param Size Size of the filter
	/// \param FilterKernel Pointer to the array containing the filter kernel
	/// \param FilterDirection The filter direction the 1D-filter will be applied to the data, e.g. Direction_1 for filtering each single A-scan
	SPECTRALRADAR_API void convolutionFilter1D(DataHandle Data, int FilterSize, float* FilterKernel, Direction FilterDirection);

	/// \fn void convolutionFilter2D(DataHandle Data, int FilterSize1, int FilterSize2, float* FilterKernel, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Calculates a mathematical convolution of the Data and the 2D-FilterKernel
	/// \param Data The #DataHandle the filter will be applied to
	/// \param FilterSize1 Size of the first dimension of the filter
	/// \param FilterSize2 Size of the second dimension of the filter
	/// \param FilterKernel Pointer to the array containing the filter kernel
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the data, e.g. Direction_3 for filtering each single B-scan
	SPECTRALRADAR_API void convolutionFilter2D(DataHandle Data, int FilterSize1, int FilterSize2, float* FilterKernel, Direction FilterNormalDirection);

	/// \fn void convolutionFilter3D(DataHandle Data, int FilterSize1, int FilterSize2, int FilterSize3, float* FilterKernel)
	/// \ingroup PostProcessing
	/// \brief Calculates a mathematical convolution of the Data and the 3D-FilterKernel
	/// \param Data The #DataHandle the filter will be applied to
	/// \param FilterSize1 Size of the first dimension of the filter
	/// \param FilterSize2 Size of the second dimension of the filter
	/// \param FilterSize3 Size of the third dimension of the filter
	/// \param FilterKernel Pointer to the array containing the filter kernel
	SPECTRALRADAR_API void convolutionFilter3D(DataHandle Data, int FilterSize1, int FilterSize2, int FilterSize3, float* FilterKernel);

	/// \fn void predefinedFilter1D(DataHandle Data, FilterType1D Filter, Direction FilterDirection);
	/// \ingroup PostProcessing
	/// \brief Applies the predefined 1D-Filter to the Data
	/// \param Data The #DataHandle the filter will be applied to
	/// \param Filter Selection of a predefined filter #FilterType1D
	/// \param FilterDirection The filter direction the 1D-filter will be applied to the data, e.g. Direction_1 for filtering each single A-scan
	SPECTRALRADAR_API void predefinedFilter1D(DataHandle Data, FilterType1D Filter, Direction FilterDirection);

	/// \fn void predefinedFilter2D(DataHandle Data, FilterType2D Filter, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Applies the predefined 2D-Filter to the Data
	/// \param Data The #DataHandle the filter will be applied to
	/// \param Filter Selection of a predefined filter #FilterType2D
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the data, e.g. Direction_3 for filtering each single B-scan
	SPECTRALRADAR_API void predefinedFilter2D(DataHandle Data, FilterType2D Filter, Direction FilterNormalDirection);

	/// \fn void predefinedFilter3D(DataHandle Data, FilterType3D FilterType);
	/// \ingroup PostProcessing
	/// \brief Applies the predefined 3D-Filter to the Data
	/// \param Data The #DataHandle the filter will be applied to
	/// \param FilterType Selection of a predefined filter #FilterType3D
	SPECTRALRADAR_API void predefinedFilter3D(DataHandle Data, FilterType3D FilterType);

	/// \fn void predefinedComplexFilter2D(ComplexDataHandle ComplexData, ComplexFilterType2D Type, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Applies the predefined 2D-Filter to the ComplexData
	/// \param ComplexData The #ComplexDataHandle the filter will be applied to
	/// \param Type Chosen predefined filter for complex data. See #ComplexFilterType2D for selection.
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the complex data, e.g. Direction_3 for filtering each single B-scan
	SPECTRALRADAR_API void predefinedComplexFilter2D(ComplexDataHandle ComplexData, ComplexFilterType2D Type, Direction FilterNormalDirection);

	/// \fn void darkFieldComplexFilter2D(ComplexDataHandle ComplexData, double Radius, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Filters the image such that the image contrast comes from light scattered by the sample.
	/// \param ComplexData The #ComplexDataHandle the filter will be applied to.
	/// \param Radius Parameter to adjust the image contrast.
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the complex data, e.g. Direction_3 for filtering each single B-scan
	SPECTRALRADAR_API void darkFieldComplexFilter2D(ComplexDataHandle ComplexData, double Radius, Direction FilterNormalDirection);

	/// \fn void brightFieldComplexFilter2D(ComplexDataHandle ComplexData, double Radius, Direction FilterNormalDirection)
	/// \ingroup PostProcessing
	/// \brief Filters the image such that the image contrast comes from absorbance of light in the sample.
	/// \param ComplexData The #ComplexDataHandle the filter will be applied to.
	/// \param Radius Parameter to adjust the image contrast.
	/// \param FilterNormalDirection The normal of the direction the 2D-filter will be applied to the complex data, e.g. Direction_3 for filtering each single B-scan
	SPECTRALRADAR_API void brightFieldComplexFilter2D(ComplexDataHandle ComplexData, double Radius, Direction FilterNormalDirection);

	/// \fn void polynomialFitAndEval1D(int Size, const float* OrigPosX, const float* OrigY, int DegreePolynom, int EvalSize, const float* EvalPosX, float* EvalY)
	/// \ingroup Math
	/// \brief Computes the polynomial fit of the given 1D data.
	/// \param Size The size of the arrays OrigPosX and OrigY
	/// \param OrigPosX The x-positions of the OrigY of the given data.
	/// \param OrigY The y-values to the belonging OrigPosX of the given data.
	/// \param DegreePolynom The degree of the polynomial for the fit.
	/// \param EvalSize The size of the array EvalPosX.
	/// \param EvalPosX The x-positions for evaluation the polynomial fit. 
	/// \param EvalY The resulting y-values belonging to the given positions EvalPosX.
	SPECTRALRADAR_API void polynomialFitAndEval1D(int Size, const float* OrigPosX, const float* OrigY, int DegreePolynom, int EvalSize, const float* EvalPosX, float* EvalY);

	/// \fn float calcParabolaMaximum(float x0, float y0, float yLeft, float yRight, float* peakHeight)
	/// \ingroup Math
	/// \brief Computes the x-position of the highest peak of the parabola given by the point x0, y0, yLeft, yRight. y0 needs to be the point with the highest value.
	/// \param x0 The x-position of the point with the highest value y0.
	/// \param y0 The value of x0.
	/// \param yLeft The y-value from the point left to x0. The distance (x0, xLeft) is assumed to be 1.
	/// \param yRight The y-value from the point right to x0. The distance (x0, xRight) is assumed to be 1.
	/// \param peakHeight The y-value of th highest peak of the parabloa will be written to this parameter.
	SPECTRALRADAR_API float calcParabolaMaximum(float x0, float y0, float yLeft, float yRight, float* peakHeight);

	/// \fn void crossCorrelatedProjection(DataHandle DataIn, DataHandle DataOut)
	/// \ingroup Data
	/// \brief Upon return DataOut contains an average of all B-Scans in DataIn. Right before averaging,
	///        the datasets are crosscorrelated to eliminate registration errors.
	SPECTRALRADAR_API void crossCorrelatedProjection(DataHandle DataIn, DataHandle DataOut);

	/// \fn void averagingResizeFilter(DataHandle DataIn, DataHandle DataOut, int Decimation1, int Decimation2, int Decimation3)
	/// \ingroup Data
	/// \brief Resizes a data object by averaging. The number of pixels that are averaged on each axis can be specified by the
	///        Decimation parameters. E.g. if Decimation2 is set to 3, Dimension2 of the resulting DataOut object will be 1/3 of
	///        Dimension2 of DataIn.
	/// \param DataIn Input #DataHandle
	/// \param DataOut Output #DataHandle (needs to be initialized)
	/// \param Decimation1 Number of pixels to average in Dimension1
	/// \param Decimation2 Number of pixels to average in Dimension2
	/// \param Decimation3 Number of pixels to average in Dimension3
	SPECTRALRADAR_API void averagingResizeFilter(DataHandle DataIn, DataHandle DataOut, int Decimation1, int Decimation2, int Decimation3);

	/// \fn void thresholdDopplerData(DataHandle Phase, DataHandle Intensity, float intensityThreshold, float phaseTargetValue)
	/// \ingroup Data
	/// \brief At points whose Intensity does not exceed the intensityThreshold, the phase is set to the phaseTargetValue.
	SPECTRALRADAR_API void thresholdDopplerData(DataHandle Phase, DataHandle Intensity, float intensityThreshold, float phaseTargetValue);

	/// used in SR Service, do not remove
	/// \fn void getCurrentIntensityStatistics(OCTDeviceHandle Dev, ProcessingHandle Proc, float* relToRefIntensity, float* relToProjAbsIntensity);
	/// \ingroup Data
	/// \brief Returns two statistical interpretations of the current light intensity on the sensor.
	SPECTRALRADAR_API void getCurrentIntensityStatistics(OCTDeviceHandle Dev, ProcessingHandle Proc, float* relToRefIntensity, float* relToProjAbsIntensity);

	///
	/// \fn int getNumberOfProbeConfigs()
	/// \ingroup Probe
	/// \brief Returns the number of available probe configuration files.
	SPECTRALRADAR_API int getNumberOfProbeConfigs();

	/// \fn void getProbeConfigName(int Index, char* ProbeName, int StringSize)
	/// \ingroup Probe
	/// \brief Returns the name of the specified probe configuration file.
	/// \param Index Selects one specific configuration file from all available probe configuration files.
	/// \param ProbeName Return value for the name of the probe configuration file.
	/// \param StringSize The length of the returned char*.
	SPECTRALRADAR_API void getProbeConfigName(int Index, char* ProbeName, int StringSize);

	/// \enum ObjectivePropertyString
	/// \ingroup Probe
	/// \brief Properties of the objective mounted to the scanner such as the name. 
	typedef enum {
		/// Human-readable name of the objective to display in calibration process, or as device info.
		Objective_DisplayName,
		/// The mount specification is used to find the compatible probes and objectives (to be found in .ordf and .prdf files).
		Objective_Mount
	} ObjectivePropertyString;

	/// \enum ObjectivePropertyInt
	/// \ingroup Probe
	/// \brief Properties of the objective mounted to the scanner such as valid scan range in mm.
	typedef enum {
		/// The maximum range in mm of the x-direction for the specified objective
		Objective_RangeMaxX_mm,
		/// The maximum range in mm of the y-direction for the specified objective
		Objective_RangeMaxY_mm
	} ObjectivePropertyInt;

	/// \enum ObjectivePropertyFloat
	/// \ingroup Probe
	/// \brief Properties of the objective mounted to the scanner such as the focal length of the lens.
	typedef enum {
		/// Focal length in mm of the specidifed objective
		Objective_FocalLength_mm,
		/// Optical path length, in millimeter (without counting the focal length, multiplied by the equivalent refractive index).
		Objective_OpticalPathLength
	} ObjectivePropertyFloat;

	/// \enum ProbeScanRangeShape
	/// \ingroup Probe
	/// \brief The shape of the maximal valid scan range.
	typedef enum {
		/// The shape of the valid scan range for the specified objective
		Probe_ScanRange_Rectangular,
		/// The maximum range in mm of the y-direction for the specified objective
		Probe_ScanRange_Round
	} ProbeScanRangeShape;

	/// \fn int getNumberOfAvailableProbes(void)
	/// \ingroup Probe
	/// \brief Returns the number of the available probe types.
	/// \return The number of the available probe types.
	SPECTRALRADAR_API int getNumberOfAvailableProbes(void);

	/// \fn void getAvailableProbe(int Index, char* ProbeName, int StringSize)
	/// \ingroup Probe
	/// \brief Returns the name of the desired probe type.
	/// \param[in] Index Selects one specific probe type from all available ones.
	/// \param[out] ProbeName The desired string with the name of the probe type, e.g. standard, user-customizable or compact handheld.
	///                       This string is essentially the name of the corresponding .prdf file, except that a version number and the
	///                       termination should be added.
	/// \param[in] StringSize The length of the returned char*.
	SPECTRALRADAR_API void getAvailableProbe(int Index, char* ProbeName, int StringSize);

	/// \fn void getProbeDisplayName(const char* ProbeName, char* DisplayName, int StringSize)
	/// \ingroup Probe
	/// \brief Returns the display name for the probe name specified.
	/// \param[in] ProbeName Name of the probe.
	///                       This string is essentially the name of the corresponding .prdf file, except that a version number and the
	///                       termination should be added.
	/// \param[out] DisplayName The string to be shown in OCTImage software.
	/// \param[in] StringSize The length of the returned char*.
	SPECTRALRADAR_API void getProbeDisplayName(const char* ProbeName, char* DisplayName, int StringSize);

	/// \fn void getObjectiveDisplayName(const char* ObjectiveName, char* DisplayName, int StringSize)
	/// \ingroup Probe
	/// \brief Returns the display name for the objective name specified.
	/// \param[in] ObjectiveName Name of the objective.
	///                          This string is essentially the name of the corresponding .odf file, except that a version number and the
	///                          termination should be added.
	/// \param[out] DisplayName The string to be shown in OCTImage software.
	/// \param[in] StringSize The length of the returned char*.
	SPECTRALRADAR_API void getObjectiveDisplayName(const char* ObjectiveName, char* DisplayName, int StringSize);

	/// \fn int getNumberOfCompatibleObjectives(const char* ProbeName)
	/// \ingroup Probe
	/// \brief Returns the number of objectives compatible with the specified objective mount.
	/// \param[in] ProbeName The name of the probe, as retrieved with the function #getAvailableProbe.
	/// \return The number of objectives compatible with the specified probe name.
	SPECTRALRADAR_API int getNumberOfCompatibleObjectives(const char* ProbeName);

	/// \fn void getCompatibleObjective(int Index, const char* ProbeName, char* Objective, int StringSize)
	/// \ingroup Probe
	/// \brief Returns the name of the specified objective for the selected probe type.
	/// \param[in] Index Selects one specific objective from all available objective for the specified probe type.
	/// \param[in] ProbeName The name of the probe, as retrieved with the function #getAvailableProbe.
	/// \param[out] Objective Return value for the name of the objective file.
	///                       This string is essentially the name of the corresponding .odf file, except that a version number and the
	///                       termination will be added.
	/// \param[in] StringSize The length of the returned char*.
	SPECTRALRADAR_API void getCompatibleObjective(int Index, const char* ProbeName, char* Objective, int StringSize);

	/// \fn ProbeScanRangeShape getProbeMaxScanRangeShape(ProbeHandle Probe)
	/// \ingroup Probe
	/// \brief Returns the shape of the valid scan range for the #ProbeHandle. All possible scan range are defined in #ProbeScanRangeShape.
	/// \param[in] Probe Specified #ProbeHandle.
	SPECTRALRADAR_API ProbeScanRangeShape getProbeMaxScanRangeShape(ProbeHandle Probe);

	/// \fn ProbeScanRangeShape setProbeMaxScanRangeShape(ProbeHandle Probe, ProbeScanRangeShape Shape)
	/// \ingroup Probe
	/// \brief Sets the \p Shape of the valid scan range for the #ProbeHandle. All possible scan-range shapes are defined in #ProbeScanRangeShape.
	/// \param[in] Probe Specified #ProbeHandle.
	/// \param[in] Shape the desired shape, which should be in the range defined by #ProbeScanRangeShape.
	SPECTRALRADAR_API void setProbeMaxScanRangeShape(ProbeHandle Probe, ProbeScanRangeShape Shape);

	/// \fn void setQuadraticProbeFactors(ProbeHandle Probe, double* QuadFactorsX, double* QuadFactorsY, int NumberOfFactors)
	/// \ingroup Probe
	/// \brief Sets the probe calibration factors
	/// \param[in] Probe Specified #ProbeHandle.
	/// \param[in] QuadFactorsX the specidifed quadratic factors for the x-axis.
	/// \param[in] QuadFactorsY the specidifed quadratic factors for the y-axis.
	/// \param[in] NumberOfFactors the number of quadartic factors and the length of the specified arrays QuadFactorsX and QuadFactorsY.
	SPECTRALRADAR_API void setQuadraticProbeFactors(ProbeHandle Probe, double* QuadFactorsX, double* QuadFactorsY, int NumberOfFactors);

	/// \fn int getObjectivePropertyInt(const char* Objective, ObjectivePropertyInt Selection)
	/// \ingroup Probe
	/// \brief Returns the selected #ObjectivePropertyInt for the chosen objective.
	/// \param Objective Specifies the name of the objective.
	/// \param Selection Specifies the #ObjectivePropertyInt property. 
	SPECTRALRADAR_API int getObjectivePropertyInt(const char* Objective, ObjectivePropertyInt Selection);

	/// \fn double getObjectivePropertyFloat(const char* Objective, ObjectivePropertyFloat Selection)
	/// \ingroup Probe
	/// \brief Returns the selected #ObjectivePropertyFloat for the chosen objective.
	/// \param Objective Specifies the name of the objective.
	/// \param Selection Specifies the #ObjectivePropertyFloat property.
	SPECTRALRADAR_API double getObjectivePropertyFloat(const char* Objective, ObjectivePropertyFloat Selection);

	/// \fn const char* getObjectivePropertyString(const char* Objective, ObjectivePropertyString Selection);
	/// \ingroup Probe
	/// \brief Returns the selected #ObjectivePropertyString for the chosen objective. Warning: The returned const char* will only be valid until the next call to #getObjectivePropertyString.
	SPECTRALRADAR_API const char* getObjectivePropertyString(const char* Objective, ObjectivePropertyString Selection);

	/// \typedef cbProbeMessageReceived
	/// \ingroup Probe
	/// \brief The prototype for callback functions registered for probe button events. As of the creation time of this document, only the OCTH probe is equipped with buttons.
	/// \param int Zero-based ID of the pressed button
	typedef void(__stdcall* cbProbeMessageReceived)(int);

	/// \fn void addProbeButtonCallback(OCTDeviceHandle Dev, cbProbeMessageReceived Callback);
	/// \ingroup Probe
	/// \brief Registers a callback function to notify when a button on the probe has been pressed. The int parameter passed to the callback function will contain the pressed button's ID. Caution: Since the callbacks will not be called in separate threads but in the order of addition, make sure that the callback function returns as soon as possible.
	SPECTRALRADAR_API void addProbeButtonCallback(OCTDeviceHandle Dev, cbProbeMessageReceived Callback);

	/// \fn void removeProbeButtonCallback(OCTDeviceHandle Dev, cbProbeMessageReceived Callback);
	/// \ingroup Probe
	/// \brief Removes a previously registered probe button callback function
	SPECTRALRADAR_API void removeProbeButtonCallback(OCTDeviceHandle Dev, cbProbeMessageReceived Callback);

	/// \enum RefstageStatus
	/// \ingroup Hardware
	/// \brief Defines the status of the motorized reference stage.
	typedef enum ReferenceStatus_
	{
		/// The reference stage is not busy and available for a task.
		RefStage_Status_Idle = 0,
		/// The reference stage is in its homing process. Please wait until this process is finished.
		RefStage_Status_Homing = 1,
		/// The reference stage is moving, you can stop this movement with #stopRefstageMovement
		RefStage_Status_Moving = 2,
		/// The reference stage it moving to a certain position. Please wait until this process is finished.
		RefStage_Status_MovingTo = 3,
		/// The reference stage is in the stopping process after #stopRefstageMovement was called. Please wait until this process is finished.
		RefStage_Status_Stopping = 4,
		/// The reference stage is not available any more.
		RefStage_Status_NotAvailable = 5,
		/// The status of the reference stage is not defined.
		RefStage_Status_Undefined = -1
	} RefstageStatus;

	/// \enum RefstageSpeed
	/// \ingroup Hardware
	/// \brief Defines the velocity of movement for the motorized reference stage.
	typedef enum
	{
		/// Slow speed (~0.4mm/s)
		RefStage_Speed_Slow = 0,
		/// Fast speed (~1.8mm/s)
		RefStage_Speed_Fast = 1,
		/// Very slow speed
		RefStage_Speed_VerySlow = 2,
		/// Very fast speed (~13mm/s)
		RefStage_Speed_VeryFast = 3
	} RefstageSpeed;

	/// \enum RefstageWaitForMovement
	/// \ingroup Hardware
	/// \brief Defines the behaviour whether the the function should wait until the movement of the motorized reference stage has stopped to return.
	typedef enum RefstageWaitForMovement_
	{
		/// Function waits until the movement has stopped before it returns
		RefStage_Movement_Wait = 0,
		/// The movement of the motorized reference stage will be started and runs in another thread. The function returns while the reference stage is still moving.
		RefStage_Movement_Continue = 1
	} RefstageWaitForMovement;

	/// \enum RefstageMovementDirection
	/// \ingroup Hardware
	/// \brief Defines the direction of movement for the motorized reference stage. Please note that not in all systems a motorized reference stage is present.
	typedef enum
	{
		/// Shortens reference arm length
		RefStage_MoveShorter = 0,
		/// Extends reference arm length
		RefStage_MoveLonger = 1
	} RefstageMovementDirection;

	/// \typedef cbRefstageStatusChanged	/// \ingroup Hardware
	/// \brief Defines the function prototype for the reference stage status callback (see also setRefstageStatusCallback()). The argument contains the current status of the reference stage when called.
	/// \param RefstageStatus Current status of the reference stage
	typedef void(__stdcall* cbRefstageStatusChanged)(RefstageStatus);

	/// \typedef cbRefstagePositionChanged	/// \ingroup Hardware
	/// \brief Defines the function prototype for the reference stage position change callback (see also setRefstagePosChangedCallback()). The argument contains the reference stage position in mm when called.
	/// \param double Current position of the reference stage in mm
	typedef void(__stdcall* cbRefstagePositionChanged)(double);

	/// \fn SPECTRALRADAR_API BOOL isRefstageAvailable(OCTDeviceHandle Dev);
	/// \ingroup Hardware
	/// \brief Returns whether a motorized reference stage is available or not for the specified device. Please note that a motorized reference stage is not included in all systems.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	SPECTRALRADAR_API BOOL isRefstageAvailable(OCTDeviceHandle Dev);

	/// \fn RefstageStatus getRefstageStatus(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Returns the current status of the reference stage, e.g. if it is moving.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	SPECTRALRADAR_API RefstageStatus getRefstageStatus(OCTDeviceHandle Dev);

	/// \fn double getRefstageLength_mm(OCTDeviceHandle Dev, ProbeHandle Probe)
	/// \ingroup Hardware
	/// \brief Returns the total length in mm of the reference stage.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe the #ProbeHandle that was initially provided by initProbe.
	SPECTRALRADAR_API double getRefstageLength_mm(OCTDeviceHandle Dev, ProbeHandle Probe);

	/// \fn double getRefstagePosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe)
	/// \ingroup Hardware
	/// \brief Returns the current position in mm of the reference stage.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe the #ProbeHandle that was initially provided by initProbe.
	SPECTRALRADAR_API double getRefstagePosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe);

	/// \fn void homeRefstage(OCTDeviceHandle Dev, RefstageWaitForMovement WaitForMoving)
	/// \ingroup Hardware
	/// \brief Homes the reference stage to calibrate the zero position.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param WaitForMoving specifies whether to wait for the end of the homing process before returning from the function or not.
	SPECTRALRADAR_API void homeRefstage(OCTDeviceHandle Dev, RefstageWaitForMovement WaitForMoving);

	/// \fn void moveRefstageToPosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe, double pos_mm, RefstageSpeed Speed, RefstageWaitForMovement WaitForMoving)
	/// \ingroup Hardware
	/// \brief Moves the reference stage to the specified position in mm.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe the #ProbeHandle that was initially provided by initProbe.
	/// \param pos_mm gives the desired position in mm
	/// \param Speed is the velocity of the reference stage movement specified with #RefstageSpeed
	/// \param WaitForMoving defines whether the function should wait until the movement of the reference stage has stopped or not until it returns
	SPECTRALRADAR_API void moveRefstageToPosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe, double Pos_mm, RefstageSpeed Speed, RefstageWaitForMovement WaitForMoving);

	/// \fn void moveRefstage_mm(OCTDeviceHandle Dev, ProbeHandle Probe, double Length_mm, RefstageMovementDirection Direction, RefstageSpeed Speed, RefstageWaitForMovement WaitForMoving)
	/// \ingroup Hardware
	/// \brief Moves the reference stage with the specified length in mm.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe the #ProbeHandle that was initially provided by initProbe.
	/// \param Length_mm gives the desired length in mm relative to the current position
	/// \param Direction is the specified direction of the movement with #RefstageMovementDirection.
	/// \param Speed is the velocity of the reference stage movement specified with #RefstageSpeed
	/// \param WaitForMoving defines whether the function should wait until the movement of the reference stage has stopped or not until it returns
	SPECTRALRADAR_API void moveRefstage_mm(OCTDeviceHandle Dev, ProbeHandle Probe, double Length_mm, RefstageMovementDirection Direction, RefstageSpeed Speed, RefstageWaitForMovement WaitForMoving);

	/// \fn void startRefstageMovement(OCTDeviceHandle Dev, RefstageMovementDirection Direction, RefstageSpeed Speed)
	/// \ingroup Hardware
	/// \brief Starts the movement of the reference stage with the chosen speed. Please note that the movement does not stop until #stopRefstageMovement is called.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Direction is the specified direction of the movement with #RefstageMovementDirection.
	/// \param Speed is the velocity of the reference stage movement specified with #RefstageSpeed.
	SPECTRALRADAR_API void startRefstageMovement(OCTDeviceHandle Dev, RefstageMovementDirection Direction, RefstageSpeed Speed);

	/// \fn void stopRefstageMovement(OCTDeviceHandle Dev)
	/// \ingroup Hardware
	/// \brief Stops the movement of the reference stage.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	SPECTRALRADAR_API void stopRefstageMovement(OCTDeviceHandle Dev);

	/// \fn void setRefstageSpeed(OCTDeviceHandle Dev, RefstageSpeed Speed)
	/// \ingroup Hardware
	/// \brief Sets the velocity of the movement of the reference stage.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Speed the chosen velocity of the movement.
	SPECTRALRADAR_API void setRefstageSpeed(OCTDeviceHandle Dev, RefstageSpeed Speed);

	/// \fn void setRefstageStatusCallback(OCTDeviceHandle Dev, cbRefstageStatusChanged Callback)
	/// \ingroup Hardware
	/// \brief Registers the callback to get notified if the reference stage status changed.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Callback to register. 
	SPECTRALRADAR_API void setRefstageStatusCallback(OCTDeviceHandle Dev, cbRefstageStatusChanged Callback);

	/// \fn void setRefstagePosChangedCallback(OCTDeviceHandle Dev, cbRefstagePositionChanged Callback)
	/// \ingroup Hardware
	/// \brief Registers the callback to get notified if the reference stage position changed.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Callback to register. 
	SPECTRALRADAR_API void setRefstagePosChangedCallback(OCTDeviceHandle Dev, cbRefstagePositionChanged Callback);

	/// \fn double getRefstageMinPosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe)
	/// \ingroup Hardware
	/// \brief Returns the minimal position in mm the reference stage can move to.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe the #ProbeHandle that is currently. 
	SPECTRALRADAR_API double getRefstageMinPosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe);

	/// \fn double getRefstageMaxPosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe)
	/// \ingroup Hardware
	/// \brief Returns the maximal position in mm the reference stage can move to.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Probe the #ProbeHandle that is currently. 
	SPECTRALRADAR_API double getRefstageMaxPosition_mm(OCTDeviceHandle Dev, ProbeHandle Probe);

	// After leaving the SLD on for the specified amount of time, it will be switched off, automatically
	// Controlled by following functions/enums/callbacks.

	/// \enum LightSourceState
	/// \ingroup Hardware
	/// \brief Values that define the state of the light source.
	typedef enum LightSourceState_
	{
		Activating,
		On, 
		Off
	} LightSourceState;

	/// \ingroup Hardware
	/// \brief Defines the function prototype for the light source callback(see also #setLightSourceTimeoutCallback()). The argument contains the current state of the light source.
	/// \param LightSourceState Current state of the light source
	typedef void(__stdcall* lightSourceStateCallback)(LightSourceState);

	/// \fn void setLightSourceTimeoutCallback(OCTDeviceHandle Dev, lightSourceStateCallback Callback);
	/// \ingroup Hardware
	/// \brief Sets a callback function that will be invoked by the SDK whenever the state of the lightsource of the device changes.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Callback the #lightSourceStateCallback that will be called when state of the lightsource changes
	SPECTRALRADAR_API void setLightSourceTimeoutCallback(OCTDeviceHandle Dev, lightSourceStateCallback Callback);

	/// \fn void setLightSourceTimeout_s(OCTDeviceHandle Dev, double Timeout);
	/// \ingroup Hardware
	/// \brief Sets a the timeout in seconds, after which the OCT lightsource will be turned off if no scanning is performed.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Timeout Time in seconds after which the lightsource will be turned off.
	SPECTRALRADAR_API void setLightSourceTimeout_s(OCTDeviceHandle Dev, double Timeout);

	/// \fn double getLightSourceTimeout_s(OCTDeviceHandle Dev);
	/// \ingroup Hardware
	/// \brief Gets a the timeout in seconds, after which the OCT lightsource will be turned off if no scanning is performed.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return Time in seconds after which the lightsource will be turned off.
	SPECTRALRADAR_API double getLightSourceTimeout_s(OCTDeviceHandle Dev);

	// Polarization Processing


	/// \defgroup Polarization Polarization 
	/// \brief Polarization Sensitive OCT Processing Routines.
	///
	/// This section deals with polarization sensitive OCT (PS-OCT).

	/// \enum PolarizationDOPUFilterType
	/// \ingroup Polarization
	/// \brief Values that determine the behaviour of temporal filter, if enabled.
	typedef enum PolarizationDOPUFilterType_{
		/// Median
		PolarizationProcessing_DOPU_Median,
		/// Average
		PolarizationProcessing_DOPU_Average,
		/// Convolution with a Gaussian kernel
		PolarizationProcessing_DOPU_Gaussian,
		/// FFT convolution with a Gaussian kernel (preumably more efficient for very large kernels).
		PolarizationProcessing_DOPU_GaussianWithFFT,
	} PolarizationDOPUFilterType;


	/// \enum PolarizationPropertyInt
	/// \ingroup Polarization
	/// \brief Values that determine the behaviour of the Polarization processing routines.
	typedef enum {
		/// Number of pixels for DOPU averaging in the z-direction.
		PolarizationProcessing_DOPU_Z = 0,
		/// Number of pixels for DOPU averaging in the z-direction.
		PolarizationProcessing_DOPU_X = 1,
		/// Number of pixels for DOPU averaging in the y-direction.
		PolarizationProcessing_DOPU_Y = 2,
		/// DOPU filter specification. See #PolarizationDOPUFilterType.
		PolarizationProcessing_DOPU_FilterType = 3,
		/// Number of frames for averaging.
		PolarizationProcessing_BScanAveraging = 4,
		/// Number of frames actually averaged (transiently different from #PolarizationProcessing_BScanAveraging until enough frames are available).
		PolarizationProcessing_BScanAveraged = 9,
		/// Number of pixels for averaging along the x axis.
		PolarizationProcessing_AveragingZ = 5,
		/// Number of pixels for averaging along the y axis.
		PolarizationProcessing_AveragingX = 6,
		/// Number of pixels for averaging along the z axis.
		PolarizationProcessing_AveragingY = 7,
		/// A-Scan averaging. This parameter influences the way data get acquired, it cannot be changed for offline processing.
		PolarizationProcessing_AScanAveraging = 8,
	} PolarizationPropertyInt;

	/// \enum PolarizationPropertyFloat
	/// \ingroup Polarization
	/// \brief Values that determine the behaviour of the Polarization processing routines.
	typedef enum
	{
		/// Threshold value to enable/disable features of PS computation based on the total intensity value.
		PolarizationProcessing_IntensityThreshold_dB = 0,
		/// Correction angle (in radians) to get circularly polarized light at the upper surface of the sample. This angle
		/// is a compensation fo the polarization-mode-dispersion (PMD).  More in detail, this angle is used to compute a
		/// phasor (\f$\exp(i\alpha)\f$), that will be applied to the complex reflectivities vector associated with camera 0.
		PolarizationProcessing_PMDCorrectionAngle_rad = 1,
		/// Assuming a gaussian light source, the value of the wavenumber with maximal intensity (in nm).
		PolarizationProcessing_CentralWavelength_nm = 2,
		/// Refer to a particular orientation on the sample holder.
		/// The angle should be expressed in radians.
		PolarizationProcessing_OpticalAxisOffset_rad = 3,
	} PolarizationPropertyFloat;

	/// \enum PolarizationFlag
	/// \ingroup Polarization
	/// \brief Flags that determine the behaviour of the Polarization processing routines.
	typedef enum {
		/// TRUE if thresholding should be applied. In that case, the total intensity will be compared to the threshold.
		/// If lower, then the computed Stokes parameters will be set to zero.
		PolarizationProcessing_ApplyThresholding = 0,
		/// When post-processing a multiframe B-scan, this flag signals that the many frames are encoded along the Y-axis.
		PolarizationProcessing_YAxisIsFrameAxis = 1,
	} PolarizationFlag;

	/// \fn void setPolarizationFlag(PolarizationProcessingHandle Polarization, PolarizationFlag Flag, BOOL OnOff);
	/// \ingroup Polarization
	/// \brief Sets the polarization processing flags. 
	SPECTRALRADAR_API void setPolarizationFlag(PolarizationProcessingHandle Polarization, PolarizationFlag Flag, BOOL OnOff);

	/// \fn BOOL getPolarizationFlag(PolarizationProcessingHandle Polarization, PolarizationFlag Flag);
	/// \ingroup Polarization
	/// \brief Gets the desired polarization processing flag. 
	SPECTRALRADAR_API BOOL getPolarizationFlag(PolarizationProcessingHandle Polarization, PolarizationFlag Flag);

	/// \fn PolarizationProcessingHandle createPolarizationProcessing(void);
	/// \ingroup Polarization
	/// \brief Returns a Polarization processing handle to the Processing routines for polarization analysis.
	SPECTRALRADAR_API PolarizationProcessingHandle createPolarizationProcessing(void);

	/// \fn void clearPolarizationProcessing(PolarizationProcessingHandle)
	/// \ingroup Polarization
	/// \brief Clears the polarization processing routines and frees the memory that has been allocated for these to work properly.
	SPECTRALRADAR_API void clearPolarizationProcessing(PolarizationProcessingHandle Polarization);

	/// \fn int getPolarizationPropertyInt(PolarizationProcessingHandle Polarization, PolarizationPropertyInt Property)
	/// \ingroup Polarization
	/// \brief Gets the desired polarization processing property.
	SPECTRALRADAR_API int getPolarizationPropertyInt(PolarizationProcessingHandle Polarization, PolarizationPropertyInt Property);

	/// \fn void setPolarizationPropertyInt(PolarizationProcessingHandle Polarization, PolarizationPropertyInt Property, int Value);
	/// \ingroup Polarization
	/// \brief Sets polarization processing properties.
	SPECTRALRADAR_API void setPolarizationPropertyInt(PolarizationProcessingHandle Polarization, PolarizationPropertyInt Property, int Value);

	/// \fn double getPolarizationPropertyFloat(PolarizationProcessingHandle Polarization, PolarizationPropertyFloat Property);
	/// \ingroup Polarization
	/// \brief Gets the desired polarization processing floating-point property. 
	SPECTRALRADAR_API double getPolarizationPropertyFloat(PolarizationProcessingHandle Polarization, PolarizationPropertyFloat Property);

	/// \fn void setPolarizationPropertyFloat(PolarizationProcessingHandle Polarization, PolarizationPropertyFloat Property, double Value);
	/// \ingroup Polarization
	/// \brief Sets the desired polarization processing floating-point property. 
	SPECTRALRADAR_API void setPolarizationPropertyFloat(PolarizationProcessingHandle Polarization, PolarizationPropertyFloat Property, double Value);

	/// \fn void setPolarizationOutputI(PolarizationProcessingHandle Polarization, DataHandle Intensity)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting polarization intensity output (Stokes parameter I).
	SPECTRALRADAR_API void setPolarizationOutputI(PolarizationProcessingHandle Polarization, DataHandle Intensity);

	/// \fn void setPolarizationOutputQ(PolarizationProcessingHandle Polarization, DataHandle StokesQ)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting Stokes parameter Q.
	///
	/// The range of Q is [-1,1]. It takes the value -1 when the polarization is 100% parallel (zero degrees), and the value 1 when the polarization
	/// is 100% perpendicular (ninety degrees).
	SPECTRALRADAR_API void setPolarizationOutputQ(PolarizationProcessingHandle Polarization, DataHandle StokesQ);

	/// \fn void setPolarizationOutputU(PolarizationProcessingHandle Polarization, DataHandle StokesU)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting Stokes parameter U.
	///
	/// The range of U is [-1,1]. It takes the value -1 when the polarization is 100% at -45 degrees, and the value 1 when the polarization
	/// is 100% at 45 degrees.
	SPECTRALRADAR_API void setPolarizationOutputU(PolarizationProcessingHandle Polarization, DataHandle StokesU);

	/// \fn void setPolarizationOutputV(PolarizationProcessingHandle Polarization, DataHandle StokesV)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting Stokes parameter U.
	///
	/// The range of V is [-1,1]. It takes the value -1 when the reflected light is 100% left-hand circularly polarized, and the value 1 when
	/// the reflected light is 100% right-hand circularly polarized.
	SPECTRALRADAR_API void setPolarizationOutputV(PolarizationProcessingHandle Polarization, DataHandle StokesV);

	/// \fn void setPolarizationOutputDOPU(PolarizationProcessingHandle Polarization, DataHandle DOPU)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting DOPU.
	///
	/// The range of the DOPU is [0,1]. It takes the value when light appearsto be completely unpolarized, and 1 when the opposite is the case.
	SPECTRALRADAR_API void setPolarizationOutputDOPU(PolarizationProcessingHandle Polarization, DataHandle DOPU);

	/// \fn void setPolarizationOutputRetardation(PolarizationProcessingHandle Polarization, DataHandle Retardation)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting retardation.
	///
	/// The range of the retardation is [0,pi/2].
	SPECTRALRADAR_API void setPolarizationOutputRetardation(PolarizationProcessingHandle Polarization, DataHandle Retardation);

	/// \fn void setPolarizationOutputOpticAxis(PolarizationProcessingHandle Polarization, DataHandle OpticAxis)
	/// \ingroup Polarization
	/// \brief Sets the location of the resulting optic axis.
	///
	/// The range of the optic axil is [-pi/2,pi/2].
	SPECTRALRADAR_API void setPolarizationOutputOpticAxis(PolarizationProcessingHandle Polarization, DataHandle OpticAxis);

	/// \fn void executePolarizationProcessing(PolarizationProcessingHandle Polarization, ComplexDataHandle Data_P_Camera1, ComplexDataHandle Data_S_Camera0)
	/// \ingroup Polarization
	/// \brief Executes the polarization processing of the input data and returns, if previously setup, intensity, retardation, and phase differences.
	SPECTRALRADAR_API void executePolarizationProcessing(PolarizationProcessingHandle Polarization, ComplexDataHandle Data_P_Camera1, ComplexDataHandle PData_S_Camera0);

	/// \fn void saveFileMetadataPolarization(OCTFileHandle FileHandle, PolarizationProcessingHandle PolProc);
	/// \ingroup Polarization
	/// \brief Saves metadata to the specified file. These metadata specify the operational arguments needed by the polarization processing routines
	///        to redo the polarization-analysis starting from two #ComplexDataHandle delivered by \p Proc_0 and \p Proc_1.
	/// \param[in] FileHandle A valid (non null) handle of OCTFile (#OCTFileHandle), previously obtained with the function #createOCTFile.
	/// \param[in] PolProc A valid (non null) polarization-processing handle to the processing routines for polarization analysis.
	SPECTRALRADAR_API void saveFileMetadataPolarization(OCTFileHandle FileHandle, PolarizationProcessingHandle PolProc);

	/// \fn PolarizationProcessingHandle createPolarizationProcessingForFile(OCTFileHandle FileHandle)
	/// \ingroup Polarization
	/// \brief Loads metadata to the specified file. These metadata specify the operational arguments needed by the polarization processing routines
	///        to redo the polarization-analysis starting from two #ComplexDataHandle delivered by \p Proc_0 and \p Proc_1, exactly as they were done
	///        before the file was written.
	/// \param[in] FileHandle A valid (non null) handle of OCTFile (#OCTFileHandle), previously obtained with the function #createOCTFile.
	/// \return A valid (non null) polarization-processing handle to the processing routines for polarization analysis.
	SPECTRALRADAR_API PolarizationProcessingHandle createPolarizationProcessingForFile(OCTFileHandle FileHandle);

	/// \fn void updateAfterPresetChange(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc, int CameraIndex)
	/// \ingroup Hardware
	/// \brief Updates the processing handle after preset change. Please use #setDevicePreset first for the first camera (with index 0) and this function to update the corresponding #ProcessingHandle for the second camera (with index 1).
	/// \param[in] Dev A valid (non null) OCT device handle (#OCTDeviceHandle), previously generated with the function #initDevice.
	/// \param[in] Probe A handle to the probe (#ProbeHandle); whose galvo position is to be set.
	/// \param[in] Proc A valid (non null) processing handle.
	/// \param[in] CameraIndex The index of the camera. The function #setDevicePreset updates the #ProcessingHandle for the first camera (with index 0) automatically. 
	SPECTRALRADAR_API void updateAfterPresetChange(OCTDeviceHandle Dev, ProbeHandle Probe, ProcessingHandle Proc, int CameraIndex);


	/// \fn double analyzeComplexAScan(ComplexDataHandle AScanIn, AScanAnalyzation Selection);
	/// \ingroup Volume
	/// \brief Analyzes the given complex A-scan data, extracts the selected feature, and returns the computed value.
	/// \param[in] AScanIn A valid (non null) complex data handle of the A-scan (#ComplexDataHandle).
	/// \param[in] Selection The desired feature that should be computed (#AScanAnalyzation).
	/// \return The computed feature.
	///
	/// If the given data is multi-dimensional, only the first A-scan will be analyzed.
	SPECTRALRADAR_API double analyzeComplexAScan(ComplexDataHandle AScanIn, AScanAnalyzation Selection);

	/// \enum WaitForCompletion
	/// \ingroup Hardware
	/// \brief Defines the behaviour whether a function should wait for the operation to complete or return immediately.
	typedef enum WaitForCompletion_
	{
		Wait = 0,
		Continue = 1
	} WaitForCompletion;


	// Polarization Adjustment
	/// \defgroup PolarizationAdjustment Polarization Adjustment

	/// \enum PolarizationRetarder
	/// \ingroup Polarization Adjustment
	/// \brief List of available polarization retarders in a polarization control unit.
	typedef enum PolarizationRetarder_
	{
		Retarder_Quarter_Wave = 0,
		Retarder_Half_Wave = 1,
	} PolarizationRetarder;

	/// \ingroup PolarizationAdjustment
	/// \brief Defines the function prototype for the polarization adjustment retardation callback (see also #setPolarizationAdjustmentRetardationChangedCallback()). The argument contains the current (unitless) position (see also #setPolarizationAdjustmentRetardation()) of the specified #PolarizationRetarder.
	typedef void(__stdcall* cbRetardationChanged)(PolarizationRetarder, double);

	/// \fn SPECTRALRADAR_API BOOL isPolarizationAdjustmentAvailable(OCTDeviceHandle Dev);
	/// \ingroup PolarizationAdjustment
	/// \brief Returns whether or not a motorized polarization adjustment stage is available for the specified device.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return true if a polarization adjustment is available
	SPECTRALRADAR_API BOOL isPolarizationAdjustmentAvailable(OCTDeviceHandle Dev);

	/// \fn void setPolarizationAdjustmentRetardationChangedCallback(OCTDeviceHandle Dev, cbRetardationChanged Callback)
	/// \ingroup PolarizationAdjustment
	/// \brief Registers the callback to get notified when the polarization adjustment retardation has changed.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Callback the Callback to register. 
	SPECTRALRADAR_API void setPolarizationAdjustmentRetardationChangedCallback(OCTDeviceHandle Dev, cbRetardationChanged Callback);

	/// \fn double setPolarizationAdjustmentRetardation(OCTDeviceHandle Dev, PolarizationRetarder Retarder, double Retardation, WaitForCompletion Wait)
	/// \ingroup PolarizationAdjustment
	/// \brief Sets the retardation of the specified retarder in the polarization adjustment. The retardation is a unitless value between 0 and 1, which represents the full adjustment range of the retarder. The retarder may take some time to physically reach the new Retardation. Use the Wait parameter to choose if the function should block until the new position is reached.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Retarder the #PolarizationRetarder which shall be adjusted
	/// \param Retardation the new retardation value (0 <= retardation <= 1)
	/// \param Wait specify #WaitForCompletion Wait to block until the new Retardation value has been reached
	SPECTRALRADAR_API void setPolarizationAdjustmentRetardation(OCTDeviceHandle Dev, PolarizationRetarder Retarder, double Retardation, WaitForCompletion Wait);

	/// \fn double getPolarizationAdjustmentRetardation(OCTDeviceHandle Dev, PolarizationRetarder Retarder)
	/// \ingroup PolarizationAdjustment
	/// \brief Gets the current retardation of the specified retarder in the polarization adjustment. If #setPolarizationAdjustmentRetardation was used in a non-blocking fashion, the function returns the current position of the retarder, not the final target position.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Retarder the #PolarizationRetarder which shall be queried
	/// \return The current unitless Retardation of the selected Retarder (0 <= Retardation <= 1)
	SPECTRALRADAR_API double getPolarizationAdjustmentRetardation(OCTDeviceHandle Dev, PolarizationRetarder Retarder);

	// Reference Intensity Control
	/// \defgroup RefIntControl Reference Intensity Control

	/// \ingroup RefIntControl
	/// \brief Defines the function prototype for the reference intensity control status callback (see also #setReferenceIntensityControlCallback()). The argument contains the current (unitless) intensity between 0 and 1 (see also #setReferenceIntensityControlValue()).
	typedef void(__stdcall* cbReferenceIntensityControlValueChanged)(double);

	/// \fn SPECTRALRADAR_API BOOL isReferenceIntensityControlAvailable(OCTDeviceHandle Dev);
	/// \ingroup RefIntControl
	/// \brief Returns whether or not an automated reference intensity control is available for the specified device.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return true if a reference intensity control is available
	SPECTRALRADAR_API BOOL isReferenceIntensityControlAvailable(OCTDeviceHandle Dev);

	/// \fn void setReferenceIntensityControlCallback(OCTDeviceHandle Dev, cbReferenceIntensityControlValueChanged Callback)
	/// \ingroup RefIntControl
	/// \brief Registers the callback to get notified when the reference intensity has changed.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Callback the Callback to register. 
	SPECTRALRADAR_API void setReferenceIntensityControlCallback(OCTDeviceHandle Dev, cbReferenceIntensityControlValueChanged Callback);

	/// \fn void setReferenceIntensityControlValue(OCTDeviceHandle Dev, double ReferenceIntensity, WaitForCompletion Wait)
	/// \ingroup RefIntControl
	/// \brief Sets the reference intensity of the specified device. The intensity is a unitless value between 0 and 1, which represents the full adjustment range of the reference intensity control, but may or may not be linear. The control may take some time to physically reach the new intensity. Use the Wait parameter to choose if the function should block until the new intensity is reached.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param ReferenceIntensity the new reference intensity value (0 <= ReferenceIntensity <= 1)
	/// \param Wait specify #WaitForCompletion Wait to block until the new intensity value has been reached
	SPECTRALRADAR_API void setReferenceIntensityControlValue(OCTDeviceHandle Dev, double ReferenceIntensity, WaitForCompletion Wait);

	/// \fn double getReferenceIntensityControlValue(OCTDeviceHandle Dev)
	/// \ingroup RefIntControl
	/// \brief Gets the current reference intensity of the specified device. If #setReferenceIntensityControlValue was used in a non-blocking fashion, the function returns the current value of the control, not the final target value.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return The current unitless reference intensity of the selected device (0 <= ReferenceIntensity <= 1)
	SPECTRALRADAR_API double getReferenceIntensityControlValue(OCTDeviceHandle Dev);

	// Amplification Control
	/// \defgroup AmplificationControl Amplification Control

	/// \fn SPECTRALRADAR_API BOOL isAmplificationControlAvailable(OCTDeviceHandle Dev);
	/// \ingroup AmplificationControl
	/// \brief Returns whether or not the sampling amplification of specified device can be adjusted.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return true if an amplification control is available
	SPECTRALRADAR_API BOOL isAmplificationControlAvailable(OCTDeviceHandle Dev);

	/// \fn int getAmplificationControlNumberOfSteps(OCTDeviceHandle Dev)
	/// \ingroup AmplificationControl
	/// \brief Gets the number of discrete amplification control steps available on the specified device. Please note that the largest amplification step is #getAmplificationControlNumberOfSteps() - 1.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return The number of amplification steps.
	SPECTRALRADAR_API int getAmplificationControlNumberOfSteps(OCTDeviceHandle Dev);

	/// \fn void setAmplificationControlStep(OCTDeviceHandle Dev, int Step)
	/// \ingroup AmplificationControl
	/// \brief Sets the sampling amplification on the the specified device. The lowest amplification is always 0. In general, the amplification should be set as high as possible without going into saturation.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \param Step Which amplification step to use. 0 <= AmplificationStep < #getAmplificationControlNumberOfSteps()
	SPECTRALRADAR_API void setAmplificationControlStep(OCTDeviceHandle Dev, int Step);

	/// \fn int getAmplificationControlStep(OCTDeviceHandle Dev)
	/// \ingroup AmplificationControl
	/// \brief Gets the current sampling amplification of the specified device.
	/// \param Dev the #OCTDeviceHandle that was initially provided by initDevice.
	/// \return The current amplification step of the selected device (0 <= Step <= #getAmplificationControlNumberOfSteps())
	SPECTRALRADAR_API int getAmplificationControlStep(OCTDeviceHandle Dev);


	// General information
	/// \defgroup GeneralInfo General Information

	/// \fn void getSoftwareVersion(char* Version, int Length);
	/// \ingroup GeneralInfo
	/// \brief Returns the current software version. 
	/// \param[out] Version The current software version returned from the function.
	/// \param[in] Length Length of the user-provided buffer.
	SPECTRALRADAR_API void getSoftwareVersion(char* Version, int Length);



	/// \fn void useProbeCalibration(bool OnOff);
	/// \ingroup Probe
	/// \brief Enable or disable use of probe calibration. Needs to be called before #initProbe
	/// \warning Experimental
	/// \param[in] OnOff True to enable probe calibration
	SPECTRALRADAR_API void useProbeCalibration(bool OnOff);


	/// \fn void extractLine(DataHandle Data, DataHandle Res, double P1_1_mm, double P1_2_mm, double P2_1_mm, double P2_2_mm)
	/// \ingroup Data
	/// \brief Extract 1D data from 2D along a specified line
	/// \warning Experimental
	/// \param[in] Data 2D source data
	/// \param[out] Res 1D result data
	/// \param[in] P1_1_mm Start of line on axis 1 in mm
	/// \param[in] P1_2_mm Start of line on axis 2 in mm
	/// \param[in] P2_1_mm End of line on axis 1 in mm
	/// \param[in] P2_2_mm End of line on axis 2 in mm
	SPECTRALRADAR_API void extractLine(DataHandle Data, DataHandle Res, double P1_1_mm, double P1_2_mm, double P2_1_mm, double P2_2_mm);

	/// \fn int extractLocalMaxima(DataHandle Data1D, int N, double* dataPos_mm, double* dataHeight)
	/// \ingroup Data
	/// \brief Extract multiple local maxima from 1D data
	/// \warning Experimental
	/// \param[in] Data1D Source data
	/// \param[in] N Maximum number of maxima to find
	/// \param[out] dataPos_mm Initialized array of double values to be filled with the locations of the maxima
	/// \param[out] dataHeight Initialized array of double values to be filled with the values of the maxima
	/// \return Actual number of maxima found
	SPECTRALRADAR_API int extractLocalMaxima(DataHandle Data1D, int N, double* dataPos_mm, double* dataHeight);

	/// \fn int extractLocalMaximaEx(DataHandle Data1D, int N, double minDist, double* dataPos_mm, double* dataHeight)
	/// \ingroup Data
	/// \brief Extract multiple local maxima from 1D data while mainting a minimum distance between peaks
	/// \warning Experimental
	/// \param[in] Data1D Source data
	/// \param[in] N Maximum number of maxima to find
	/// \param[in] minDist Minimum distance between each maxima
	/// \param[out] dataPos_mm Initialized array of double values to be filled with the locations of the maxima
	/// \param[out] dataHeight Initialized array of double values to be filled with the values of the maxima
	/// \return Actual number of maxima found
	SPECTRALRADAR_API int extractLocalMaximaEx(DataHandle Data1D, int N, double minDist, double* dataPos_mm, double* dataHeight);

	/// \fn void readData(DataHandle Data, const char* filename, int SizeZ, int SizeX, int SizeY)
	/// \ingroup Data
	/// \brief Read data object from raw data stream in file
	/// \warning Experimental
	/// \param[out] Data Resulting data
	/// \param[in] filename File to read
	/// \param[in] SizeZ SizeZ of dataset
	/// \param[in] SizeX SizeX of dataset
	/// \param[in] SizeY SizeY of dataset
	SPECTRALRADAR_API void readData(DataHandle Data, const char* filename, int SizeZ, int SizeX, int SizeY);

	// experimental API:

	typedef enum SpectrumDirectionType_ {
		LongToShortWavelengths, // this should be default?? Check!
		ShortToLongWavelengths,
		UnknownSpectrumDirection
	} SpectrumDirectionType;




#ifdef __cplusplus 
}
#endif

#endif // _SPECTRALRADAR_H
