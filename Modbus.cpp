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
	if (std::find(m_vectDeviceList.begin(), m_vectDeviceList.end(), iDeviceAddress) == m_vectDeviceList.end())
	{
		m_vectDeviceList.push_back(iDeviceAddress);
		return true;
	}
	else return false;

}

bool Modbus::removeDevice(int iDeviceAddress)
{
	if (std::find(m_vectDeviceList.begin(), m_vectDeviceList.end(), iDeviceAddress) == m_vectDeviceList.end())
		return false;
	m_vectDeviceList.erase(std::remove(m_vectDeviceList.begin(), m_vectDeviceList.end(), iDeviceAddress), m_vectDeviceList.end());
	return true;
}

bool Modbus::sendCommand(int iDeviceAddress, int iCommandCode, unsigned char* arguments, int iArgCount, std::string &response, unsigned int &response_size)
{
	unsigned char data[20];
	data[0] = (char)iDeviceAddress;
	data[1] = (char)iCommandCode;
	memcpy(data + 2, arguments, iArgCount);

	/// SerialPort::write is blocking function!
	if (!(m_pSerialPort->write(data, 2 + iArgCount)))
	{
		return false;
	}
	
	///We'll wait here to receive data. 
	std::string rxData;
	unsigned int size = 0;
	m_pSerialPort->read(rxData,size,100);
	if (size == 0)
		return false;

	rxData.erase(0, 2);
	response = rxData;
	size -= 2;
	response_size = size;
	return true;
}
