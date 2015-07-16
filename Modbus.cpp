#include "stdafx.h"
#include "Modbus.h"



Modbus::Modbus()
{
}


Modbus::~Modbus()
{
}

bool Modbus::setSerialPort(SerialPort* sp)
{
	if (sp->getPortStatus() == true)
	{
		m_pSerialPort = sp;
	}
	else return false;

}

bool Modbus::addDevice(int iDeviceAddress)
{
	m_vectDeviceList.push_back(iDeviceAddress);

}

bool Modbus::removeDevice(int iDeviceAddress)
{
	m_vectDeviceList.erase(std::remove(m_vectDeviceList.begin(), m_vectDeviceList.end(), iDeviceAddress), m_vectDeviceList.end());
}

bool Modbus::sendCommand(int iDeviceAddress, int iCommandCode, unsigned char* arguments, int iArgCount)
{
	//m_pSerialPort->write();

	///We'll wait here to receive data. 
	//	m_pSerialPort->read();
}
