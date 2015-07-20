#include "stdafx.h"
#include "Controller.h"


Controller::Controller()
{
	m_cModbus = new Modbus();
}


Controller::~Controller()
{
	delete m_cModbus;
}

bool Controller::setSerialPort(SerialPort*  sp)
{
	/// Assign port to Modbus object
	return m_cModbus->setSerialPort(sp);
}

unsigned __stdcall Controller::ThreadFn(void* pvParam)
{
	Controller* _pThis = (Controller*)pvParam;
	bool _bLoopThread = true;
	DWORD dwEventMask = 0;

	///TODO ....
}