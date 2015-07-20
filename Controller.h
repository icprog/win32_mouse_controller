#pragma once
#include "SerialPort.h"
#include "Modbus.h"

/// Callback function type definition
typedef BOOL(_stdcall *callback)(const std::string& data);

class Controller
{
public:

	Controller();
	~Controller();

	/**
	* @brief Sets pointer to SerialPort object which will be used to communicate
	* @param sp - pointer to SerialPort object.
	* @return True if all correct. False, if pointed SerialPort isn't opened.
	*/
	bool setSerialPort(SerialPort*  sp);

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
	SerialPort* m_cSerialPort;	/*< Pointer to SerialPort object used to communicate with device*/
	Modbus*		m_cModbus;		/*< Modbus object*/


};

