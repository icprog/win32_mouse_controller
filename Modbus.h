#pragma once
#include "SerialPort.h"
#include <vector>
#include <algorithm>
class Modbus
{
public:
	Modbus();
	~Modbus();
	
	bool setSerialPort(SerialPort* sp);
	bool addDevice(int iDeviceAddress);
	bool removeDevice(int iDeviceAddress);
	bool sendCommand(int iDeviceAddress, int iCommandCode, unsigned char* arguments, int iArgCount);
	
private:
	std::vector<int> m_vectDeviceList;

	SerialPort* m_pSerialPort;
};

