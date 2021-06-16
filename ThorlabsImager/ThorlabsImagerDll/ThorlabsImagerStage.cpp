#include "stdafx.h"
#include "ThorlabsImager.h"
//#include "MCM3000_SDK.h"
#include "Thorlabs.MotionControl.KCube.DCServo.h"
#include <iostream>
#include <windows.h>
#include <math.h>
using namespace std;

long getSNByAxes(char axes) //Run Thorlabs.MotionControl.Kinesis.exe to see which serial numbers are avilable
{
	switch (axes)
	{
	case 'x':
	case 'X':
		return 27254221;
	case 'y':
	case 'Y':
		return 27254232;
	case 'z':
	case 'Z':
		return 27254238;
	default: // code to be executed if n doesn't match any cases
		return -1;
	}
}

// Return how much is one mm in device units
double mmToDeviceUnits(long sn) //Stage SN
{
	// To get those numbers we move can move 100000 device points, and read what was the movement in mm
	// or use Kinesis software to move known mm and see how much of units it came to be
	// A more accurate design is by using Stage Calibration scripts in 00 Calibration and Analysis Scripts (OCT-Hist)

	switch (sn)
	{
	case 27254221:
		return 34609.935;
	case 27254232:
		return 34862.567;
	default:
		return 100000 / 2.9151;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Initialize Stage
///////////////////////////////////////////////////////////////////////////////////////////////////////////
double yOCTStageInit(char axes) // returns current position in mm
{

	if (TLI_BuildDeviceList() == 0)
	{
		long serialNo = getSNByAxes(axes);

		char serialNoText[16];
		sprintf_s(serialNoText, "%d", serialNo);

		// operate device
		if (CC_Open(serialNoText) == 0)
		{
			// start the device polling at 200ms intervals
			CC_StartPolling(serialNoText, 200);

			Sleep(3000);
			CC_ClearMessageQueue(serialNoText);

			// get actual poaition
			double pos = CC_GetPosition(serialNoText);
			pos = pos / mmToDeviceUnits(serialNo); // Convert pos to mm

			// set velocity & acc if desired
			int velocity = 1000000; //Units unknown. 0 means no limit. max vilocity 
			int acc = 40; //Units unknown (default is 400)
			int currentMaxVelocity, currentAcceleration;
			CC_GetVelParams(serialNoText, &currentAcceleration, &currentMaxVelocity);
			printf("Device: %s current Acc: %d, max Vel: %d\n", serialNoText, currentAcceleration, currentMaxVelocity);
			CC_SetVelParams(serialNoText, acc, velocity);

			return pos;
		}

		return -1;
	}

	return -1;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// SET POSITION
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Set Stage Position [mm]
void yOCTStageSetPosition(char axes, double position)
{

	// Get device
	int serialNo = getSNByAxes(axes);
	char serialNoText[16];
	sprintf_s(serialNoText, "%d", serialNo);

	// Convert mm to device units 
	position = position * mmToDeviceUnits(serialNo);

	// move to position (channel 1)
	CC_ClearMessageQueue(serialNoText);
	CC_MoveToPosition(serialNoText, (int)position);
	printf("Device %s moving\r\n", serialNoText);

	WORD messageType;
	WORD messageId;
	DWORD messageData;
	// wait for completion
	do
	{
		//double pos = CC_GetPosition(testSerialNo);
		//printf("Position %f\n", pos);
		//Sleep(100);
		CC_WaitForMessage(serialNoText, &messageType, &messageId, &messageData);
	} while (messageType != 2 || messageId != 1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Close Stage
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Close stage after use
void yOCTStageClose(char axes)
{

	int serialNo = getSNByAxes(axes);

	char testSerialNo[16];
	sprintf_s(testSerialNo, "%d", serialNo);

	// stop polling
	CC_StopPolling(testSerialNo);
	// close device
	CC_Close(testSerialNo);
}
