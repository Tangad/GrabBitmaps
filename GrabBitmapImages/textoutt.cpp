#include "stdafx.h"
#include "CWindowControl.h"
#include "textoutt.h"
#include "slider.h"
#include <windowsx.h>
#include <dshow.h>
const		LONGLONG NANOSECONDS = (1000000000);
const		LONGLONG UNITS		 = (NANOSECONDS / 100); 
const		LONGLONG ONE_MSEC	 = 10000;		
namespace TextOutControl
{
	LPCTSTR ClassName	= TEXT("TEXTOUT_TIME");
	LPCTSTR	ClassName_	= TEXT("TEXTOUT_DUR");
	LPCTSTR	InstanceData= TEXT("TEXTOUT_PROP");
	struct TextOut_Info
	{
		LONGLONG lltimePos; // Logical units
		HBRUSH hBackGround;	// GDI
	};
	// Window Procedure:hwnd is the handler to combo
	LRESULT CALLBACK TextOut_WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	// Message Handler
	LRESULT OnCreate(HWND hwnd); 
	LRESULT	OnNcDestroy(HWND hwnd,TextOut_Info* pInfo);
	LRESULT	OnPaint(HWND hwnd,TextOut_Info* pInfo); 
	// Those are called in Window Procedure 
	LRESULT	OnSetTime(HWND hwnd,LONG lPos,const TextOut_Info* pInfo);
	LRESULT	OnGetTime();
	LRESULT	OnSetMinMax();
	//@from OnCreate function 
	inline BOOL SetInfo(HWND hwnd,TextOut_Info* pInfo)
	{
		return SetProp(hwnd,InstanceData,pInfo);
	}
	inline TextOut_Info* GetInfo(HWND hwnd)
	{
		return (TextOut_Info*)GetProp(hwnd,InstanceData);
	}	
	// List View wimdow procedure
	// hwnd  is the handler of the Combo Box
	LRESULT CALLBACK TextOut_WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		TextOut_Info* const pInfo=GetInfo(hwnd);
		switch (uMsg)
		{
			case WM_CREATE:
				return 	OnCreate(hwnd);
			case WM_PAINT:
				return	OnPaint(hwnd,pInfo);
			case WM_NCDESTROY:
				return	OnNcDestroy(hwnd,pInfo);
			case WM_SIZE: /* Additional to position change */
				return 1;
			default:
				return DefWindowProc(hwnd,uMsg,wParam,lParam);
		}
		return 0;
	};
	//--------------------------------------------------------------------------------------
    // NotifyParent
    // Description:	Send the parent window a WM_NOTIFY message with our current status.
    //				when an event has occurred or the control requires some information. 
    // hwnd:		Control window.
    // code:		WM_NOTIFY code. (One of the SLIDER_NOTIFY_xxx constants.)
    // pInfo:		Instance data.
    //--------------------------------------------------------------------------------------
	void NotifyParent(HWND hwnd,UINT NotifyCode,TextOut_Info* pInfo)
	{
		// hwnd	¨ Handler to the hwnd
		// code	¨ Notification message 
		// pInfo¨ Slider time position
		// Get Handler to the parent window
		HWND hParent=GetParent(hwnd);
		if(hParent)
		{	
			NM_TEXTOUT_INFO TextOutInfo;
			TextOutInfo.hdr.hwndFrom = hwnd;
			TextOutInfo.hdr.code     = NotifyCode;
			TextOutInfo.hdr.idFrom   = (UINT)GetMenu(hwnd);
			TextOutInfo.llPosition	 = pInfo->lltimePos;
			// Notify to Parent Window
			SendMessage( hParent, WM_NOTIFY,(WPARAM)TextOutInfo.hdr.idFrom,(LPARAM)&TextOutInfo);
		}		
	}
	//-----------------------------------------------------
	// TextOut_WndProc¨ WM_CREATE message ¨ Creating
	//-----------------------------------------------------
	LRESULT	OnCreate(HWND hwnd)
	{
		TextOut_Info* pInfo=new TextOut_Info();
		if(!pInfo)
			return (LRESULT) -1;
		ZeroMemory(pInfo, sizeof(TextOut_Info));
		//pInfo->hBackGround	= CreateSolidBrush(RGB(0xFF, 0x80, 0x80));
	    if(SetInfo(hwnd,pInfo)){
            return 0;
        }else{
            delete pInfo;
            return -1;
        }		
		return 0;		
	}
	//-----------------------------------------------------
	// TextOut_WndProc¨ WM_NCDESTROY message ¨ Destroying
	//-----------------------------------------------------
	LRESULT OnNcDestroy(HWND hwnd,TextOut_Info* pInfo)
	{
		//if pointer is not null
		if(pInfo){
			DeleteObject(pInfo->hBackGround);
			//DeleteObject(pInfo->lltimePos );
			delete pInfo;
		}
		return 0;	
	}
	//-----------------------------------------------------
	// TextOut_WndProc¨WM_PAINT message ¨ Painting
	//-----------------------------------------------------
	LRESULT OnPaint(HWND hwnd,TextOut_Info* pInfo)
	{
		PAINTSTRUCT ps;
		HDC hdc;
		BOOL bReturn=TRUE;
		RECT Rect_;
		LONG yPosition;
		LONG xPosition1;
		WCHAR wchTime[11];
		ZeroMemory(wchTime, sizeof(wchTime));
		/* Get handler to the DC */
		hdc=BeginPaint(hwnd,&ps);
		/* Set the Text Outtransparently */
		SetBkMode(hdc,TRANSPARENT);
		/* Get the rect of the Text Out */
		GetClientRect(hwnd,&Rect_);
		SelectClipPath(hdc,RGN_AND);
		/* Fill the Text Out */
		SetBkColor(hdc,RGB(236,233,216));		
		yPosition=0;
		xPosition1=0;	
		(void)StringCchPrintf(wchTime,NUMELMS(wchTime),_TEXT("0%d:0%d:0%d:%d"), 0,0,0,0);					
		int iRet=wcslen( wchTime );
		wchTime[iRet]=0;
		SetBkColor(hdc,RGB(236,233,216));		
		bReturn=TextOut(hdc,xPosition1,yPosition,wchTime,iRet);
		EndPaint(hwnd, &ps);
		return 0;
	}
}
//-----------------------------------------------------
// Registration
//-----------------------------------------------------
HRESULT TextOut_Init()
{
	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
	wndClass.cbSize		   =sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc   =TextOutControl::TextOut_WndProc;
	wndClass.hInstance	   =GetInstance();
	wndClass.lpszClassName =TextOutControl::ClassName;
	wndClass.cbWndExtra	   =sizeof(TextOutControl::TextOut_Info);	
	ATOM a=RegisterClassEx(&wndClass);
	if(a==0)
		return  __HRESULT_FROM_WIN32(GetLastError());
	else
		 return S_OK;
}
//-----------------------------------------------------
// Create Window 
//-----------------------------------------------------
HRESULT TextOutTime::Create(HINSTANCE hInstance,HWND hParent,DWORD_PTR id,DWORD dwStyle)
{
	if(m_hwnd != 0)
		return E_FAIL;
	HRESULT hResult=S_OK; 
	RECT rc;
	rc.left=0;
	rc.top=0;
	rc.right=74;
	rc.bottom=17;
	/*
	rc.left=420;
	rc.top=4;
	rc.right=494;
	rc.bottom=21;
	*/
	/* Position	*/
	HWND hwnd=CreateWindowEx(0,
							 TextOutControl::ClassName, NULL,
							 WS_CHILD | WS_VISIBLE ,
							 rc.left,		
							 rc.top,			
							 rc.right-rc.left,		 
							 rc.bottom-rc.top,			
							 hParent,
							 (HMENU)id,
							 hInstance,
							 NULL
							);
	if(hwnd==NULL){
		hResult=E_FAIL;
		return hResult;
	}else{	
		/* Object handler is set to the CWindowControl */
		m_hwnd=hwnd;
		SetPosition(rc);
		/* Set the Information */
		NMHDDISPINFO TextOut_;
		TextOut_.hdr.code     =NULL;
		TextOut_.hdr.hwndFrom =hwnd;
		TextOut_.hdr.idFrom   =id;
		SetTextOutId(&TextOut_);		
	}	
	return hResult;	
}
//-----------------------------------------------------
// Set the position to Text Out When Movie playbacks
//-----------------------------------------------------
HRESULT TextOutTime::SetPosToTextOut(LONGLONG llPos,BOOL bInitializedFlag) 
{
	HRESULT				hResult = S_OK;
	LONGLONG			pllPosToMilli;
	LONGLONG			pllPosToMilli_Min;
	LONGLONG			pllPosToMilli_Milli;
	LONGLONG			llHour;
	LONGLONG			llMinute;
	LONGLONG			llSec;
	TEXTMETRIC			Metrics;
	LONG				yPosition;
	LONG				xPosition1;
	HDC					hdc;
	WCHAR				wchTime[11];
	HWND				hParent_;
	HWND				hTextOut;
	ZeroMemory(wchTime, sizeof(wchTime));
	if(bInitializedFlag==TRUE)
	{
		pllPosToMilli		= llPos/ONE_MSEC;
		pllPosToMilli_Min	= pllPosToMilli / 1000;
		pllPosToMilli_Milli	= (pllPosToMilli % 1000)/100;		// plDurationToMilli_Milli is MilliSecond, 10 = 1 sec, therefore it is divided by 10 * 1Milli
		llHour		= pllPosToMilli_Min/3600;
		llMinute	= (pllPosToMilli_Min - (llHour * 3600)) / 60;
		llSec		= pllPosToMilli_Min	- (llHour * 3600 + llMinute * 60 );
		NMHDDISPINFO* pIdTextOut_=GetTextOutId();
		if(pIdTextOut_==NULL)
			return E_FAIL;
		hTextOut	= pIdTextOut_->hdr.hwndFrom;
		HWND hParent=GetParent(hTextOut);
		if(hParent==NULL)
			return E_FAIL;
		hParent_=hParent;
		hdc=GetDC(hTextOut);
		GetTextMetrics(hdc,&Metrics);
		yPosition =0;
		xPosition1=0;
		if((llMinute<10) && (llSec<10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:0%d:0%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		if((llMinute<10) && (llSec>=10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:0%d:%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		if((llMinute>=10) && (llSec>=10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:%d:%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		if((llMinute>=10) && (llSec<10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:%d:0%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		int iRet	 =wcslen(wchTime);
		wchTime[iRet]=0;
		SetBkColor(hdc,RGB(236,233,216));		
		BOOL bReturn=TextOut(hdc,xPosition1,yPosition,wchTime,iRet);
		ReleaseDC(hTextOut,hdc);
	}
	if(bInitializedFlag==FALSE)
	{
		llHour      		= llPos;
		llMinute			= llPos;
		llSec				= llPos;
		pllPosToMilli_Milli	= llPos;
		NMHDDISPINFO* pIdTextOut_=GetTextOutId();
		if(pIdTextOut_==NULL)
			return E_FAIL;
		hTextOut =pIdTextOut_->hdr.hwndFrom;
		HWND hParent=GetParent(hTextOut);
		if(hParent==NULL)
			return E_FAIL;
		hParent_=hParent;
		hdc=GetDC(hTextOut);
		GetTextMetrics(hdc,&Metrics);
		yPosition=0;
		xPosition1=0;
		(void)StringCchPrintf(wchTime,NUMELMS(wchTime),_TEXT("0%d:0%d:0%d:%d"),(LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);
		int iRet=wcslen( wchTime );
		wchTime[iRet]=0;
		SetBkColor(hdc,RGB(236,233,216));
		BOOL bReturn=TextOut(hdc,xPosition1,yPosition,wchTime,iRet);
		ReleaseDC(hTextOut,hdc);	
	}
	return 0;
}
//-----------------------------------------------------
// Set the id information
//-----------------------------------------------------
HRESULT TextOutTime::SetTextOutId(NMHDDISPINFO* pIdTextOut_)
{
	pIdTextOut=new NMHDDISPINFO;
	*pIdTextOut=*pIdTextOut_;
	return S_OK;
}
//-----------------------------------------------------
// Get the id information
//-----------------------------------------------------
NMHDDISPINFO* TextOutTime::GetTextOutId()
{
	return pIdTextOut;
}
void TextOutTime::SetDurationToPrivate(LONGLONG llPos)
{
	llDuration=llPos;
}
LONGLONG* TextOutTime::GetDurationOfPrivate()
{
	LONGLONG*  pllPos;
	pllPos=&llDuration;
	return pllPos;
}
//---------------------------------------------
// Duration of video is set
//---------------------------------------------
HRESULT	TextOut_Init_()
{
	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass,sizeof(WNDCLASSEX));	
	wndClass.cbSize			=sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc	=TextOutControl::TextOut_WndProc;
	wndClass.hInstance		=GetInstance();
	wndClass.lpszClassName	=TextOutControl::ClassName_;
	wndClass.cbWndExtra		=sizeof(TextOutControl::TextOut_Info );
	ATOM a=RegisterClassEx(&wndClass);
	if(a==0)
		return __HRESULT_FROM_WIN32(GetLastError());
	else
		 return S_OK;
}
//-----------------------------------------------------
// Create Window ¨Duration
//-----------------------------------------------------
HRESULT TextOutDuration::Create(HINSTANCE hInstance,HWND hParent,DWORD_PTR id,DWORD dwStyle)
{
	if(m_hwnd!=0)
		return E_FAIL;
	HRESULT hResult=S_OK; 
	RECT rc;
	rc.left=0;
	rc.top=0;
	rc.right=74;
	rc.bottom=17;

	/*
	rc.left=850;
	rc.top=4;
	rc.right=924;
	rc.bottom=21;
	*/
	HWND hwnd=CreateWindowEx(0,
							 TextOutControl::ClassName_, NULL,
							 WS_CHILD | WS_VISIBLE ,
							 rc.left,
							 rc.top,
							 rc.right-rc.left,
							 rc.bottom-rc.top,			
							 hParent,
							 (HMENU)id,
							 hInstance,
							 NULL
							);
	if(hwnd==NULL){
		hResult=E_FAIL;
		return hResult;
	}else{	
		// Object handler is set to the CWindowControl
		m_hwnd=hwnd;
		SetPosition(rc);
		// Set the Information
		NMHDDISPINFO TextOut_;
		TextOut_.hdr.code    =NULL;
		TextOut_.hdr.hwndFrom=hwnd;
		TextOut_.hdr.idFrom  =id;
		SetTextOutIdD(&TextOut_);		
	}		
	return hResult;	
}
//-----------------------------------------------------
// Set the id information
//-----------------------------------------------------
HRESULT TextOutDuration::SetTextOutIdD(NMHDDISPINFO* pIdTextOut_)
{
	pIdTextOutD= new NMHDDISPINFO;
	*pIdTextOutD=*pIdTextOut_;
	return  S_OK;
}
//-----------------------------------------------------
// Get the id information
//-----------------------------------------------------
NMHDDISPINFO* TextOutDuration::GetTextOutIdD()
{
	return pIdTextOutD;
}
void TextOutDuration::SetDurationToPrivateD(LONGLONG llPos)
{
	llDurationD=llPos;
}
LONGLONG* TextOutDuration::GetDurationOfPrivateD()
{
	LONGLONG*  pllPos;
	pllPos=&llDurationD;
	return pllPos;
}
//-----------------------------------------------------
// Set the duration to Text Out
//-----------------------------------------------------
HRESULT TextOutDuration::SetDurationToTextOut(LONGLONG llPos, BOOL bInitializedFlag)
{
	LONGLONG*		pllDuration = NULL;
	HRESULT			hResult		= S_OK;
	LONGLONG		pllPosToMilli;
	LONGLONG		pllPosToMilli_Min;
	LONGLONG		pllPosToMilli_Milli;
	LONGLONG		llHour;
	LONGLONG		llMinute;
	LONGLONG		llSec;
	TEXTMETRIC		Metrics;
	LONG			yPosition;
	LONG			xPosition1;
	HDC				hdc;
	WCHAR			wchTime[11];
	HWND			hParent_;
	HWND			hTextOut;
	ZeroMemory(wchTime,sizeof(wchTime));	
	if(bInitializedFlag==TRUE)
	{
		pllDuration = GetDurationOfPrivateD();
		if(pllDuration == NULL)
			return E_FAIL;
		llPos = *pllDuration;
		pllPosToMilli		= llPos/ONE_MSEC;
		pllPosToMilli_Min	= pllPosToMilli / 1000;
		pllPosToMilli_Milli	= (pllPosToMilli % 1000)/100;		// plDurationToMilli_Milli is MilliSecond, 10 = 1 sec, therefore it is divided by 10 * 1Milli
		llHour		= pllPosToMilli_Min/3600;
		llMinute	= (pllPosToMilli_Min - (llHour * 3600))/ 60;
		llSec		= pllPosToMilli_Min - (llHour * 3600 + llMinute * 60 );
		NMHDDISPINFO*	pIdTextOut_ = GetTextOutIdD();
		if(pIdTextOut_ == NULL)
			return E_FAIL;
		hTextOut = pIdTextOut_->hdr.hwndFrom;
		HWND hParent = GetParent(hTextOut);
		if(hParent==NULL)
			return E_FAIL;
		hParent_ = hParent;
		hdc = GetDC(hTextOut);
		GetTextMetrics(hdc,&Metrics);
		yPosition  = 0;
		xPosition1 = 0;		
		if((llMinute < 10) && (llSec < 10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:0%d:0%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		if((llMinute < 10) && (llSec >= 10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:0%d:%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		if((llMinute >=  10) && (llSec >= 10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:%d:%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		if((llMinute >= 10) && (llSec < 10))
			(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:%d:0%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		int iRet		= wcslen( wchTime );
		wchTime[iRet]	= 0;
		SetBkColor(hdc,RGB(236,233,216));
		BOOL	bReturn = TextOut(hdc,xPosition1,yPosition,wchTime,iRet);
		ReleaseDC(hTextOut,hdc);
	}
	if(bInitializedFlag == FALSE)
	{
		llHour				= llPos;
		llMinute			= llPos;
		llSec				= llPos;
		pllPosToMilli_Milli	= llPos;
		NMHDDISPINFO* pIdTextOut_=GetTextOutIdD();
		if(pIdTextOut_==NULL)
			return E_FAIL;
		hTextOut=pIdTextOut_->hdr.hwndFrom;
		HWND hParent=GetParent(hTextOut);
		if(hParent==NULL)
			return E_FAIL;
		hParent_=hParent;
		hdc=GetDC(hTextOut);
		GetTextMetrics(hdc,&Metrics);
		yPosition=0;
		xPosition1=0;
		(void)StringCchPrintf(wchTime, NUMELMS(wchTime), _TEXT("0%d:0%d:0%d:%d"), (LONG)llHour,(LONG)llMinute,(LONG)llSec,(LONG)pllPosToMilli_Milli);		
		int iRet=wcslen( wchTime );
		wchTime[iRet]=0;
		SetBkColor(hdc,RGB(236,233,216));
		BOOL bReturn=TextOut(hdc,xPosition1,yPosition,wchTime,iRet);
		ReleaseDC(hTextOut,hdc);	
	}
	return hResult;
}
void TextOutTime::SetPosition(const RECT rc)
{
	m_RectRec.left=rc.left;
	m_RectRec.top=rc.top;
	m_RectRec.right=rc.right;
	m_RectRec.bottom=rc.bottom;
}
RECT* TextOutTime::GetPosition(void)
{
	return &m_RectRec;
}
void TextOutDuration::SetPosition(const RECT rc)
{
	m_RectDur.left=rc.left;
	m_RectDur.top=rc.top;
	m_RectDur.right=rc.right;
	m_RectDur.bottom=rc.bottom;
}
RECT* TextOutDuration::GetPostition(void)
{
	return &m_RectDur;
}
void TextOutTime::NotifyPositionChange()
{
	SendMessage(WM_SIZE,0,0);
}
