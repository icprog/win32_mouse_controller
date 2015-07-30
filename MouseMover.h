#pragma once
class MouseMover
{
public:
	MouseMover();
	~MouseMover();
public:

	/**
	* @brief Starts Controller thread
	* @return True if start succeeded
	*/
	bool start();

	/**
	* @brief Ends Controller thread
	* @return True if termination succeeded
	*/
	bool terminate();

	/**
	* @brief Updates Euler angles
	* @param theta - theta angle
	* @param psi - psi angle
	* @return None
	*/
	void updateAngles(float theta, float psi, UINT32 buttonState);

	/**
	* @brief Sets mouse sensivity.
	* @param sensivity - mouse sensivity
	* @return None
	*/
	void setSensivity(int sensivity);

	/**
	* @brief Sets mouse update interval.
	* @param interval - mouse update interval
	* @return None
	*/
	void setInterval(int interval);

	/**
	* @brief Controller Thread function.
	* @details Contains thread's loop.
	* @param None.
	* @return None.
	*/
	static unsigned __stdcall ThreadFn(void* pvParam);
	/*TODO
	Napisz funkcjê w¹tku Controller.
	Otrzymawszy dane, powinna wywo³ywaæ Callback
	Callback ma byæ zaimplementowany w main'ie i powinien uaktualniaæ GUI
	*/

private:
	HANDLE		m_hThread;				/*< Thread handle */
	HANDLE		m_hThreadTerminator;	/*< Thread terminator event handle */
	HANDLE		m_hThreadStart;			/*< Thread start event handle */

	CRITICAL_SECTION	m_hLock;		/*< Critical section lock*/

	float m_pfAngles[2];				/*< Array with angles*/
	UINT32 m_uiButtonState;				/*< Button state bitfield*/

	int m_iSensivity;					/*< Mouse sensivity*/
	int m_iInterval;					/*< Mouse update interval*/
};

