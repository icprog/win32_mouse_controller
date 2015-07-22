#include "afxwin.h"_
#include "stdafx.h"
#include "process.h"
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

BOOL SerialPort::open(DCB dcb, TCHAR* name)
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
//			AfxMessageBox(_T("Unable to set port event mask")); //TODO: ??
			return FALSE;
		}
		/// Try to get current port settings
		if (GetCommState(m_hSerialPortHandle, &m_SerialPortDCB) == 0)
		{
//			AfxMessageBox(_T("Unable to get port configuration.")); // TODO: ??
			return FALSE;
		}
		m_SerialPortDCB.BaudRate = dcb.BaudRate;
		m_SerialPortDCB.StopBits = dcb.StopBits;
		m_SerialPortDCB.Parity	= dcb.Parity;
		m_SerialPortDCB.ByteSize = dcb.ByteSize;

		/// Try to set demanded port settings.
		if (SetCommState(m_hSerialPortHandle, &m_SerialPortDCB) == 0)
		{
//			AfxMessageBox(_T("Unable to set port configuration.")); // TODO: ??
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
//			AfxMessageBox(_T("Failed closing the port."));
			return FALSE;
		}
		else return TRUE;
	}
	else return FALSE; /**< Port wasnt opened */
}

BOOL SerialPort::read(std::string& data, unsigned int& size, unsigned long timeout)
{
	/// Return false if atempting to read from disconnected port
	if (getPortStatus() == false)
		return false;
	bool _bContinue = true;
	while (_bContinue)
	{
		DWORD dwWait = WaitForSingleObject(m_hEventRx, timeout);
		switch (dwWait)
		{
			case WAIT_TIMEOUT:
			{
				_bContinue = false;
			} break;
			case WAIT_OBJECT_0:
			{
								  __nop();
				///w sumie to nic do roboty tu.
			} break;
		}
	}

	size = m_szFrameBuffer.size();
	if (size > 0)
	{
		acquireLock();
		data = m_szFrameBuffer;
		m_szFrameBuffer.clear();
		unlock();
	}
	

}


BOOL SerialPort::write(const unsigned char* data, DWORD dwSize)
{
	/// If not connected, don't bother trying.
	if (m_bSerialPortStatus == false)
	{
		return false;
	}

	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent(0, true, 0, 0);
	DWORD dwBytesWritten = 0;
	acquireLock();
	bool _bRet = WriteFile(m_hSerialPortHandle,
		data,
		dwSize,
		&dwBytesWritten,
		&ov);
	/// WriteFile will return false if it returns asynchronously (this is our situation)
	if (!_bRet)
	{
		/// So we wait for write to finish
		WaitForSingleObject(ov.hEvent, INFINITE);
	}
	
	Sleep(1);
	unlock();
	CloseHandle(ov.hEvent);
	return true;
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
	SerialPort* _pThis = (SerialPort*)pvParam;
	bool _bContinue = true;
	DWORD dwEventMask = 0;

	OVERLAPPED ov;
	ov.hEvent = CreateEvent(0, true, 0, 0);
	HANDLE _rHandles[2];
	_rHandles[0] = _pThis->m_hThreadTerminator;

	DWORD dwWait;
	/// Notify that thread started
	SetEvent(_pThis->m_hThreadStarted);

	/// Thread loop
	while (_bContinue)
	{
		bool _bRetval = WaitCommEvent(_pThis->m_hSerialPortHandle, &dwEventMask, &ov);
		if (!_bRetval)
		{
			// TODO handle this.
		}

		_rHandles[1] = ov.hEvent;

		/// dwWait will store index of event that just occured
		dwWait = WaitForMultipleObjects(2, _rHandles, FALSE, INFINITE);

		switch (dwWait)
		{
		case WAIT_OBJECT_0: /**< If thread terminator event occured, just end thread */
			{
				 _endthreadex(1);
				 _bContinue = false;
			} break;
		case WAIT_OBJECT_0 + 1:
			{
				DWORD dwMask;
				if (GetCommMask(_pThis->m_hSerialPortHandle, &dwMask))
				{
					if (dwMask == EV_TXEMPTY)
					{
						///Data send finished
						// TODO: do stuff here
						ResetEvent(ov.hEvent);
						continue;
					}

				}

				int iAccumulator = 0;
				
				///Enter critical section
				_pThis->acquireLock();

				bool _bRet = false;
				DWORD dwBytesRead = 0;
				OVERLAPPED ovRead;
				memset(&ovRead, 0, sizeof(ovRead));
				ovRead.hEvent = CreateEvent(0, true, 0, 0);

				do
				{
					ResetEvent(ovRead.hEvent);
					char szTmp[10];
					int iSize = 1;
					_bRet = ReadFile(_pThis->m_hSerialPortHandle, szTmp, iSize, &dwBytesRead, &ovRead);
					if (!_bRet)
					{
						
						break;
					}
			
					if (dwBytesRead > 0)
					{
						_pThis->m_szFrameBuffer.append(szTmp, dwBytesRead);
						iAccumulator += dwBytesRead;
					}
				} while (dwBytesRead > 0);
				CloseHandle(ovRead.hEvent);

				


				///Exit critical section
				_pThis->unlock();

				if (iAccumulator > 0)
				{
					SetEvent(_pThis->m_hEventRx);
				}
				ResetEvent(ov.hEvent);
			} break; 

		}
	}
	return 0;
}

