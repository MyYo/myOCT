#include "stdafx.h"
#include "ThorlabsImager.h"
//#include "MCM3000_SDK.h"
#include "Thorlabs.MotionControl.KCube.DCServo.h"
#include <iostream>
#include <windows.h>
#include <math.h>
using namespace std;

int getSNByAxes(char axes) //Run Thorlabs.MotionControl.Kinesis.exe to see which serial numbers are avilable
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Initialize Stage
///////////////////////////////////////////////////////////////////////////////////////////////////////////
double yOCTStageInit(char axes)
{

	double unitconversion = 2.9151 / 100000;

	if (TLI_BuildDeviceList() == 0)
	{
		int serialNo = getSNByAxes(axes);

		char testSerialNo[16];
		sprintf_s(testSerialNo, "%d", serialNo);

		// operate device
		if (CC_Open(testSerialNo) == 0)
		{
			// start the device polling at 200ms intervals
			CC_StartPolling(testSerialNo, 200);

			Sleep(3000);
			CC_ClearMessageQueue(testSerialNo);

			// get actual poaition
			double pos = CC_GetPosition(testSerialNo);
			pos = pos * unitconversion;
			
			// set velocity & acc if desired
			int velocity = 1000000; //Units unknown. 0 means no limit. max vilocity 
			int acc = 400; //Units unknown
			int currentMaxVelocity, currentAcceleration;
			CC_GetVelParams(testSerialNo, &currentAcceleration, &currentMaxVelocity);
			printf("Device: %s current Acc: %d, max Vel: %d\n", testSerialNo, currentAcceleration, currentMaxVelocity);
			CC_SetVelParams(testSerialNo, acc, velocity);

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

	double unitconversion = 2.9151 / 100000; //Conversion between internal and external units, determined ampiricly
	position = position / unitconversion;

	int serialNo = getSNByAxes(axes);

	char testSerialNo[16];
	sprintf_s(testSerialNo, "%d", serialNo);


	// move to position (channel 1)
	CC_ClearMessageQueue(testSerialNo);
	CC_MoveToPosition(testSerialNo, (int)position);
	printf("Device %s moving\r\n", testSerialNo);

	WORD messageType;
	WORD messageId;
	DWORD messageData;
	// wait for completion
	do 
	{
		//double pos = CC_GetPosition(testSerialNo);
		//printf("Position %f\n", pos);
		//Sleep(100);
		CC_WaitForMessage(testSerialNo, &messageType, &messageId, &messageData);
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
