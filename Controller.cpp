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

	if (m_cMouseMover.start() == false)
		return false;

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

	/// Stop also mouse mover thread
	if (m_cMouseMover.terminate() == false)
	{
		return false;
	}

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

		float boardData[9];
		UINT32 buttonState = 0;
		std::string response;
		unsigned int responseSize = 0;
		bool dataAcquired = false;
		int dataToAcquireNum = 10;
		
		/// Arguments of command: read 9 bytes from index 0
		unsigned char argument[2] = { 0, dataToAcquireNum };

		/// Loop till proper data received
		while (!dataAcquired)
		{
			/// Send READ 32BIT DATA command
			_pThis->m_cModbus->sendCommand(_pThis->m_iDeviceAddress,
											Modbus::COMMAND_READ_32BIT,
											argument, 2, response,
											responseSize);
			/// If received proper amount of data, convert in to float array
			if (responseSize == (dataToAcquireNum * sizeof(float)))
			{
				memcpy(&buttonState, response.c_str(), sizeof(buttonState));
				for (int i = 0; i < dataToAcquireNum; i++)
				{
					memcpy(&(boardData[i]), response.c_str() + (i+1)*sizeof(float), sizeof(float));
				}
				/// Escape from loop
				dataAcquired = true;

			}
			
		}
		
		/// Store data for calibration purposes if enabled
		if (_pThis->m_bStoreData)
		{
			std::ofstream dataFile;
			dataFile.open("data.dat", std::ofstream::app);
			dataFile << formatString(boardData, sizeof(boardData)/sizeof(float), 3) << std::endl;
			dataFile.close();
		}


		/// Do the mathematical processing
		float result[3];
		_pThis->processData(boardData, result);

		/// Update GUI values
		std::string resultString = formatString(result, 3, 3);
		_pThis->m_pfGuiCallback(resultString);

		_pThis->m_cMouseMover.updateAngles(result[0], result[1], buttonState);

	} /*< End of thread loop*/

	return 0;
}

void Controller::setMoverParams(int factor, int samplingTime)
{
	if (factor != -1)
	{
		m_cMouseMover.setSensivity(factor);
	}
	if (samplingTime != -1)
	{
		m_cMouseMover.setInterval(factor);
	}
}

void Controller::enableDataStorage(bool enable)
{
	m_bStoreData = enable;
}

bool Controller::isStorageEnabled()
{
	return m_bStoreData;
}



bool Controller::processData(float* boardData, float* result)
{
	/// @TODO 
	//memcpy(result, magnField, 3 * sizeof(float));

	float theta = -1 * atan2(boardData[1], boardData[2]);
	float psi = -1 * atan2(boardData[0], boardData[2]);
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