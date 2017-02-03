#include "stdafx.h"
#include "CCreateWindow.h"
/* Constructor */
CCreateWindow::CCreateWindow() : 
	m_hwnd(NULL), 
	m_hInstance(NULL)
{
}
/* Destructor */
CCreateWindow::~CCreateWindow()
{
}
//----------------------------------------------------------
//Method @@@
//				Register()
//Description @@@
//				Registers the window class.
//				
//------------------------------------------------------------
HRESULT CCreateWindow::Register()
{
	WNDCLASSEX wndClassInfo;
	wndClassInfo.cbSize=sizeof(WNDCLASSEX);
	wndClassInfo.style			= CS_HREDRAW | CS_VREDRAW;
	wndClassInfo.lpfnWndProc	= WindowProc;
	wndClassInfo.cbClsExtra		= 0;
	wndClassInfo.cbWndExtra		= 0;
	wndClassInfo.hInstance		= m_hInstance;
	wndClassInfo.hIcon			= NULL; 
	wndClassInfo.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndClassInfo.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wndClassInfo.lpszMenuName	= MenuName();
	wndClassInfo.lpszClassName	= ClassName();
	wndClassInfo.hIconSm		= NULL;
	ATOM atom=RegisterClassEx(&wndClassInfo);
	if (atom==0)
		return __HRESULT_FROM_WIN32(GetLastError());
	else
		return S_OK;
}
//----------------------------------------------------------
//Method @@@
//				Create(HINSTANCE hInstance)
//Description @@@
//				Creates an instance of the window.
//				Creating Main Window
//					1. Register class
//					2. Creating Window
//				
//------------------------------------------------------------
HRESULT CCreateWindow::Create(HINSTANCE hInstance)
{
	m_hInstance=hInstance;
	HRESULT hResult=Register();
	if(SUCCEEDED(hResult)){
		HWND hwnd=CreateWindow(ClassName(),
							   WindowName(), 
							   WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
							   NULL,NULL, 
							   m_hInstance, 
							   this
							  );
		if(hwnd==0)
			hResult=__HRESULT_FROM_WIN32(GetLastError());
	}
	return hResult;
}
//----------------------------------------------------------
//Method @@@
//				CCW_Show(int nCmdShow)
//Description @@@
//				 Once Created the Window, Show or Hide the window
//				
//------------------------------------------------------------
HRESULT CCreateWindow::CCW_Show(int nCmdShow)
{
	ShowWindow(m_hwnd,nCmdShow);
	UpdateWindow(m_hwnd);
	return S_OK;
}
//----------------------------------------------------------
//Method @@@
//				WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//Description @@@
//				 Window procedure.
//				
//------------------------------------------------------------
LRESULT CALLBACK CCreateWindow::WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CCreateWindow* ccw_pWin=NULL;
	if(uMsg==WM_NCCREATE){
		/* When we create the window, we pass in a pointer to this class as part of the CREATESTRUCT structure */
		LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lParam;
		ccw_pWin=(CCreateWindow*)lpcs->lpCreateParams;		
    	/* Set the window handle */
        ccw_pWin->m_hwnd=hwnd;
 		/* Set the pointer to the class as user data */
		SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)ccw_pWin);
	}else{
        /* Get the pointer to the class */
		ccw_pWin=(CCreateWindow*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	}
	if(ccw_pWin) 
		return ccw_pWin->OnReceiveMessage(uMsg,wParam,lParam);
	else 
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
}
//----------------------------------------------------------
//Method @@@
//				OnReceiveMessage(UINT uiMessage, WPARAM wParam, LPARAM lParam)
//Description @@@
//				Window Procedure Å® Message Handle from Wimdows System
//				Handle window messages other than WM_NCCREATE.
//				
//------------------------------------------------------------
LRESULT CCreateWindow::OnReceiveMessage(UINT uiMessage,WPARAM wParam,LPARAM lParam)
{
	switch(uiMessage) 
	{
		case WM_NCDESTROY:
			SetWindowLongPtr(m_hwnd,GWLP_USERDATA,0);
			return DefWindowProc(m_hwnd,uiMessage,wParam,lParam);
		case WM_PAINT:
			Paint_();
			return 0;
	}
	return DefWindowProc(m_hwnd,uiMessage,wParam,lParam);
}