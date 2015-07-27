// MouseController.cpp : Defines the entry point for the application.
//
#include <Windows.h>
#include "stdafx.h"
#include "MouseController.h"
#include "Modbus.h"
#include "SerialPort.h"
#include "Controller.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								/*< Current instance */
TCHAR szTitle[MAX_LOADSTRING];					/*< The title bar text */
TCHAR szWindowClass[MAX_LOADSTRING];			/*< The main window class name */

HWND g_hWnd;									/*< Main window handle */
HWND g_hSendButton;								/*< START button handle */
HWND g_hStopButton;								/*< STOP button handle */
HWND g_hSetButton;								/*< SET button handle */
HWND g_hMouseFactorEdit;						/*< Mouse factor edit handle */
HWND g_hSamplingTimeEdit;						/*< Sampling time edit handle */
HWND g_hConsoleText;							/*< console view handle */
HWND g_hVector1Label;
HWND g_hVector2Label;
HWND g_hVector3Label;
HWND g_hVectorDisplayEdit;

Modbus g_cModbus;				/*< Modbus communicator object*/
SerialPort g_cSerialPort;		/*< Serial Port object */
Controller* g_cController;		/*< Controller object */

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
BOOL				InitHardware(void);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
bool updateVector(const std::string& data);
bool  updateConsole(const std::string& data);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	//Controller cController(0x1, &g_cModbus);		/*< Controller object */
	

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MOUSECONTROLLER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	// Perform HW initialization:
	if (!InitHardware())
	{
		return FALSE;
	}
	g_cController = new Controller(1, &g_cModbus);
	g_cController->setSerialPort(&g_cSerialPort);
	/// Setup GUI callbacks
	g_cController->setGuiCallback(updateVector);
	g_cController->setTraceCallback(updateConsole);
	/// Start controller thread
	g_cController->start();


	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOUSECONTROLLER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	g_cController->terminate();
	g_cSerialPort.close();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOUSECONTROLLER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MOUSECONTROLLER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   

   hInst = hInstance; // Store instance handle in our global variable

   g_hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!g_hWnd)
   {
      return FALSE;
   }
   /// Create Start Button
   g_hSendButton = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"),
									_T("Start"), WS_CHILD | WS_VISIBLE | WS_BORDER, 
									10, 10, 70, 50, g_hWnd, (HMENU) IDM_STARTBUTTON, hInst, NULL);
   if (!g_hSendButton)
   {
	   return FALSE;
   }

   /// Create Stop Button
   g_hStopButton = CreateWindowEx(WS_EX_CLIENTEDGE, _T("BUTTON"),
	   _T("Stop"), WS_CHILD | WS_VISIBLE | WS_BORDER,
	   80, 10, 70, 50, g_hWnd, (HMENU)IDM_STOPBUTTON, hInst, NULL);
   if (!g_hStopButton)
   {
	   return FALSE;
   }

   /// Create Mouse Threshold Edit
   g_hMouseFactorEdit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"),
	   NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
	   130, 130, 70, 20, g_hWnd, NULL, hInst, NULL);
   if (!g_hMouseFactorEdit)
   {
	   return FALSE;
   }

   /// Create Mouse Threshold Edit
   g_hSamplingTimeEdit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"),
	   NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
	   130, 160, 70, 20, g_hWnd, NULL, hInst, NULL);
   if (!g_hSamplingTimeEdit)
   {
	   return FALSE;
   }

   /// Create Console Text View
   g_hConsoleText = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"),
									NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 
									10, 240, 300, 200, g_hWnd, NULL, hInst, NULL);   
   if (!g_hConsoleText)
   {
      return FALSE;
   }

   /// Create Result Vector text edit
   g_hVectorDisplayEdit = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"),
	   NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
	   130, 210, 200, 20, g_hWnd, NULL, hInst, NULL);
   if (!g_hVectorDisplayEdit)
   {
	   return FALSE;
   }

   /// Create labels
   HWND _hLabelMouseThresh = CreateWindowEx(NULL, _T("STATIC"),
	   _T("Mouse factor"), WS_CHILD | WS_VISIBLE,
	   10, 130, 120, 20, g_hWnd, NULL, hInst, NULL);
   if (!_hLabelMouseThresh)
   {
	   return FALSE;
   }

   HWND _hLabelSampleTime = CreateWindowEx(NULL, _T("STATIC"),
	   _T("Sampling time"), WS_CHILD | WS_VISIBLE,
	   10, 160, 120, 20, g_hWnd, NULL, hInst, NULL);
   if (!_hLabelSampleTime)
   {
	   return FALSE;
   }

   HWND _hLabelVector = CreateWindowEx(NULL, _T("STATIC"),
	   _T("Result vector"), WS_CHILD | WS_VISIBLE,
	   10, 210, 120, 20, g_hWnd, NULL, hInst, NULL);
   if (!_hLabelVector)
   {
	   return FALSE;
   }


   /// Window successfully built, now show it
   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   return TRUE;
}


BOOL InitHardware(void)
{
	/// Fill DCB structure with settings
	DCB dcb;
	memset(&dcb, 0, sizeof(dcb));

	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	/// Open port and check if failed
	BOOL retval = g_cSerialPort.open(dcb, _T("COM3"));
	if (!retval)
	{
		return FALSE;
	}

	/// Assign serial port to Modbus class.
	retval = g_cModbus.setSerialPort(&g_cSerialPort);
	if (!retval)
	{
		return FALSE;
	}

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_STARTBUTTON:
			g_cController->start();
			EnableWindow(g_hSendButton, false);
			break;
		case IDM_STOPBUTTON:
			g_cController->terminate();
			EnableWindow(g_hSendButton, true);
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


bool updateVector(const std::string& data)
{
	/// Do some job with converting from std::string to LPWSTR
	int bufferlen = MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), NULL, 0);
	if (bufferlen == 0)
		return false;

	LPWSTR widestr = new WCHAR[bufferlen + 1];

	MultiByteToWideChar(CP_ACP, 0, data.c_str(), data.size(), widestr, bufferlen);

	/// Ensure wide string is null terminated
	widestr[bufferlen] = 0;

	/// Set control text

	SetWindowText(g_hVectorDisplayEdit, widestr);

	delete[] widestr;
	
	return true;
}

bool  updateConsole(const std::string& data)
{
	return true;
}