#include "stdafx.h"
#include "ThorlabsImager.h"
#include "MCM3000_SDK.h"
#include <iostream>
#include <windows.h>
#include <math.h>
using namespace std;

static double zPosition;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Initialize Stage
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void yOCTStageInit()
{
	zPosition = 0;

	long deviceCount = 0;

	if (false == FindDevices(deviceCount))
	{
		cerr << "An error occurred: No Devices Found" << endl;
		return;
	}

	//Use device 0 for BScope
	if (false == SelectDevice(0))
	{
		cerr << "An error occurred: See i1" << endl;
		return;
	}

	// Beginning of Original PositionSetZero
	long setZero = 1;

	if (0 == SetParam(PARAM_X_ZERO, static_cast<double>(setZero)))
	{
		cerr << "An error occurred: See i2" << endl;
		return;
	}


	if (0 == PreflightPosition())
	{
		cerr << "An error occurred: See i3" << endl;
		return;
	}

	if (0 == SetupPosition())
	{
		cerr << "An error occurred: See i4" << endl;
		return;
	}

	if (0 == StartPosition())
	{
		cerr << "An error occurred: See i5" << endl;
		return;
	}

	long status = STATUS_READY;
	do
	{
		if (0 == StatusPosition(status))
		{
			cerr << "An error occurred: See i6" << endl;
			return;
		}
	} while (STATUS_BUSY == status);

	if (0 == PostflightPosition())
	{
		cerr << "An error occurred: See i7" << endl;
		return;
	}

	// Beginning of Actual MoveX Part
	long currentPos = 0;
	long paramType;
	long paramAvailable;
	long paramReadOnly;
	double paramMin;
	double paramMax;
	double paramDefault;

	GetParamInfo(PARAM_X_POS, paramType, paramAvailable, paramReadOnly, paramMin, paramMax, paramDefault);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//GET / SET POSITION
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//Get / Set Stage Position [mm]
void yOCTStageSetZPosition(const double newZ)
{
	double dz = abs(zPosition - newZ);
	
	if (0 == SetParam(PARAM_X_POS, newZ))
	{
		cerr << "An error occurred: See sz1" << endl;
		return;
	}

	if (0 == PreflightPosition())
	{
		cerr << "An error occurred: See sz2" << endl;
		return;
	}

	if (0 == SetupPosition())
	{
		cerr << "An error occurred: See sz3" << endl;
		return;
	}

	if (0 == StartPosition())
	{
		cerr << "An error occurred: See sz4" << endl;
		return;
	}

	long status = STATUS_READY;
	do
	{
		if (0 == StatusPosition(status))
		{
			cerr << "An error occurred: See sz5" << endl;
			return;
		}
	} while (STATUS_BUSY == status);

	if (0 == PostflightPosition())
	{
		cerr << "An error occurred: See sz6" << endl;
		return;
	}

	double readPos;
	if (0 == GetParam(PARAM_X_POS_CURRENT, readPos))
	{
		cerr << "An error occurred: See sz7" << endl;
		return;
	}

	//Wait for movement completion (for motor to move)
	Sleep((long)(10.0 * (dz*1000.0))); // empirically determined, wait time is 10 msec for every 1um of displacement

	//Update z position
	zPosition = newZ;
}
