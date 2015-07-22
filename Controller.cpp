#include "stdafx.h"
#include "process.h"
#include "Controller.h"


Controller::Controller(int deviceAddress, Modbus* modbusInstance) : 
											m_hThreadStart(INVALID_HANDLE_VALUE), 
											m_hThreadTerminator(INVALID_HANDLE_VALUE),
											m_hThread(INVALID_HANDLE_VALUE),
											m_iDeviceAddress(deviceAddress),
											m_cModbus(modbusInstance)
{
}


Controller::~Controller()
{
}

bool Controller::setSerialPort(SerialPort*  sp)
{
	/// Assign port to Modbus object
	return m_cModbus->setSerialPort(sp);
}

bool Controller::start()
{
	m_hThreadStart		= CreateEvent(0, 0, 0, 0);
	m_hThreadTerminator = CreateEvent(0, 0, 0, 0);
	
	m_hThread = (HANDLE)_beginthreadex(0, 0, Controller::ThreadFn, (void*)this, 0, 0);

	DWORD dwWaitResult = WaitForSingleObject(m_hThreadStart, 1000);
	/// Thread didnt start
	if (dwWaitResult == WAIT_TIMEOUT)
	{
		return false;
	}
	
	CloseHandle(m_hThreadStart);
	m_hThreadStart = INVALID_HANDLE_VALUE;

	return true;
}

bool Controller::terminate()
{
	/// Signal the terminator event and wait for thread to terminate
	DWORD dwWaitResult = SignalObjectAndWait(m_hThreadTerminator, m_hThread, 1000, FALSE);
	if (dwWaitResult == WAIT_TIMEOUT)
	{
		return false;
	}
	CloseHandle(m_hThreadTerminator);
	m_hThreadTerminator = INVALID_HANDLE_VALUE;
	return true;
}

unsigned __stdcall Controller::ThreadFn(void* pvParam)
{
	Controller* _pThis = (Controller*)pvParam;
	bool _bLoopThread = true;
	DWORD dwWaitResult = 0;
	unsigned char argument[1] = { 0 };
	float acceleration[3] = { 0.00 };

	/// Notify that thread started successfully
	SetEvent(_pThis->m_hThreadStart);

	/// Thread main loop. First we check if event termination was requested, if not - we're doing data aqcuisition and processing
	while (_bLoopThread)
	{
		/// Check if terminator event occured
		dwWaitResult = WaitForSingleObject(_pThis->m_hThreadTerminator, 0);
		if (dwWaitResult == WAIT_OBJECT_0)
		{
			/// Terminator event occured, time to terminate the thread
			_endthreadex(1);
			_bLoopThread = false;
			break;
		}
		memset(acceleration, 0, sizeof(acceleration));
		for (int i = 0; i < 3; i++)
		{
			std::string response;
			unsigned int responseSize =0 ;
			argument[0] = i;
			_pThis->m_cModbus->sendCommand(_pThis->m_iDeviceAddress,
				Modbus::COMMAND_READ_32BIT,
				argument, 1, response,
				responseSize);
			if (responseSize != 0)
			{
				char bytes[4];
				std::copy(response.begin(), response.end(), bytes);
				memcpy(acceleration + i, bytes, sizeof(bytes));
			}
		}

	} /*< End of thread loop*/

	return 0;
}