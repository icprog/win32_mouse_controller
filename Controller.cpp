#include "stdafx.h"
#include "process.h"
#include "Controller.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

Controller::Controller(int deviceAddress, Modbus* modbusInstance) : 
											m_hThreadStart(INVALID_HANDLE_VALUE), 
											m_hThreadTerminator(INVALID_HANDLE_VALUE),
											m_hThread(INVALID_HANDLE_VALUE),
											m_iDeviceAddress(deviceAddress),
											m_cModbus(modbusInstance),
											m_pfGuiCallback(NULL),
											m_pfTraceCallback(NULL)
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
	m_hThread = INVALID_HANDLE_VALUE;
	return true;
}

void Controller::setTraceCallback(gui_callback cb)
{
	m_pfTraceCallback = cb;
}

void Controller::setGuiCallback(gui_callback cb)
{
	m_pfGuiCallback = cb;
}

unsigned __stdcall Controller::ThreadFn(void* pvParam)
{
	Controller* _pThis = (Controller*)pvParam;
	bool _bLoopThread = true;
	DWORD dwWaitResult = 0;
	unsigned char argument[1] = { 0 };
	float acceleration[3] = { 0.00 };
	float ang_rate[3] = { 0.00 };
	float magn_field[3] = { 0.00 };
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

		/// Get accelerometer data
		memset(acceleration, 0, sizeof(acceleration));
		for (int i = 0; i < 3; i++)
		{
			//Sleep(100);
			std::string response;
			unsigned int responseSize =0 ;
			argument[0] = i;
			/// Query device until it answers
			bool data_acquired = false;
			while (!data_acquired)
			{
				_pThis->m_cModbus->sendCommand(_pThis->m_iDeviceAddress,
												Modbus::COMMAND_READ_32BIT,
												argument, 1, response,
												responseSize);
				if (responseSize == 4)
				{
					char bytes[4];
					std::copy(response.begin(), response.end(), bytes);
					memcpy(acceleration + i, bytes, sizeof(bytes));
					data_acquired = true;
				}
			}
		}
		/*
		/// Get magn data
		memset(magn_field, 0, sizeof(magn_field));
		for (int i = 0; i < 3; i++)
		{
			//Sleep(100);
			std::string response;
			unsigned int responseSize = 0;
			argument[0] = i+3;
			/// Query device until it answers
			bool data_acquired = false;
			while (!data_acquired)
			{
				_pThis->m_cModbus->sendCommand(_pThis->m_iDeviceAddress,
					Modbus::COMMAND_READ_32BIT,
					argument, 1, response,
					responseSize);
				if (responseSize == 4)
				{
					char bytes[4];
					std::copy(response.begin(), response.end(), bytes);
					memcpy(magn_field + i, bytes, sizeof(bytes));
					data_acquired = true;
				}
			}
		}*/

		/// Get gyro data
		memset(ang_rate, 0, sizeof(ang_rate));
		for (int i = 0; i < 3; i++)
		{
			//Sleep(100);
			std::string response;
			unsigned int responseSize = 0;
			argument[0] = i + 6;
			/// Query device until it answers
			bool data_acquired = false;
			while (!data_acquired)
			{
				_pThis->m_cModbus->sendCommand(_pThis->m_iDeviceAddress,
					Modbus::COMMAND_READ_32BIT,
					argument, 1, response,
					responseSize);
				if (responseSize == 4)
				{
					char bytes[4];
					std::copy(response.begin(), response.end(), bytes);
					memcpy(ang_rate + i, bytes, sizeof(bytes));
					data_acquired = true;
				}
			}
		}
		
		/// Process data
		/*
		std::ofstream dataFile;
		dataFile.open("data.dat", std::ofstream::app);
		dataFile << formatString(acceleration, 3, 4) << formatString(ang_rate, 3, 4) << std::endl;
		dataFile.close();
		*/
		float result[3];
		_pThis->processData(acceleration, ang_rate, magn_field, result);
		std::string resultString = formatString(result, 3, 3);
		_pThis->m_pfGuiCallback(resultString);
		INPUT input;
		ZeroMemory(&input, sizeof(input));
		input.type = INPUT_MOUSE;
		input.mi.dx = result[0]*100;
		input.mi.dy = result[1]*100;
		input.mi.dwFlags = MOUSEEVENTF_MOVE;
		UINT retval = SendInput(1, &input, sizeof(input));

		//Sleep(200);
	} /*< End of thread loop*/

	return 0;
}

bool Controller::processData(float* acc, float* angRate, float* magnField, float* result)
{
	/// @TODO 
	//memcpy(result, magnField, 3 * sizeof(float));

	float theta = -1 * atan2(acc[1], acc[2]);
	float psi = -1 * atan2(acc[0], acc[2]);
	result[0] = theta;
	result[1] = psi;
	result[2] = 0;
	return false;
}

std::string Controller::formatString(float* data, size_t dataSize, size_t precision)
{
	std::stringstream ss;
	for (int i = 0; i < dataSize; i++)
	{
		ss << std::fixed << std::setprecision(precision) << data[i] << " ";
	}
	return ss.str();
	
}