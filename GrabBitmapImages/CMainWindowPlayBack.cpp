#include"stdafx.h"
#include "CMainWindowPlayBack.h"
const LONG		ONE_MSEC=10000;		// The number of 100-ns in 1 msec
const UINT_PTR	IDT_TIMER1=1;		// Timer ID
const UINT		TICK_FREQ=200;		// Timer frequency in msec	
void NotifyError(HWND hwnd,TCHAR* sMessage, HRESULT hrStatus);
void DrawBoxOutLine(HWND hwnd,POINT ptBeg,POINT ptEnd);

/* Constructor */
CMainWindowPlayBack::CMainWindowPlayBack() :
	MWbrush(NULL), 
	MWtimerID(0), 
	pMWPlayer(NULL),
	iAreaCode(0)
{
	/* Creating a new instance of CPlayBack class */
	pMWPlayer=new CPlayBack;
	fRegionSet=FALSE;
}
/* Destructor */
CMainWindowPlayBack::~CMainWindowPlayBack()
{
	if(MWbrush)
		DeleteObject(MWbrush);
	StopTimer();
	SAFE_DELETE(pMWPlayer);
}
//----------------------------------------------------------
//Method @@@
//				OnReceiveMessage(UINT uinMessage, WPARAM wParam, LPARAM lParam)
//Description @@@
//				Window Procedure → Message Handle from Wimdows System
//				
//------------------------------------------------------------
LRESULT CMainWindowPlayBack::OnReceiveMessage(UINT uinMessage,WPARAM wParam,LPARAM lParam)
{
	int iId=0;
	int iEvent=0;
	HRESULT hResult=S_OK;
	static int iCount;
	static POINT pt[MAXPOINTS];
	HDC hdc;
	int x,y;
	RECT rect;
	PAINTSTRUCT ps;
	x=0;
	y=0;
	iCount=0;
	static POINT ptBeg,ptEnd,ptBoxBeg,ptBoxEnd;
	static BOOL fBlocking;
	switch(uinMessage)
	{
		case WM_CREATE:
			hResult=Create_();
			if(FAILED(hResult)){
				NotifyError(m_hwnd,TEXT("初期エラー"),hResult);
				return -1;			
			}
			break;
		case WM_SIZE:
			LONG cx;
			LONG cy;
			cx=LOWORD(lParam);
			cy=HIWORD(lParam);
			Size_();
			break;
		/* It informs when the -part or all window's client area is INVALID and must be UPDATED*/
		/* There is a needs to be redrawn or painted */
		case WM_PAINT:
			Paint_();
			break;
		case WM_MOVE:
			Paint_();
			break;
		case WM_DISPLAYCHANGE:
			pMWPlayer->DisplayModeChanged();
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);	
			break;
		case WM_TIMER:
			Timer_();
			break;
		case WM_NOTIFY:
			iId=LOWORD(wParam);
			if(iId==IDC_LISTBOX)
				NotifyListView((NMHDR*)lParam);
			else
				NotifyW((NMHDR*)lParam);
			break;
		case WM_LBUTTONDOWN:
			/* TRUE:  the background is erased when BeginPaint function is called */
			/* FALSE: the background remains unchanged */
			/*
			ptBeg.x=LOWORD(lParam);
			ptBeg.y=HIWORD(lParam);
			if(ptBeg.x<MinSX){
				ptBeg.x=ptEnd.x=MinSX;
				ptBeg.y=ptEnd.y=HIWORD(lParam);
			}else{
				ptBeg.x=ptEnd.x=LOWORD(lParam);
				ptBeg.y=ptEnd.y=HIWORD(lParam);
			}
			*/
			InvalidateRect(m_hwnd,NULL,FALSE);
			ptBeg.x=ptEnd.x=LOWORD(lParam);
			ptBeg.y=ptEnd.y=HIWORD(lParam);
			ptRegion.left=LOWORD(lParam);
			ptRegion.top=HIWORD(lParam);
			DrawBoxOutLine(m_hwnd,ptBeg,ptEnd);
			SetCursor(LoadCursor(NULL,IDC_CROSS));
			fBlocking=TRUE;
			return 0;
			break;
		case WM_MOUSEMOVE:		
			if(fBlocking==TRUE)
			{
				SetCursor(LoadCursor(NULL,IDC_CROSS));
				DrawBoxOutLine(m_hwnd,ptBeg,ptEnd);
				ptBeg.x=LOWORD(lParam);
				ptBeg.y=HIWORD(lParam);
				DrawBoxOutLine(m_hwnd,ptBeg,ptEnd);
			}
			return 0;
			break;
		case WM_LBUTTONUP:
			if(fBlocking==TRUE)
			{
				ptEnd.x=LOWORD(lParam);
				ptEnd.y=HIWORD(lParam);
				DrawBoxOutLine(m_hwnd,ptBeg,ptEnd);
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				fBlocking=FALSE;
				fRegionSet=TRUE;
			}
			pMWPlayer->SetRegionWasSetFlag(fRegionSet);
			ptRegion.right=ptEnd.x;
			ptRegion.bottom=ptEnd.y;
			return 0;
			break;	
		/* This message is sent When mouse was clicked on outside of the client area, non client area */
		/*
		case WM_NCLBUTTONDOWN:
			return 0;
			break;
		*/
		case WM_COMMAND:
			iId=LOWORD(wParam);
			iEvent=HIWORD(wParam);
			switch(iId)
			{
				case IDM_EXIT:
					DestroyWindow(m_hwnd);
					break;
				case ID_FILE_OPENFILE:
					OpenFile_();
					break;
				case IDC_BUTTON_PLAY:
					Play_();
					break;
				case IDC_BUTTON_STOP:
					Stop_();
					break;
				case IDC_BUTTON_PAUSE:
					Pause_();
					break;
				case IDC_BUTTON_GRAB:
					GrabBitmaps_();
					break;
				case IDC_BUTTON_START:
					iAreaCode=1;
					SetGrabArea(iAreaCode);
					break;
				case IDC_BUTTON_END:
					iAreaCode=2;
					SetGrabArea(iAreaCode);
					break;
				case IDC_PROGRESSBAR:
					SetPosProgressBar();
					break;
				case IDC_BUTTON_INFO:
					break;
			}
			break;
		case WM_GRAPH_EVENT:
			// It is Window Notification
			// The Filter Graph send user-defined Windows message to this application window
			// whenever there is a new event.
			hResult=pMWPlayer->HandleGraphEvent(this);
			break;
		default:
			return CCreateWindow::OnReceiveMessage(uinMessage,wParam,lParam);
	}
	return 0;
}
//----------------------------------------------------------
//Method @@@
//				Create_()
//Description @@@
//				Create_()→ CreateWindowEx() Execution
//				Create Background, Rebar,ToolBar,Set buttons image,add buttons to Toolbar
//------------------------------------------------------------
HRESULT CMainWindowPlayBack::Create_()
{
	HRESULT hResult=S_OK;
	MWbrush=CreateHatchBrush(HS_BDIAGONAL,RGB(0xFF,0xFF,0xFF));
	if(MWbrush==NULL)
		hResult=__HRESULT_FROM_WIN32(GetLastError());
	if(SUCCEEDED(hResult))
		hResult=MWrebar.Create(m_hInstance,m_hwnd,IDC_REBAR_CONTROL);
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.Create(m_hInstance,m_hwnd,IDC_TOOLBAR,TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS);
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.SetImageList(Toolbar::Normal,			    // Image list for normal state
									   IDB_TOOLBAR_IMAGES_NORMAL,	// Bitmap resource
									   Size(48,48),					// Size of each button
									   7,							// Number of buttons
									   RGB(0xFF,0x00,0xFF)          // Color mask
									   );
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.SetImageList(Toolbar::Disabled,			// Image list for normal state
									   IDB_TOOLBAR_IMAGES_DISABLED,	// Bitmap resource
									   Size(48,48),				    // Size of each button
									   7,							// Number of buttons
									   RGB(0xFF,0x00,0xFF)		    // Color mask
									  );
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_IMAGE_PLAY,IDC_BUTTON_PLAY));
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_IMAGE_STOP,IDC_BUTTON_STOP));
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_IMAGE_PAUSE,IDC_BUTTON_PAUSE));
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_GRAB_BITMAP,IDC_BUTTON_GRAB));
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_GRABAREA_START,IDC_BUTTON_START));
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_GRABAREA_END,IDC_BUTTON_END));
	if(SUCCEEDED(hResult))
		hResult=MWtoolbar.AddButton(Toolbar::Button(ID_VIDEO_INFO,IDC_BUTTON_INFO));
	if(SUCCEEDED(hResult))
		hResult=MWrebar.AddBand(MWtoolbar.Window(),0);
	if(SUCCEEDED(hResult))
		hResult=Slider_Init();	
	if(SUCCEEDED(hResult)){
		RECT rectSlider;
		rectSlider.left=0;
		rectSlider.top=0;
		rectSlider.right=450;
		rectSlider.bottom=16;
		hResult=MWseekbar.Create(m_hwnd,Rect(rectSlider.left,rectSlider.top,rectSlider.right,rectSlider.bottom),IDC_SEEKBAR);
	}
	if(SUCCEEDED(hResult)){
		hResult=MWseekbar.SetThumbBitmap(IDB_SLIDER_THUMB);
		MWseekbar.SetBackground(CreateSolidBrush(RGB(4,50,128)));
		MWseekbar.Enable(FALSE);
	}
	if(SUCCEEDED(hResult))
		hResult=MWrebar.AddBand(MWseekbar.Window(),1);
	if(SUCCEEDED(hResult))
		hResult=TextOut_Init();
	if(SUCCEEDED(hResult))
		hResult=MWTextOut.Create(m_hInstance,m_hwnd,IDC_TEXTOUTT,NULL); 
	if(SUCCEEDED(hResult))
		hResult=MWTextOut.SetPosToTextOut(0,FALSE);
	if(SUCCEEDED(hResult))
		hResult=MWrebar.AddBand(MWTextOut.Window(),1);
	if(SUCCEEDED(hResult))
		hResult=TextOut_Init_();
	if(SUCCEEDED(hResult))
		hResult=MWTextOutDuration.Create(m_hInstance,m_hwnd,IDC_TEXTOUTD,NULL); 
	if(SUCCEEDED(hResult))
		hResult=MWrebar.AddBand(MWTextOutDuration.Window(),1);
	if(SUCCEEDED(hResult))
		hResult=MWTextOutDuration.SetDurationToTextOut(0,FALSE);
	if(SUCCEEDED(hResult))
		hResult=MWProgressBar.Create(m_hInstance,m_hwnd,IDC_PROGRESSBAR,NULL);
	if(SUCCEEDED(hResult))
		hResult=MWrebar.AddBand(MWProgressBar.Window(),1);
    if(SUCCEEDED(hResult))
        hResult=MWtoolTip.Create(m_hwnd);
	if(SUCCEEDED(hResult)){
        MWtoolTip.AddTool(MWseekbar.Window(),L"Seek");
        MWrebar.SendMessage(RB_SETTOOLTIPS,(WPARAM)MWtoolTip.Window(),0);
    }
	if(SUCCEEDED(hResult)){
		pMWPlayer=new CPlayBack(m_hwnd);
		if(pMWPlayer==NULL)
			hResult=E_OUTOFMEMORY;
	}
	if(SUCCEEDED(hResult))
		hResult=pMWPlayer->SetEventWindow(m_hwnd,WM_GRAPH_EVENT);
	if(SUCCEEDED(hResult))
		UpdateUI();
	return hResult;
}
//--------------------------------
// Method @@@
//				Paint_()
// Description @@@
//				Window Paint
//--------------------------------
void CMainWindowPlayBack::Paint_()
{
	PAINTSTRUCT pStruct;
	HDC hdc;
	hdc=BeginPaint(m_hwnd,&pStruct);
	if(pMWPlayer->State()!=STATE_CLOSED && pMWPlayer->HasVideo()){
		pMWPlayer->Repaint(hdc);
	}else{
		RECT rectClient;
		RECT rectToolbar;
		GetClientRect(m_hwnd,&rectClient);
		GetClientRect(MWrebar.Window(),&rectToolbar);
		HRGN hRgnA_=CreateRectRgnIndirect(&rectClient);
		HRGN hRgnB_=CreateRectRgnIndirect(&rectToolbar);
		CombineRgn(hRgnA_,hRgnA_,hRgnB_,RGN_DIFF);
		FillRgn(hdc,hRgnA_,MWbrush);
		DeleteObject(hRgnA_);
		DeleteObject(hRgnB_);
	}	
	EndPaint(m_hwnd, &pStruct);
}
//--------------------------------
// Method @@@
//				Size_()
// Description @@@
//				Window Resize
//--------------------------------
void CMainWindowPlayBack::Size_()
{
	SendMessage(MWtoolbar.Window(),WM_SIZE,0,0);
	SendMessage(MWrebar.Window(),WM_SIZE,0,0);
	SendMessage(MWProgressBar.Window(),WM_SIZE,0,0);
	MWTextOut.NotifyPositionChange();
	RECT rectWindow;
	RECT rectControl;
	GetClientRect(m_hwnd,&rectWindow);
	GetClientRect(MWrebar.Window(),&rectControl);
	SubtractRect(&rectWindow,&rectWindow,&rectControl);
	if(rectWindow.right<ptDstVideo.right)
		rectWindow.right=ptDstVideo.right;
	if(rectWindow.bottom<ptDstVideo.bottom)
		rectWindow.bottom=ptDstVideo.bottom;
	//SetWindowPlacement();
	pMWPlayer->UpdateVideoWindow(&rectWindow);
	RECT rectSlider;
	BOOL bRet=GetClientRect(MWseekbar.Window(),&rectSlider);
	if(!bRet)
		bRet=false;
}
//------------------------------------------------------------------------
// Method @@@
//				Timer_()
// Description @@@
//				When the timer elapses.It is timer related to video play
//------------------------------------------------------------------------
void CMainWindowPlayBack::Timer_()
{
	if(pMWPlayer->CanSeek()){
        REFERENCE_TIME rTimeNow;
        if(SUCCEEDED(pMWPlayer->GetCurrentPosition(&rTimeNow))){
			MWseekbar.SetPosition((LONG)(rTimeNow/ONE_MSEC));
			if((pMWPlayer->State()==STATE_RUNNING)||(pMWPlayer->State()==STATE_PAUSED) ){
				MWTextOut.SetPosToTextOut(rTimeNow,TRUE);				
			}else{
				MWTextOutDuration.SetDurationToTextOut(rTimeNow,TRUE);
				MWTextOut.SetPosToTextOut(rTimeNow,TRUE);
			}			
        }
    }
}
//--------------------------------------------------------------------------------
// Method @@@
//				NotifyW(const NMHDR *pHdr)
// Description @@@
//				WM_NOTIFY Message (Handle the NotifyW message)
//				Sent by a common control to its Parent window 
//				when an event has occurred or the control requires some information. 
//---------------------------------------------------------------------------------
void CMainWindowPlayBack::NotifyW(const NMHDR* pHdr)
{
	switch(pHdr->code)
    {
		case TTN_GETDISPINFO:			
			MWtoolbar.ShowToolTip((NMTTDISPINFO*)pHdr);           
			break;
		default:
			switch(pHdr->idFrom)
			{
				case IDC_SEEKBAR:
					NotifySeekBar((NMSLIDER_INFO*)pHdr);
					break;
			}
			break;
	}
}
void CMainWindowPlayBack::NotifyListView(const NMHDR* pInfo)
{
	// It can be after
}
//---------------------------------------------------------
// Method @@@
//				NotifySeekBar(const NMSLIDER_INFO *pInfo)
// Description @@@
//				Handle the WM_NOTIFY message from seekbar
//---------------------------------------------------------
void CMainWindowPlayBack::NotifySeekBar(const NMSLIDER_INFO* pInfo)
{
	static CPlayBackState pbState=STATE_CLOSED;
	HRESULT hResult=S_OK;		
	if(pInfo->hdr.code==SLIDER_NOTIFY_SELECT){
		pbState=pMWPlayer->State();
		pMWPlayer->Pause();
	}
	REFERENCE_TIME rTime=((REFERENCE_TIME)ONE_MSEC) * ((REFERENCE_TIME)pInfo->position);
	hResult=pMWPlayer->SetPosition(rTime);
	if(SUCCEEDED(hResult)){
		LONGLONG pCurrentTime=0;
		hResult=pMWPlayer->GetCurrentPosition(&pCurrentTime);	
	}		
	if(pInfo->hdr.code==SLIDER_NOTIFY_RELEASE){
		if(pbState==STATE_STOPPED){
			pMWPlayer->Stop();
		}
		else if(pbState==STATE_RUNNING){
			pMWPlayer->Play();
		}
	}
}
//---------------------------------------------------------
// Method @@@
//					OpenFile_() 
// Description @@@
//					File will be opened and sent to CPlayBack
//---------------------------------------------------------
void CMainWindowPlayBack::OpenFile_() 
{
	OPENFILENAME FileOpen;
	WCHAR szFileName[MAX_PATH];
	HRESULT hResult;
	szFileName[0]=L'\0';
	ZeroMemory(&FileOpen,sizeof(FileOpen));
	FileOpen.lStructSize=sizeof(FileOpen);
	FileOpen.hwndOwner=m_hwnd;
	FileOpen.hInstance=m_hInstance;
	FileOpen.lpstrFilter=L"All (*.*)/0*.*/0";
	FileOpen.lpstrFile=szFileName;
	FileOpen.nMaxFile=MAX_PATH;
	FileOpen.Flags=OFN_FILEMUSTEXIST;
	if(GetOpenFileName(&FileOpen))
	{
		/* Send file to CPlayBack */
		hResult=pMWPlayer->OpenFile(szFileName);
		UpdateUI();
		/* Invalidate the appliction window: Old video or  No video Now, It is Failed */
		InvalidateRect(m_hwnd,NULL,FALSE);
		/* Update the seek bar to match the current state.*/
		UpdateSeekBar();
		if(SUCCEEDED(hResult))
			/* If File has a video stream, Notification to VMR: Video Size to the Destination */
			Size_();
		else
			/* Error notification */
			NotifyError(m_hwnd,TEXT("ファイルを開けませんでした。"),hResult);
		RECT rectSrc;
		Rect rectDst;
		pMWPlayer->GetVideoRect(&rectSrc,&rectDst);
		SetVideoRect(&rectSrc,&rectDst);
	}	
}
//---------------------------------------------------------
// Method @@@
//					SetGrabArea(int iSetArea)
// Description @@@
//					Grabbing Area is set 
//---------------------------------------------------------
int CMainWindowPlayBack::SetGrabArea(int iSetArea)
{
	HRESULT	hResult=pMWPlayer->SetGrabTime(&iSetArea);
	return hResult;
}
//---------------------------------------------------------
// Method @@@
//					GrabBitmaps_() 
// Description @@@
//					Grabbing Bitmaps from AVI file , 
//					write to file and save the files
//---------------------------------------------------------
void CMainWindowPlayBack::GrabBitmaps_()
{
	HWND hParent=this->m_hwnd;
	pMWPlayer->SetParentWnd(hParent);
	HRESULT hResult=pMWPlayer->GrabBitmaps();	
	UpdateUI();
}
//---------------------------------------------------------
// Method @@@
//					Play_()
// Description @@@
//					Playback the AVI 
//---------------------------------------------------------
void CMainWindowPlayBack::Play_()
{	
	HRESULT hResult=pMWPlayer->Play();
	UpdateUI();
}
//---------------------------------------------------------
// Method @@@
//					Stop_()
// Description @@@
//					Stop the AVI 
//---------------------------------------------------------
void CMainWindowPlayBack::Stop_()
{	
	HRESULT	hResult=pMWPlayer->Stop();
	if(SUCCEEDED(hResult)){
		if(pMWPlayer->CanSeek()){
			hResult=pMWPlayer->SetPosition(0);
		}
	}
	UpdateUI();
}
//---------------------------------------------------------
// Method @@@
//					Pause_()
// Description @@@
//					Pause the AVI 
//---------------------------------------------------------
void CMainWindowPlayBack::Pause_() 
{
	HRESULT hResult=pMWPlayer->Pause();
	UpdateUI();
}
//-------------------------------------------------------------------------------------
// Method @@@
//					OnGraphEvent(long lEventCode, LONG_PTR lptrParam, LONG_PTR lptrParam_)
// Description @@@
//					Call back the handle events from Filter Graph 
//-------------------------------------------------------------------------------------
void CMainWindowPlayBack::OnGraphEvent(long lEventCode,LONG_PTR lptrParam,LONG_PTR lptrParam_)
{
	switch(lEventCode){
		case EC_COMPLETE:
			Stop_();
			break;
	}
}
//---------------------------------------------------------
// Method @@@
//					UpdateUI()
// Description @@@
//					Update the UI based on the current CPlayBack state. 
//---------------------------------------------------------
void CMainWindowPlayBack::UpdateUI()
{
	BOOL bPlay=FALSE;
	BOOL bPause=FALSE;
	BOOL bStop=FALSE;
	BOOL bGrabBitmap= FALSE;
	BOOL bGrabAreaStart=FALSE;
	BOOL bGrabAreaEnd=FALSE;
	switch(pMWPlayer->State())
	{
		case STATE_RUNNING:
			bPause=TRUE;
			bStop=TRUE;
			bGrabBitmap=TRUE;
			bGrabAreaStart=TRUE;
			bGrabAreaEnd=TRUE;
			break;
		case STATE_PAUSED:
			bPlay=TRUE;
			bStop=TRUE;
			bGrabBitmap=TRUE;
			bGrabAreaStart=TRUE;
			bGrabAreaEnd=TRUE;
			break;
		case STATE_STOPPED:
			bPlay=TRUE;
			bGrabBitmap=TRUE;
			bGrabAreaStart=TRUE;
			bGrabAreaEnd=TRUE;
			break;	
		case STATE_GRABBING:
			bPlay=TRUE;
			bPause=TRUE;
			bStop=TRUE;
			bGrabAreaStart=FALSE;
			bGrabAreaEnd=FALSE;
			break;
		case STATE_SETTING_AREA:
			bPlay=FALSE;
			bPause=FALSE;
			bStop=FALSE;
			bGrabAreaStart=TRUE;
			bGrabAreaEnd=TRUE;	
			break;	
	}
	MWtoolbar.Enable(IDC_BUTTON_PLAY,bPlay);
	MWtoolbar.Enable(IDC_BUTTON_PAUSE,bPause);
	MWtoolbar.Enable(IDC_BUTTON_STOP,bStop);
	MWtoolbar.Enable(IDC_BUTTON_GRAB,bGrabBitmap);
	MWtoolbar.Enable(IDC_BUTTON_START,bGrabAreaStart);
	MWtoolbar.Enable(IDC_BUTTON_END,bGrabAreaEnd);
}
//---------------------------------------------------------
// Method @@@
//					UpdateSeekBar() 
// Description @@@
//					Update SeekBar
//---------------------------------------------------------
void CMainWindowPlayBack::UpdateSeekBar() 
{
	/* If the player can seek, set the seekbar range and start the time.*/
    /* Otherwise, disable the seekbar.*/
	if(pMWPlayer->CanSeek())
	{
		MWseekbar.Enable(TRUE);
		LONGLONG llDuration=0;
		pMWPlayer->GetDuration(&llDuration);
		MWTextOutDuration.SetDurationToPrivateD(llDuration);
		MWTextOutDuration.SetDurationToTextOut(llDuration,TRUE);
		MWTextOut.SetPosToTextOut(0,FALSE);
		MWseekbar.SetRange(0,(LONG)(llDuration/(LONGLONG)ONE_MSEC));
		/*Start the timer */
		MWtimerID=SetTimer(m_hwnd,IDT_TIMER1,TICK_FREQ,NULL);
	}else{
		MWseekbar.Enable(TRUE);
		StopTimer();
	}
}
//---------------------------------------------------------
// Method @@@
//					StopTimer()
// Description @@@
//					Stop Timer
//---------------------------------------------------------
void CMainWindowPlayBack::StopTimer() 
{
	if(MWtimerID!=0){
		KillTimer(m_hwnd,MWtimerID);
		MWtimerID=0;
	}
}
void CMainWindowPlayBack::SetPosProgressBar(void)
{
	int iProgressBarPos=pMWPlayer->GetGrabRatio();
	HWND h_wnd;
	MWProgressBar.GetWnd(&h_wnd);
	//LRESULT lResult = SendMessageA(h_wnd,PBM_SETPOS,(WPARAM)1,(LPARAM)iProgressBarPos);
	LRESULT lResult = SendMessageA(h_wnd,PBM_STEPIT,(LPARAM)iProgressBarPos,(WPARAM)1);
	if(lResult==0)
		lResult=1;
}
void CMainWindowPlayBack::SetVideoRect(RECT* rectSrc, RECT* rectDst)
{
	ptSrcVideo=*rectSrc;
	ptDstVideo=*rectDst;
	CalculateRegMinMax();
}
void CMainWindowPlayBack::CalculateRegMinMax()
{
	LONG SrcCenterX,SrcCenterY,DstCenterX,DstCenterY;

	SrcCenterX=(ptSrcVideo.right-ptSrcVideo.left)/2;
	SrcCenterY=(ptSrcVideo.bottom-ptSrcVideo.top)/2;
	DstCenterX=(ptDstVideo.right-ptDstVideo.left)/2;
	DstCenterY=(ptDstVideo.bottom-ptDstVideo.top)/2;
	if(SrcCenterX<DstCenterY){
		MinSX=ptDstVideo.left+(DstCenterX-SrcCenterX);
		MinEX=ptDstVideo.top-(DstCenterX-SrcCenterX);
	}else{
		MinSX=ptDstVideo.left+(SrcCenterX-DstCenterX);
		MinEX=ptDstVideo.top+(SrcCenterX-DstCenterX);
	}
}
//---------------------------------------------------------
// Method @@@
//					NotifyError(HWND hwnd, TCHAR* sMessage, HRESULT hrStatus)
// Description @@@
//					Notify any error
//---------------------------------------------------------
void NotifyError(HWND hwnd,TCHAR* sMessage,HRESULT hrStatus)
{
	TCHAR sTmp[512];
	HRESULT hResult=StringCchPrintf(sTmp,512,TEXT("%s hr = 0x%X"),sMessage,hrStatus);
	if(SUCCEEDED(hResult))
		MessageBox(hwnd,sTmp,TEXT("Error"),MB_OK | MB_ICONERROR);
}
void DrawBoxOutLine(HWND hwnd,POINT ptBeg,POINT ptEnd)
{
	HDC hdc;
	hdc=GetDC(hwnd);
	SetROP2(hdc,R2_NOT);
	SelectObject(hdc,GetStockObject(NULL_BRUSH));
	Rectangle(hdc,ptBeg.x,ptBeg.y,ptEnd.x,ptEnd.y);
}