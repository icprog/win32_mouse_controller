#include "stdafx.h"
#include "MouseMover.h"
#include "process.h"

MouseMover::MouseMover() :	m_iInterval(20),
							m_iSensivity(10)
{
	InitializeCriticalSection(&m_hLock);
}


MouseMover::~MouseMover()
{
	DeleteCriticalSection(&m_hLock);
}



bool MouseMover::start()
{

	m_hThreadStart = CreateEvent(0, 0, 0, 0);
	m_hThreadTerminator = CreateEvent(0, 0, 0, 0);

	m_hThread = (HANDLE)_beginthreadex(0, 0, MouseMover::ThreadFn, (void*)this, 0, 0);

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

bool MouseMover::terminate()
{
	/// Signal the terminator event and wait for thread to terminate
	DWORD dwWaitResult = SignalObjectAndWait(m_hThreadTerminator, m_hThread, 1000, FALSE);
	if (dwWaitResult == WAIT_TIMEOUT)
	{
		return false;
	}
	CloseHandle(m_hThreadTerminator);
	m_hThreadTerminator = INVALID_HANDLE_VALUE;
	m_hThread = INVALID_HANDLE_VALUE;
	return true;
}

void MouseMover::updateAngles(float theta, float psi, UINT32 buttonState)
{
	EnterCriticalSection(&m_hLock);
	
	m_pfAngles[0] = theta;
	m_pfAngles[1] = psi;
	m_uiButtonState = buttonState;

	LeaveCriticalSection(&m_hLock);
}


void MouseMover::setSensivity(int sensivity)
{
	EnterCriticalSection(&m_hLock);
	m_iSensivity = sensivity;
	LeaveCriticalSection(&m_hLock);
}

void MouseMover::setInterval(int interval)
{
	EnterCriticalSection(&m_hLock);
	m_iInterval = interval;
	LeaveCriticalSection(&m_hLock);
}

unsigned __stdcall MouseMover::ThreadFn(void* pvParam)
{
	MouseMover* _pThis = (MouseMover*)pvParam;
	bool _bLoopThread = true;
	DWORD dwWaitResult = 0;

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

		

		/// Move the mouse!
		EnterCriticalSection(&(_pThis->m_hLock));

		INPUT input;
		ZeroMemory(&input, sizeof(input));
		input.type = INPUT_MOUSE;
		input.mi.dx = _pThis->m_pfAngles[0] * _pThis->m_iSensivity;
		input.mi.dy = _pThis->m_pfAngles[1] * _pThis->m_iSensivity;

		if (input.mi.dx == 0)
			input.mi.dwFlags = MOUSEEVENTF_MOVE;

		input.mi.dwFlags = MOUSEEVENTF_MOVE;
		if (_pThis->m_uiButtonState) input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
		else input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
		UINT retval = SendInput(1, &input, sizeof(input));

		LeaveCriticalSection(&(_pThis->m_hLock));

		Sleep(_pThis->m_iInterval);
	} /*< End of thread loop*/

	return 0;
}