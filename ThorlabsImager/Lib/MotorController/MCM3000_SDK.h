/*! \file 
*/

//MCM3000_SDK.h

#ifdef __cplusplus
extern "C" 
{
#endif

/*! \enum Params Device parameter settings*/
	enum Params
	{
		PARAM_FIRST_PARAM,
		PARAM_DEVICE_TYPE=0,
		
		PARAM_X_POS = 200,///<Set Location for X
		PARAM_X_HOME = 201,///<Home X 
		PARAM_X_ZERO = 202,///<Set the current location to zero for X. The device will read zero for postion after this parameter is set
		PARAM_X_POS_CURRENT = 207, ///<Get Current Location for X
		PARAM_X_STOP = 209, ///<Stop X Stage
		PARAM_X_VELOCITY_CURRENT = 210, ///<Get Current Velocity for X

		PARAM_Y_POS = 300,///<Set Location for Y
		PARAM_Y_HOME = 301,///<Home Y
		PARAM_Y_ZERO = 302,///<Set the current location to zero for Y. The device will read zero for postion after this parameter is set
		PARAM_Y_POS_CURRENT = 307,///<Get Current Location for Y
		PARAM_Y_STOP = 309,	///<Stop Y Stage
		PARAM_Y_VELOCITY_CURRENT = 310,///<Get Current Velocity for Y

		PARAM_Z_POS = 400,///<Set Location for Z
		PARAM_Z_HOME = 401,///<Home Z
		PARAM_Z_ZERO = 402,///<Set the current location to zero for Z. The device will read zero for postion after this parameter is set
		PARAM_Z_POS_CURRENT = 407,///<Get Current Location for Z
		PARAM_Z_STOP = 409,	///<Stop Z Stage
		PARAM_Z_VELOCITY_CURRENT = 410,///<Get Current Velocity for Z

		PARAM_LAST_PARAM
	};

/*! \enum ParamType data of the parameter*/
	enum ParamType
	{
		TYPE_LONG,///<Parameter is of type integer
		TYPE_DOUBLE///<Parameter is of type double precision floating point
	};

/*! \enum DeviceType Type of the device returned by PARAM_DEVICE_TYPE*/
	enum DeviceType
	{
		DEVICE_TYPE_FIRST,
		STAGE_X				= 0x00000004, ///<Stage that moves in the X direction
		STAGE_Y				= 0x00000008, ///<Stage that moves in the Y direction
		STAGE_Z				= 0x00000010, ///<Stage that moves in the Z direction
		DEVICE_TYPE_LAST
	};

/*! \enum StatusType return value for the StatusPosition status variable*/
	enum StatusType
	{
		STATUS_BUSY=0,///<Device is busy executing a command(s)
		STATUS_READY,///<Device is idle or has completed executing a command(s)
		STATUS_ERROR///<Error occuring during the capture. Error flag will persist until a new capture is setup.
	};


/*! \fn long FindDevices(long &DeviceCount)
    \brief Locates connected devices.
    \param DeviceCount The number of devices currently available
	\return If the function succeeds the return value is TRUE. 	
*/
long FindDevices(long &DeviceCount);


/*! \fn long SelectDevice(const long device)
    \brief Selects an available device
    \param device zero based index of a device
	\return If the function succeeds the return value is TRUE. All parameters and actions are directed the selected device. 
*/	  
long SelectDevice(const long Device);


/*! \fn long TeardownDevice
    \brief Release resources associated with the device before shutting down the application
	\return If the function succeeds the return value is TRUE and the device shutdown normally.
*/
long TeardownDevice();

/*! \fn long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault)
    \brief  Retrieve parameter information and capabilities
    \param paramID Unique identifier for the parameter to interrogate
	\param paramType The ParamType for the paramID
	\param paramAvailable Returns TRUE if the paramID is supported
	\param paramReadOnly Returns TRUE if the paramID can only be read
	\param paramMin Minimum value for the paramID
	\param paramMax Maximum value for the param ID
	\param paramDefault Recommended default for the paramID
	\return If the function succeeds the return value is TRUE.
*/
long GetParamInfo(const long paramID, long &paramType, long &paramAvailable, long &paramReadOnly, double &paramMin, double &paramMax, double &paramDefault);

/*! \fn long SetParam(const long paramID, const double param)
    \brief Sets a parameter value
	\param paramID Unique ID of the parameter to set.
    \param param Input value. If the Paramtype is long the data must be cast to type double.
	\return If the function succeeds the return value is TRUE.
	\example PositionTest.cpp
*/
long SetParam(const long paramID, const double param);

/*! \fn long GetParam(const long paramID, double &param)
    \brief Gets a parameter value
	\param paramID Unique ID of the parameter to retrieve
    \param param Current value of the parameter
	\return If the function succeeds the return value is TRUE.
	\example PositionTest.cpp
*/
long GetParam(const long paramID, double &param);


/*! \fn long PreflightPosition()
    \brief Using the current parameters arm the device for commands
  	\return If the function succeeds the return value is TRUE.
*/
long PreflightPosition();///<This submits the current parameters to the Device.

/*! \fn long SetupPosition()
    \brief Using the current parameters the device is armed for the next command
 	\return If the function succeeds the return value is TRUE.
*/
long SetupPosition();///<This submits the parameters that can chage while the Device is active. 


/*! \fn long StartPosition()
    \brief If not already executed, execute a command.
	\return If the function succeeds the return value is TRUE.
*/
long StartPosition();///<Begin the command.

/*! \fn long StatusPosition(long &status)
    \brief Determine the status of the device.
    \param status, status of the device. See StatusType.
	\return If the function succeeds the return value is TRUE.
*/
long StatusPosition(long &status);///<returns the status of the device.

/*! \fn long PostflightPosition()
    \brief Releases any resources after the commands have been executed	
	\return If the function succeeds the return value is TRUE.	
*/
long PostflightPosition();


/*! \fn long GetLastErrorMsg(wchar_t * msg, long size)
    \brief Retrieve the last error message
    \param msg user allocated buffer for storing the error message
	\param size of the buffer (in characters) of the buffer provided
	\return If the function succeeds the return value is TRUE.	
*/
long GetLastErrorMsg(wchar_t *msg, long size);


#ifdef __cplusplus
}
#endif
