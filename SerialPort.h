#pragma once

/**	Serial Port class. A wrapper of WinAPI serial port access functions.	
 *
 */
class SerialPort
{
public:
	SerialPort(); /**< Constructor */
	~SerialPort();/**< Destructor */

	/**
	* @brief Opens a serial port of given name with given parameters.
	* @param dcb port parameters structure.
	* @param name port name.
	* @return Open attempt result
	*/
	BOOL open(DCB dcb, const char* name = "COM1"); 

	/**
	* @brief Closes port
	* @param None.
	* @return Result of operation. 
	*/
	BOOL close();

	/**
	* @brief Reads data from port
	* @param data a pointer to where put read data.
	* @param sizeBuffer size of packet to read
	* @param length
	* @return Result of operation.
	*/
	BOOL read(char* data, const unsigned int& sizeBuffer, unsigned long& length);

	/**
	* @brief Writes data to port
	* @param data a pointer to data to send.
	* @param sizeBuffer size of packet to send
	* @param length
	* @return Result of operation.
	*/
	BOOL write(LPCVOID data, const unsigned int& sizeBuffer, unsigned long& length);

	/**
	* @brief Gets port handle
	* @param None.
	* @return Port handle.
	*/
	HANDLE getPortHandle();

	/**
	* @brief Gets port status
	* @param None.
	* @return Port status.
	*/
	BOOL getPortStatus();


private:
	HANDLE m_hSerialPortHandle; /**< Port handle */
	BOOL   m_bSerialPortStatus; /**< Port Status */
	DCB    m_SerialPortDCB;		/**< Port settings */

};

