#pragma once
#include "SerialPort.h"
#include <vector>
#include <algorithm>



/**	@brief Modbus class. Used to perform MODBUS-like communication via serial port.
*
*/
class Modbus
{
public:
	enum modbus_commands_enum{
		COMMAND_READ_32BIT = 66,
		COMMAND_READ_8BIT
	};
	/**
	* @brief Class constructor */
	Modbus();

	/**
	* @brief Class destructor */
	~Modbus();
	
	/**
	* @brief Sets pointer to SerialPort object which will be used to communicate
	* @param sp - pointer to SerialPort object.
	* @return True if all correct. False, if pointed SerialPort isn't opened.
	*/
	bool setSerialPort(SerialPort* sp);

	/**
	* @brief Adds a MODBUS device to device list
	* @param iDeviceAddress - MODBUS address of device
	* @return True if all correct. False, if device is already on list
	*/
	bool addDevice(int iDeviceAddress);

	/**
	* @brief Removes MODBUS device from device list
	* @param iDeviceAddress - MODBUS address of device
	* @return True if all correct. False, if device wasn't on list
	*/
	bool removeDevice(int iDeviceAddress);

	/**
	* @brief Sends MODBUS command and waits for response. This is blocking function!
	* @details It blocks current thread for given timeout waiting for MODBUS device response.
	* @param iDeviceAddress - MODBUS address of device
	* @param iCommandCode - MODBUS command code
	* @arguments - arguments of command.
	* @iArgCount - number of arguments(number of argument bytes)
	* @response - device response
	* @response_size - device response size
	* @return True if all correct. False, well, I don't know when.
	*/
	bool sendCommand(int iDeviceAddress, int iCommandCode, unsigned char* arguments, int iArgCount, std::string &response, unsigned int &response_size);
	
private:
	std::vector<int> m_vectDeviceList;	/*< Device list*/
	SerialPort* m_pSerialPort;			/*<Pointer to serial port object*/
};

