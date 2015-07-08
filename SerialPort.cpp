#include "stdafx.h"
#include "afxwin.h"
#include "SerialPort.h"


SerialPort::SerialPort() :	m_bSerialPortStatus(FALSE),
							m_hSerialPortHandle(NULL)
{
	m_SerialPortDCB.ByteSize = 8;
	m_SerialPortDCB.BaudRate = CBR_115200;
	m_SerialPortDCB.Parity	 = NOPARITY;
	m_SerialPortDCB.StopBits = ONESTOPBIT;
}


SerialPort::~SerialPort()
{
	m_hSerialPortHandle = NULL;
}

BOOL SerialPort::open(DCB dcb, const char* name = "COM1")
{
	if (m_bSerialPortStatus == FALSE)		/**< Check if port closed. Dont try to open, if port already opened. */
	{
		/// Try to open device
		m_hSerialPortHandle = CreateFile(	name,
											GENERIC_READ | GENERIC_WRITE,
											0,
											0,
											OPEN_EXISTING,
											0,
											NULL);
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