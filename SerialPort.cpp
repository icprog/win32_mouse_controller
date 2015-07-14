#include "stdafx.h"
#include "afxwin.h"
#include "SerialPort.h"


SerialPort::SerialPort() :	m_bSerialPortStatus(FALSE),
							m_hSerialPortHandle(NULL)
{
	m_hEventRx			= INVALID_HANDLE_VALUE;
	m_hThread			= INVALID_HANDLE_VALUE;
	m_hThreadStarted	= INVALID_HANDLE_VALUE;
	m_hThreadTerminator = INVALID_HANDLE_VALUE;
	m_hSerialPortHandle = INVALID_HANDLE_VALUE;

	m_SerialPortDCB.ByteSize = 8;
	m_SerialPortDCB.BaudRate = CBR_115200;
	m_SerialPortDCB.Parity	 = NOPARITY;
	m_SerialPortDCB.StopBits = ONESTOPBIT;

	initLock();
}


SerialPort::~SerialPort()
{
	m_hSerialPortHandle = NULL;
	deleteLock();
}

BOOL SerialPort::open(DCB dcb, char* name = "COM1")
{
	if (m_bSerialPortStatus == FALSE)		/**< Check if port closed. Dont try to open, if port already opened. */
	{
		/// Try to open device
		m_hSerialPortHandle = CreateFile(	name,
											GENERIC_READ | GENERIC_WRITE,
											0,
											0,
											OPEN_EXISTING,
											FILE_FLAG_OVERLAPPED,
											NULL);
		/// Try to set port event mask
		if (SetCommMask(m_hSerialPortHandle, EV_RXCHAR | EV_TXEMPTY) == 0)
		{
			AfxMessageBox(_T("Unable to set port event mask")); //TODO: ??
			return FALSE;
		}
		/// Try to get current port settings
		if (GetCommState(m_hSerialPortHandle, &m_SerialPortDCB) == 0)
		{
			AfxMessageBox(_T("Unable to get port configuration.")); // TODO: ??
			return FALSE;
		}
		m_SerialPortDCB.BaudRate = dcb.BaudRate;
		m_SerialPortDCB.StopBits = dcb.StopBits;
		m_SerialPortDCB.Parity	= dcb.Parity;
		m_SerialPortDCB.ByteSize = dcb.ByteSize;

		/// Try to set demanded port settings.
		if (SetCommState(m_hSerialPortHandle, &m_SerialPortDCB) == 0)
		{
			AfxMessageBox(_T("Unable to set port configuration.")); // TODO: ??
			return FALSE;
		}

		/// Configure port timeouts
		COMMTIMEOUTS commTimeouts;
		commTimeouts.ReadIntervalTimeout = 3;			/**< Max delay between two bytes*/
		commTimeouts.ReadTotalTimeoutMultiplier = 3;	/**< Multiplier to calculate total transmission timeout*/
		commTimeouts.ReadTotalTimeoutConstant = 2;		/**< Constant added to timeout*/
		commTimeouts.WriteTotalTimeoutMultiplier = 3;	/**< Corresponding value to write timeout*/
		commTimeouts.WriteTotalTimeoutConstant = 2;		/**< As above */
		/// Set timeouts
		SetCommTimeouts(m_hSerialPortHandle, &commTimeouts);

		/// Create and start thread
		m_hThreadTerminator = CreateEvent(0, 0, 0, 0);
		m_hThreadStarted	= CreateEvent(0, 0, 0, 0);
		m_hThread			= (HANDLE)_beginthreadex(0, 0, SerialPort::ThreadFn, (void*)this, 0, 0);
		
		/// Wait till thread started
		WaitForSingleObject(m_hThreadStarted, INFINITE);
		CloseHandle(m_hThreadStarted);
		m_hThreadStarted = INVALID_HANDLE_VALUE;

		/// Update port status
		m_bSerialPortStatus = TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL SerialPort::close()
{
	if (m_bSerialPortStatus == TRUE)
	{
		SignalObjectAndWait(m_hThreadTerminator, m_hThread, INFINITE, FALSE); /**< Signal thread termination event */
		m_bSerialPortStatus = FALSE;
		if (CloseHandle(m_hSerialPortHandle) == 0) /**< Close the port */
		{
			AfxMessageBox(_T("Failed closing the port."));
			return FALSE;
		}
		else return TRUE;
	}
	else return FALSE; /**< Port wasnt opened */
}

BOOL SerialPort::read(char* data, const unsigned int& sizeBuffer, unsigned long& length)
{
	/// Try to read the port
	if (ReadFile(m_hSerialPortHandle,
		data,
		sizeBuffer,
		&length,
		NULL) == 0)
	{
		AfxMessageBox(_T("Failed reading the port."));
		return FALSE;
	}
	/// If received some data, set end flag at the end of message.
	if (length > 0)
	{
		data[length] = NULL;
		return TRUE;
	}
	return TRUE;
}


BOOL SerialPort::write(LPCVOID data, const unsigned int& sizeBuffer, unsigned long& length)
{
	if (length > 0)
	{
		if (WriteFile(m_hSerialPortHandle,
			data,
			sizeBuffer,
			&length,
			NULL) == 0)
		{
			AfxMessageBox(_T("Failed to write to port."));
			return FALSE;
		}
		else return TRUE;
	}
	else return FALSE;
}


HANDLE SerialPort::getPortHandle()
{
	return m_hSerialPortHandle;
}

BOOL SerialPort::getPortStatus()
{
	return m_bSerialPortStatus;
}


unsigned __stdcall SerialPort::ThreadFn(void* pvParam)
{

}