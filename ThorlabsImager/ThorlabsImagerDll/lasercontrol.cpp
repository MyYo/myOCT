
#include "stdafx.h"
#include "lasercontrol.h"

#define _SCL_SECURE_NO_WARNINGS  
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <ctime>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cassert>
#include <chrono>	
#include <vector>

using namespace std;


#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <conio.h>
#include <windows.h>
#include <SpectralRadar.h>
#include <direct.h>

#include "TL4000.h"
#include "visa.h"


/*===========================================================================
 classes
===========================================================================*/
struct ResourceString_t
{
	ViChar str[VI_FIND_BUFLEN];
};

/*===========================================================================
 Prototypes
===========================================================================*/
//void error_exit(ViSession handle, ViStatus err);
//void waitKeypress();
//ViStatus find_instruments(ViString findPattern, ViChar** resource);
//ViStatus get_device_id(ViSession handle);

/*===========================================================================
 Constants
===========================================================================*/
const int COMM_TIMEOUT = 5000;    // communications timeout in ms


/*===========================================================================
 Functions
===========================================================================*/
int controllaser(ViBoolean mode)
{
	ViStatus    err;
	ViChar* rscPtr;
	ViSession   instrHdl = VI_NULL;

	//cout << "-----------------------------------------------------------\n";
	//cout << " Thorlabs TL4000 Series instrument driver sample \n";
	//cout << "-----------------------------------------------------------\n\n";

	// Parameter checking / resource scanning
	err = find_instruments(TL4000_FIND_PATTERN_ANY, &rscPtr);
	if (err) error_exit(instrHdl, err);  // something went wrong
	if (!rscPtr) exit(EXIT_SUCCESS);     // none found

	// Open session to instrument
	//cout << "Opening session to '" << rscPtr << "' ...\n\n";
	err = TL4000_init(rscPtr, VI_ON, VI_ON, &instrHdl);
	if (err) error_exit(instrHdl, err);  // can not open session to instrument

	// Get instrument info
	err = get_device_id(instrHdl);
	if (err) error_exit(instrHdl, err);

	// Turn Laser On
	ViReal64 curr = 64.;
	TL4000_switchTecOutput(instrHdl, mode);
	TL4000_switchLdOutput(instrHdl, mode);
	TL4000_setLdCurrSetpoint(instrHdl, curr);
	// Close session to instrument
	TL4000_close(instrHdl);

	return (EXIT_SUCCESS);
}


/*---------------------------------------------------------------------------
 Read out device ID and print it to screen
---------------------------------------------------------------------------*/
ViStatus get_device_id(ViSession instrHdl)
{
	ViStatus err;
	ViChar   nameBuf[TL4000_BUFFER_SIZE];
	ViChar   snBuf[TL4000_BUFFER_SIZE];
	ViChar   fwRevBuf[TL4000_BUFFER_SIZE];
	ViChar   drvRevBuf[TL4000_BUFFER_SIZE];
	ViChar   calMsgBuf[TL4000_BUFFER_SIZE];

	err = TL4000_identificationQuery(instrHdl, VI_NULL, nameBuf, snBuf, fwRevBuf);
	if (err) return(err);
	cout << "Instrument:    " << nameBuf << "\n";
	cout << "Serial number: " << snBuf << "\n";
	cout << "Firmware:      " << fwRevBuf << "\n";

	err = TL4000_calibrationMessage(instrHdl, calMsgBuf);
	if (err) return(err);
	cout << "Calibration:   " << calMsgBuf << "\n";

	err = TL4000_revisionQuery(instrHdl, drvRevBuf, VI_NULL);
	if (err) return(err);
	cout << "Driver:        " << drvRevBuf << "\n\n";

	return(VI_SUCCESS);
}


/*---------------------------------------------------------------------------
  Find Instruments
---------------------------------------------------------------------------*/
ViStatus find_instruments(ViString findPattern, ViChar** resource)
{
	ViStatus       err;
	ViSession      resMgr, instr;
	ViFindList     findList;
	ViUInt32       findCnt;
	static ViChar  rscStr[VI_FIND_BUFLEN];

	cout << "Scanning for instruments ...\n";

	if ((err = viOpenDefaultRM(&resMgr))) return(err);
	switch ((err = viFindRsrc(resMgr, findPattern, &findList, &findCnt, rscStr)))
	{
	case VI_SUCCESS:
		break;

	case VI_ERROR_RSRC_NFOUND:
		viClose(resMgr);
		cout << "No matching instruments found\n\n";
		return (err);

	default:
		viClose(resMgr);
		return (err);
	}

	if (findCnt < 2)
	{
		// Found only one matching instrument - return this
		*resource = rscStr;
		viClose(findList);
		viClose(resMgr);
		return (VI_SUCCESS);
	}

	// Found multiple instruments - get resource strings of all instruments into buffer
	vector <ResourceString_t> rscBuf(findCnt);

	strncpy_s(rscBuf[0].str, VI_FIND_BUFLEN, rscStr, VI_FIND_BUFLEN); // copy first found instrument resource string
	for (vector <ResourceString_t>::iterator i = rscBuf.begin() + 1; i < rscBuf.end(); i++)
	{
		if ((err = viFindNext(findList, (*i).str))) return(err);
	}
	viClose(findList);

	// Display selection
	do
	{
		cout << "Found " << findCnt << " matching instruments:\n\n";

		ViChar name[256], sernr[256];
		for (ViUInt32 cnt = 0; cnt < findCnt; cnt++)
		{
			// Open resource
			if ((err = viOpen(resMgr, rscBuf[cnt].str, VI_NULL, COMM_TIMEOUT, &instr)) != 0)
			{
				viClose(resMgr);
				return(err);
			}
			// Get attribute data
			viGetAttribute(instr, VI_ATTR_MODEL_NAME, name);
			viGetAttribute(instr, VI_ATTR_USB_SERIAL_NUM, sernr);
			// Closing
			viClose(instr);

			// Print out
			cout.width(2); cout << right << cnt + 1 << ": ";
			cout << left << name << " \tS/N:" << sernr << "\n";
		}

		ViUInt32 i;
		cout << "\nPlease select: ";
		cin >> i;
		cout << endl;
		if ((i < 1) || (i > findCnt))
		{
			cout << "Invalid selection\n" << endl;
		}
		else
		{
			// Copy resource string to static buffer
			strncpy_s(rscStr, VI_FIND_BUFLEN, rscBuf[i - 1].str, VI_FIND_BUFLEN);
			*resource = rscStr;

			// Cleanup
			viClose(resMgr);
			return (VI_SUCCESS);
		}
	} while (true);

	return (VI_SUCCESS);
}


/*---------------------------------------------------------------------------
  Exit with error message
---------------------------------------------------------------------------*/
void error_exit(ViSession instrHdl, ViStatus err)
{
	ViChar   buf[TL4000_ERR_DESCR_BUFFER_SIZE];

	// Get error description and print out error
	TL4000_errorMessage(instrHdl, err, buf);
	cerr << "ERROR: " << buf << endl;

	// close session to instrument if open
	if (instrHdl != VI_NULL)
	{
		TL4000_close(instrHdl);
	}

	// exit program
	waitKeypress();
	exit(EXIT_FAILURE);
}


/*---------------------------------------------------------------------------
  Print keypress message and wait
---------------------------------------------------------------------------*/
void waitKeypress(void)
{
	cout << "Press <ENTER> to exit" << endl;
	cin.sync();
	cin.get();
}
