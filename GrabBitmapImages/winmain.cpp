// CPlayBack.cpp : アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "Resource.h"
#include "CMainWindowPlayBack.h"
#define MAX_LOADSTRING 100
int APIENTRY _tWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
 	MSG msg;
	HACCEL hAccelTable;
	hAccelTable=LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PLAYBACK));
	CMainWindowPlayBack* pWin=new CMainWindowPlayBack();
	HRESULT hResult;
	hResult=CoInitialize(NULL);
	hResult=pWin->Create(hInstance);
	hResult=pWin->CCW_Show(nCmdShow);
	while(GetMessage(&msg,NULL,0,0)){
		if(!TranslateAccelerator(msg.hwnd,hAccelTable,&msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	delete pWin;
	CoUninitialize();
	return (int) msg.wParam;
}