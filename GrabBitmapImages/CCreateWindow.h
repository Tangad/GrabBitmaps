#pragma once
class CCreateWindow
{
public:
	CCreateWindow();
	~CCreateWindow();
	static  LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
	HRESULT	Create(HINSTANCE hInstance);
	HRESULT	CCW_Show(int nCmdShow);
	virtual	LRESULT OnReceiveMessage(UINT uiMessage, WPARAM wParam, LPARAM lParam);
protected:
	HRESULT	Register();
	virtual	LPCTSTR ClassName() const=0;
	virtual	LPCTSTR MenuName() const {return NULL;}
	virtual	LPCTSTR WindowName() const=0;
	virtual	void Paint_()=0;
	HWND m_hwnd;
	HINSTANCE m_hInstance;
};