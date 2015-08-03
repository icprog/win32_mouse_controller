#pragma once
#include "SerialPort.h"
#include "Modbus.h"
#include "MouseMover.h"

/// Callback function type definition
typedef bool (*gui_callback)(const std::string& data);

class Controller
{
public:

	Controller(int deviceAddress, Modbus* modbusInstance);
	~Controller();

	/**
	* @brief Sets pointer to SerialPort object which will be used to communicate
	* @param sp - pointer to SerialPort object.
	* @return True if all correct. False, if pointed SerialPort isn't opened.
	*/
	bool setSerialPort(SerialPort*  sp);

	/**
	* @brief Starts Controller thread
	* @return True if start succeeded
	*/
	bool start();

	/**
	* @brief Ends Controller thread
	* @return True if termination succeeded
	*/
	bool terminate();

	/**
	* @brief Sets GUI callback
	* @details Callback function is called to update GUI values
	* @param Pointer to callback.
	* @return None.
	*/
	void setGuiCallback(gui_callback cb);

	/**
	* @brief Sets console trace callback
	* @details Callback function is called to update GUI values
	* @param Pointer to callback.
	* @return None.
	*/
	void setTraceCallback(gui_callback cb);

	/**
	* @brief Sets parameters of MouseMover. To be called from GUI
	* @param factor Mouse sensivity factor
	* @param samplingTime Mouse cursor update period
	* @return None.
	*/
	void setMoverParams(int factor = -1, int samplingTime = -1);

	/**
	* @brief Sets current measure as zero offset
	* @param None
	* @return None.
	*/
	void setZero();

	/**
	* @brief Enables or disables data saving.
	* @details Data is saved to "data.dat" file
	* @param enable - true to enable, false to disable.
	* @return None.
	*/
	void enableDataStorage(bool enable);

	/**
	* @brief Returns data storage state.
	* @param None
	* @return State of data storage.
	*/
	bool isStorageEnabled();

	/**
	* @brief Controller Thread function.
	* @details Contains thread's loop.
	* @param None.
	* @return None.
	*/
	static unsigned __stdcall ThreadFn(void* pvParam);
	
private:
	SerialPort* m_cSerialPort;		/*< Pointer to SerialPort object used to communicate with device */
	Modbus*		m_cModbus;			/*< Modbus object to wrap serial port communication */
	MouseMover  m_cMouseMover;		/*< MouseMover object */
	int			m_iDeviceAddress;	/*< Modbus address of device */

	HANDLE		m_hThread;				/*< Thread handle */
	HANDLE		m_hThreadTerminator;	/*< Thread terminator event handle */
	HANDLE		m_hThreadStart;			/*< Thread start event handle */

	CRITICAL_SECTION	m_hLock;		/*< Critical section lock*/

	gui_callback m_pfGuiCallback;		/*< Pointer to GUI callback function */
	gui_callback m_pfTraceCallback;		/*< Pointer to GUI console trace callback */

	bool		m_bStoreData;			/*< Flag indicating received data is stored to file*/
	
	float		m_pfAngles[3];			/*< Array of Euler angles */
	float		m_pfZeroOffset[3];		/*< Array of zero offset*/
	/**
	* @brief Does all data processing
	* @details Computes desired values from input data. 
	* @param[in] acc - acceleration vector
	* @param[in] angRate - angular rate vector
	* @param[in] magnField - magnetic field vector
	* @param[out] result - result of computation
	* @return Not implemented.
	*/
	bool processData(float* boardData, float* result);

	/**
	* @brief Formats float array to string with desired precision
	* @details Wraps some lines of code
	* @param[in] data - data vector
	* @param[in] dataSize - vector size
	* @param[in] precision - float precision
	* @return Formatted string
	*/
	static std::string formatString(float* data, size_t dataSize, size_t precision);
	
};

