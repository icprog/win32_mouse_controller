#pragma once
#include "stdafx.h"
#include <cstdlib>
#include <string>
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
	BOOL open(DCB dcb, TCHAR* name);

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
	* @param timeout time to wait for data. In MODBUS, end of packet is signaled with silence on line
	* @return Result of operation.
	*/
	BOOL read(std::string&  data, unsigned int& size, unsigned long timeout);

	/**
	* @brief Writes data to port
	* @param data a pointer to data to send.
	* @param sizeBuffer size of packet to send
	* @param length
	* @return Result of operation.
	*/
	BOOL write(const unsigned char* data, DWORD dwSize);

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


	inline void acquireLock()	{ EnterCriticalSection(&m_csCriticalSection); }
	inline void unlock()		{ LeaveCriticalSection(&m_csCriticalSection); }
	inline void initLock()		{ InitializeCriticalSection((&m_csCriticalSection)); }
	inline void deleteLock()	{ DeleteCriticalSection((&m_csCriticalSection)); }

	static unsigned __stdcall ThreadFn(void* pvParam);

private:
	HANDLE m_hSerialPortHandle; /**< Port handle */
	BOOL   m_bSerialPortStatus; /**< Port Status */
	DCB    m_SerialPortDCB;		/**< Port settings */

	HANDLE m_hThread;			/**< Serial Port worker thread handle */
	HANDLE m_hThreadTerminator; /**< Thread terminator event handle */
	HANDLE m_hThreadStarted;	/**< Thread started event handle */
	HANDLE m_hEventRx;			/**< Data received event handle */

	std::string m_szFrameBuffer;


	CRITICAL_SECTION m_csCriticalSection;
};

