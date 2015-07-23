#pragma once
#include "SerialPort.h"
#include "Modbus.h"

/// Callback function type definition
typedef BOOL(_stdcall *callback)(const std::string& data);

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
	* @brief Controller Thread function.
	* @details Contains thread's loop.
	* @param None.
	* @return None.
	*/
	static unsigned __stdcall ThreadFn(void* pvParam);
	/*TODO
		Napisz funkcjê w¹tku Controller.
		Otrzymawszy dane, powinna wywo³ywaæ Callback
		Callback ma byæ zaimplementowany w main'ie i powinien uaktualniaæ GUI
		*/
	
private:
	SerialPort* m_cSerialPort;		/*< Pointer to SerialPort object used to communicate with device */
	Modbus*		m_cModbus;			/*< Modbus object to wrap serial port communication */
	int			m_iDeviceAddress;	/*< Modbus address of device */

	HANDLE		m_hThread;				/*< Thread handle */
	HANDLE		m_hThreadTerminator;	/*< Thread terminator event handle */
	HANDLE		m_hThreadStart;			/*< Thread start event handle */

	/**
	* @brief Does all data processing
	* @details Computes desired values from input data. 
	* @param[in] acc - acceleration vector
	* @param[in] angRate - angular rate vector
	* @param[in] magnField - magnetic field vector
	* @param[out] result - result of computation
	* @return Not implemented.
	*/
	bool processData(float* acc, float* angRate, float* magnField, float* result);
	
};

