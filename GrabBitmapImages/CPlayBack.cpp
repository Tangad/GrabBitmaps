#include "stdafx.h"
#include "CPlayBack.h"
#include "DshowUtil.h"
#include "GrabBitmaps.h"
CAVIFileInfo			AVIFileInfoObj	;
CGrabBitmaps			GB				;
static REFERENCE_TIME	rKeepStartCurrentTime		= 0;
static REFERENCE_TIME	rKeepEndCurrentTime			= 0;
const					LONGLONG NANOSECONDS		= (1000000000);
const					LONGLONG UNITS				= (NANOSECONDS/100); 
/* Remove the renderer that couldn`t be connected */
HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, BOOL *pbRemoved);
/* WINDOWLESS MODE : we don`t use DirectShow`s Video Window */
HRESULT InitWindowlessVMR(IBaseFilter *pVMR, HWND hwnd, IVMRWindowlessControl** ppWc); 
/* Constructor(Overloaded) */
//--------------------------
CPlayBack::CPlayBack()
{		
}
CPlayBack::CPlayBack(HWND hwndVideo_) :
	pbState(STATE_CLOSED),
	hwndVideo(hwndVideo_),
	hwndEvent(NULL),
	uiEventMsg(0),
	pGraphBuilder(NULL),
	pMediaControl(NULL),
	pMediaEvent(NULL),
	pMediaSeek(NULL),
	pWindowlessControl(NULL),
	pBasicAudio(NULL),
	dwSeekCaps(0),
	pBaseFilter(NULL),
	pVideoInfoHeader(NULL),
	bStartArea(FALSE),
	bEndArea(FALSE),
	m_iSaveGrabRatio(0)
{
	ZeroMemory(&MediaType,sizeof(AM_MEDIA_TYPE));
	MediaType.majortype=MEDIATYPE_Video;
	MediaType.subtype=MEDIASUBTYPE_RGB24;
	MediaType.formattype=FORMAT_VideoInfo;
	tchFileName[0]=L'\0';
	iKeepFileNumber=-1;
	fRegionSet_=FALSE;
	ZeroMemory(&rectSrcVideo,sizeof(RECT));
	ZeroMemory(&rectDstVideo,sizeof(RECT));
}
/* Destructor */
CPlayBack::~CPlayBack()
{
	m_iSaveGrabRatio=0;
	TearDownGraph();
}
//----------------------------------------------------------------------------
// Method @@@
//				SetEventWindow(HWND hwnd, UINT uiMessage)
//			    This function registers a parent window to process event notification
// Description @@@
//				Window to receive the events.
//				Private window message that window will receive whenever a 
//				graph event occurs. (Must be in the range WM_APP through 0xBFFF.)
//----------------------------------------------------------------------------
HRESULT CPlayBack::SetEventWindow(HWND hwnd,UINT uiMessage)
{
	hwndEvent=hwnd;
	uiEventMsg=uiMessage;
	return S_OK;
}
//----------------------------------------------------------
// Method @@@
//				OpenFile(const WCHAR* sFileName)
// Description @@@
//				Open a new file for CPlayBack.
//				
//------------------------------------------------------------
HRESULT CPlayBack::OpenFile(const WCHAR* sFileName)
{
	HRESULT hResult=S_OK;
	IBaseFilter* pSource=NULL;
	/* File with Parent Directory is copied  to Public Variable */
	wcsncpy_s(tchFileName,MAX_PATH-1,sFileName,MAX_PATH-1);
	/* Set AVI File Information */
	/*hResult = pAVIFileInfo->AVIFileProcessing(tchFileName);*/
	hResult=AVIFileInfoObj.AVIFileProcessing(tchFileName);
	if(SUCCEEDED(hResult))
	{
		/* Create a new Filter Graph. (This also closes the old one, if any.)*/
		/*const WCHAR			*pGetDirectoryPath; */
		/*pGetDirectoryPath = pAVIFileInfo->GetDirectory();*/
		hResult=InitializeGraph();
	}else{		
		return hResult;
	}
	/* Add the File Source Filter to the Graph Builder */
	if(SUCCEEDED(hResult))
		hResult=pGraphBuilder->AddSourceFilter(sFileName,NULL,&pSource);
	/* Execute Render Streams */
	if(SUCCEEDED(hResult))
		hResult=RenderStreams(pSource);
	/* Get the seeking capabilities. */
	if(SUCCEEDED(hResult))
		hResult=pMediaSeek->GetCapabilities(&dwSeekCaps);
	/* Update our state. */
	if(SUCCEEDED(hResult))
		pbState=STATE_STOPPED;
	SAFE_RELEASE(pSource);
	InitializeFlags();
	LONGLONG llCurrentInitialTime;
	hResult=GetCurrentPosition(&llCurrentInitialTime);
	if(SUCCEEDED(hResult)){
		LONGLONG llInitialDuration;
		hResult=GetDuration(&llInitialDuration);	
	}
	return hResult;
}
void CPlayBack::InitializeFlags()
{
	rKeepStartCurrentTime=0;
	rKeepEndCurrentTime=0;
	rCurrentTime=0;
	rStartTime=0;
	rEndTime=0;
	rDuration=0;
	bStartArea=FALSE;
	bEndArea=FALSE;	
}
//----------------------------------------------------------
// Method @@@
//				HandleGraphEvent(GraphEventCallback *pCB)
//				pCB: Pointer to the GraphEventCallback callback, implemented by 
//				the application. This callback is invoked once for each event
//				in the queue.
// Description @@@
//				Respond to a graph event.
//				The owning window should call this method when it receives the window
//				message that the application specified when it called SetEventWindow.
// 
// Caution: Do not tear down the graph from inside the callback.
//				
//------------------------------------------------------------
HRESULT CPlayBack::HandleGraphEvent(GraphEventCallback *pCB)
{
	if(pCB==NULL)
		return E_POINTER;
	if(!pMediaEvent)
		return E_UNEXPECTED;
	long lEventCode	= 0;
	LONG_PTR param1 = 0;
	LONG_PTR param2	= 0;
	HRESULT hResult	= S_OK;
	/* Get the events from the queue. */
	while(SUCCEEDED(pMediaEvent->GetEvent(&lEventCode,&param1,&param2,0)))
	{
		/* Invoke the callback. */
		pCB->OnGraphEvent(lEventCode,param1,param2);
		/* Free the event data. */
		hResult=pMediaEvent->FreeEventParams(lEventCode,param1,param2);
		if(FAILED(hResult))
			break;
	}
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				SetGrabTime()
// Description @@@
//				Set grabbing time (time will be set.)
//				
//------------------------------------------------------------
HRESULT	CPlayBack::SetGrabTime(int* pSelectAreaCode)
{
	LONGLONG llCurrentTime;
	HRESULT hResult=S_OK;
	if(pSelectAreaCode==NULL){
		hResult=E_POINTER;
		return hResult;
	}
	hResult=GetCurrentPosition(&llCurrentTime);
	switch(*pSelectAreaCode)
	{
		case 1:
			if(SUCCEEDED(hResult)){
				rCurrentTime=(REFERENCE_TIME)llCurrentTime;
				rStartTime=(REFERENCE_TIME)llCurrentTime;
				bStartArea=TRUE;
			}
			break;
		case 2:
			if(SUCCEEDED(hResult)){
				rCurrentTime=(REFERENCE_TIME)llCurrentTime;
				rEndTime=(REFERENCE_TIME)llCurrentTime;
				bEndArea=TRUE;
			}
			break;
		default:
			break;
	}
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				GrabBitmaps()
// Description @@@
//				Grabbing bitmaps from video stream in AVI
//				
//------------------------------------------------------------
HRESULT CPlayBack::GrabBitmaps()
{
	IGraphBuilder* pGraph=NULL;
	ISampleGrabber* pSampleGrabber=NULL;
	IBaseFilter* pGrabberBase=NULL;
	IBaseFilter* pSource=NULL;
	IFileSourceFilter* pFileSourceFilter=NULL;
	IFileSourceFilter* pLoad=NULL;
	IPin* pSourcePin=NULL;
	IPin* pGrabPin=NULL;
	HRESULT hResult=S_OK;
	/* Assess the Graph Builder`s Existence */
	assert(pGraphBuilder);	
	/*Create the Sample Grabber Object*/
	if(SUCCEEDED(hResult))
		hResult=CoCreateInstance(CLSID_SampleGrabber,NULL,CLSCTX_INPROC_SERVER,IID_ISampleGrabber,(void**)&pSampleGrabber);
	/* Query the Interface for pGrabberBase filter */
	if(SUCCEEDED(hResult))
		hResult=pSampleGrabber->QueryInterface(IID_IBaseFilter,(void**)&pGrabberBase);
	/* Create the File Reader Filter Interface */
	if(SUCCEEDED(hResult))
		hResult=CoCreateInstance(CLSID_AsyncReader,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**)&pSource);
	/* Create Graph Builder Instance */
	if(SUCCEEDED(hResult))
		hResult=CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&pGraph);
	/* Put Source Filter in to the Graph Builder */
	if(SUCCEEDED(hResult))
		hResult=pGraph->AddFilter(pSource,L"Source");
	/* Put Grabber Filter in to the Graph Builder */
	if(SUCCEEDED(hResult))
		hResult=pGraph->AddFilter(pGrabberBase,L"Grabber");
	/* Query the Source Load Filter */
	if(SUCCEEDED(hResult))
		hResult=pSource->QueryInterface(IID_IFileSourceFilter,(void**)&pLoad);
	/* Load Source (AVI file) */
	if(SUCCEEDED(hResult))
		hResult=pLoad->Load(tchFileName,NULL);
	/* Set the Media type to the Sample Grabber */
	if(SUCCEEDED(hResult))
		hResult=pSampleGrabber->SetMediaType(&MediaType);
	/* Connect Output Pin and Input Pin */
	pSourcePin = GetOutPin(pSource,0);
	pGrabPin = GetInPin(pGrabberBase,0);
	hResult	= pGraph->Connect(pSourcePin,pGrabPin);
	/* Get the Media Type */
	if(SUCCEEDED(hResult))
		hResult=pSampleGrabber->GetConnectedMediaType(&MediaType);
	/* Obtain Video Information Header */
	pVideoInfoHeader=(VIDEOINFOHEADER*)MediaType.pbFormat;
	/* Get bitmap width and height */
	GB.lWidth=pVideoInfoHeader->bmiHeader.biWidth;
	GB.lHeight=pVideoInfoHeader->bmiHeader.biHeight;
	/* Free the Media Type */
	FreeMediaType(MediaType);
	IPin* pGrabOutPin;
	pGrabOutPin=GetOutPin(pGrabberBase,0);
	if(SUCCEEDED(hResult))
		hResult=pGraph->Render(pGrabOutPin);
	if(SUCCEEDED(hResult))
		hResult=pSampleGrabber->SetOneShot(TRUE);
	/*　SetCallBack method in DirectShow → this is a callback method to call on incoming samples.　*/
	/*　it calls the BufferCB() from the ISampleGrabberCB interface which includes BufferCB method.　*/
	if(SUCCEEDED(hResult))
		hResult=pSampleGrabber->SetCallback(&GB,1);
	/* Query the Media Seek object and Create the Interface */
	 IMediaSeeking* pSeeking=NULL;
	 if(SUCCEEDED(hResult))
		hResult=pGraph->QueryInterface(IID_IMediaSeeking,(void**)&pSeeking);
	/* Query the Video Window object and Create the Interface */ 
	 IVideoWindow* pWindow;	
	if(SUCCEEDED(hResult))
		hResult=pGraph->QueryInterface(IID_IVideoWindow,(void**)&pWindow);
	/* Disable the Video Window */
	if(SUCCEEDED(hResult))
		hResult=pWindow->put_AutoShow(OAFALSE);
	/* Query the Media Control object and Create the Interface */
	IMediaControl* pControl;
	if(SUCCEEDED(hResult))
		hResult=pGraph->QueryInterface(IID_IMediaControl,(void**)&pControl);
	/* Query the Media Event object and create the Interface */
	IMediaEvent* pEvent;   
	if(SUCCEEDED(hResult))
		hResult=pGraph->QueryInterface(IID_IMediaEvent,(void**)&pEvent);
	/* for Optimization */
	DWORD value1;
	DWORD value2;
	DWORD Dif;
	if(SUCCEEDED(hResult)){	
		int iFileNumber=0;
		int	iRet=AVIFileInfoObj.GetCurrentFileNumber(&iFileNumber);
		int	iRate=(int)AVIFileInfoObj.FileData[iFileNumber].lRate  ;
		int	iScale=(int)AVIFileInfoObj.FileData[iFileNumber].lScale ;	
		REFERENCE_TIME rStartTime_;
		REFERENCE_TIME rEndTime_;
		REFERENCE_TIME rGrabBitmapTime;			
		if(bStartArea==TRUE)
		{
			if(bEndArea==TRUE)
			{
				/* Get the size of time to grabbing bitmaps each unit of  REFERENCE_TIME  is 100 nanoseconds */
				rStartTime_=rStartTime;
				rEndTime_=rEndTime;
				if(rEndTime_==0)
					rEndTime_=rDuration;
				rGrabBitmapTime=rEndTime_-rStartTime_;
			}else{				
				rStartTime_=rStartTime;
				rEndTime_=rDuration;
				rGrabBitmapTime=rEndTime_-rStartTime_;			
			}	
		}else{
			if(bEndArea==TRUE)
			{
				rStartTime_=rCurrentTime;
				rEndTime_=rEndTime;
				rGrabBitmapTime=rEndTime_-rStartTime_ ;
			}else{
				rStartTime_=rCurrentTime;
				rEndTime_=rDuration;
				rGrabBitmapTime=rEndTime_-rStartTime_;
			}		
		}
		/* Set Bitmap Region */
		GB.SetSizeOfBitmap(&rectBitmap);
		/* Grabbing duration for the bitmap frames  */ 
		double dGrabBitmapTimeSec=ceil((double)(rGrabBitmapTime * 100)/NANOSECONDS);
		int iFramesPerSecond=iRate/iScale;	 
		LONGLONG llAccelerationTime=UNITS/(LONGLONG)iFramesPerSecond;
		REFERENCE_TIME rKeepStartTime=0;
		int iKeepFramesPerSecond=0;
		int iGrabBitmapTimeSec=(int)dGrabBitmapTimeSec;
		iKeepFramesPerSecond=iFramesPerSecond;			
		value1=GetTickCount();
		int iTotalFramesGrabbed=iGrabBitmapTimeSec*iFramesPerSecond; 
		double dfSaveIncrement=100.0/iTotalFramesGrabbed;
		double dfSaveRatio=0.0;
		m_iSaveGrabRatio=int(dfSaveRatio+0.500005);
		for(int ii=0;ii<iGrabBitmapTimeSec;ii++)
		{	
			rKeepStartTime=rStartTime_;
			for(int jj=0;jj<iFramesPerSecond+1;jj++)
			{
				rStartTime_=rKeepStartTime+jj*llAccelerationTime;
				hResult=pSeeking->SetPositions(&rStartTime_,AM_SEEKING_AbsolutePositioning ,&rEndTime_,AM_SEEKING_NoPositioning );
				
				hResult=pControl->Run();
				long lEventCode=0;
				pEvent->WaitForCompletion(INFINITE, &lEventCode);
				dfSaveRatio+=dfSaveIncrement;
				m_iSaveGrabRatio=(int)(dfSaveRatio+0.500005);
				SendMessage(Parent_hwnd,WM_COMMAND,108,0);
			}
		}
		value2=GetTickCount();
		/* Grabbing Bitmap was executed → Set True */
		AVIFileInfoObj.FileData[iFileNumber].bGrabBitmapFlag=TRUE;
		/* Remove the Grab Area set flag */
		bStartArea=FALSE;
		bEndArea=FALSE;
		rCurrentTime=0;
		rStartTime=0;
		rEndTime=0;		
	}	
	/* For Optimization */
	Dif=value2-value1;	
	FILE* fp;
	fopen_s(&fp,"Temp.txt","w");
	fprintf(fp,"%d",Dif);
	fclose(fp);	
	/* For Event Notification */
	if(SUCCEEDED(hResult))
		hResult=pMediaEvent->SetNotifyWindow((OAHWND)hwndEvent,uiEventMsg,NULL);
	/* Release the Pointer to the DirectShow Interface */
	SAFE_RELEASE(pGraph);
	SAFE_RELEASE(pSampleGrabber);
	SAFE_RELEASE(pGrabberBase);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pFileSourceFilter);
	SAFE_RELEASE(pLoad);
	SAFE_RELEASE(pSourcePin);
	SAFE_RELEASE(pGrabPin);
	SAFE_RELEASE(pGrabOutPin);
	SAFE_RELEASE(pSeeking);
	SAFE_RELEASE(pWindow);
	SAFE_RELEASE(pControl);
	SAFE_RELEASE(pEvent);
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				Play()
// Description @@@
//				Implement MediaControl and run video
//------------------------------------------------------------
HRESULT CPlayBack::Play()
{
	/* Check Video is stopped or paused */
	if (pbState!=STATE_PAUSED && pbState!=STATE_STOPPED)
		return VFW_E_WRONG_STATE;
	/* Evaluate/Assess Graph Builder Existence */
	assert(pGraphBuilder); 
	/* IMediaControl :: Play It means all the filters run in the FilterGraph */
	HRESULT hResult=pMediaControl->Run();
	if(SUCCEEDED(hResult)){
		pbState=STATE_RUNNING;
		/*
		RECT rectPosSrc;
		RECT rectPosDst;
		pWindowlessControl->GetVideoPosition(&rectPosSrc,&rectPosDst);
		LONG lWidth;
		LONG lHeight;
		pWindowlessControl->GetMaxIdealVideoSize(&lWidth,&lHeight);
		pWindowlessControl->GetMinIdealVideoSize(&lWidth,&lHeight);
		DWORD dwAspectRatio;
		pWindowlessControl->GetAspectRatioMode(&dwAspectRatio);
		*/		
	}
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				Pause()
// Description @@@
//				Implement MediaControl and pause video
//				
//------------------------------------------------------------
HRESULT CPlayBack::Pause()
{
	/* Check Video is Playing */
	if (pbState!=STATE_RUNNING)
		return VFW_E_WRONG_STATE;
	/* Evaluate/Assess Graph Builder Existence */
	assert(pGraphBuilder); 
	HRESULT hResult=pMediaControl->Pause();
	if(SUCCEEDED(hResult))
		pbState=STATE_PAUSED;
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				Stop()
// Description @@@
//				Implement MediaControl and stop video
//				
//------------------------------------------------------------
HRESULT CPlayBack::Stop()
{
	/* Check Video is run or paused */
	if (pbState!=STATE_RUNNING && pbState!=STATE_PAUSED)
		return VFW_E_WRONG_STATE;
	HRESULT hResult=pMediaControl->Stop();
	if(SUCCEEDED(hResult))
		pbState=STATE_STOPPED;
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				UpdateVideoWindow(const LPRECT prc)
// Description @@@
//				Sets the destination rectangle for the video.
//				Related to IVMRWindowlessControl Interface 
//				
//------------------------------------------------------------
HRESULT CPlayBack::UpdateVideoWindow(const LPRECT prc)
{
	/* Windowless Control :: IVMRWindowlessControl Interface */
	if(pWindowlessControl==NULL)
		return S_OK; 
	if(prc){
		/* Getting the destination window size */
		rectDstVideo=*prc;
		/* Get the native video size */
		LONG lWidth=0;
		LONG lHeight=0;
		LONG lWidth_=0;
		LONG lHeight_=0;
		pWindowlessControl->GetNativeVideoSize(&lWidth,&lHeight,&lWidth_,&lHeight_);
		/* Calculate the coordinates of the source video */
		rectSrcVideo.left=0;
		rectSrcVideo.top=0;
		rectSrcVideo.right=lWidth+rectSrcVideo.left-1;
		rectSrcVideo.bottom=lHeight+rectSrcVideo.top-1;
		/* Calculate the coordinates of the destination window */
		rectDstVideo.right=lWidth+rectDstVideo.left-1;
		rectDstVideo.bottom=lHeight+rectDstVideo.top-1;
		/*
		SetRect(&rectSrcVideo,rectDstVideo.left,0,lWidth,lHeight);
	    SetRect(&rectDstVideo,rectDstVideo.left,rectDstVideo.top,rectSrcVideo.right,rectSrcVideo.bottom+rectDstVideo.top);
		*/
		return pWindowlessControl->SetVideoPosition(&rectSrcVideo,&rectDstVideo);
	}else{
		LONG lWidth=0;
		LONG lHeight=0;
		LONG lWidth_=0;
		LONG lHeight_=0;
		/* Get the client window rect */
		GetClientRect(hwndVideo,&rectDstVideo);
		/* Get the native video size */
		pWindowlessControl->GetNativeVideoSize(&lWidth, &lHeight,&lWidth_,&lHeight_);
		/* Calculate the coordinates of the source video */
		rectSrcVideo.left=0;
		rectSrcVideo.top=0;
		rectSrcVideo.right=lWidth+rectSrcVideo.left-1;
		rectSrcVideo.bottom=lHeight+rectSrcVideo.top-1;
		/* Calculate the coordinates of the destination window */
		rectDstVideo.right=lWidth+rectDstVideo.left-1;
		rectDstVideo.bottom=lHeight+rectDstVideo.top-1;
		/*
		SetRect(&rectSrcVideo,rectDstVideo.left,0,lWidth,lHeight);
		SetRect(&rectDstVideo,rectDstVideo.left,rectDstVideo.top,rectDstVideo.right,rectDstVideo.bottom+rectDstVideo.top);
		*/
		return pWindowlessControl->SetVideoPosition(&rectSrcVideo,&rectDstVideo);
		/*
		RECT rectClient;
		GetClientRect(hwndVideo,&rectClient);
		rectDstVideo=rectClient;
		return pWindowlessControl->SetVideoPosition(NULL,&rectClient);
		*/
	}
}
//----------------------------------------------------------
// Method @@@
//				Repaint(HDC hdc)
// Description @@@
//				Repaints the video.
//				Call this method when the application receives WM_PAINT.
//				
//------------------------------------------------------------
HRESULT CPlayBack::Repaint(HDC hdc)
{
	if(pWindowlessControl)
		return pWindowlessControl->RepaintVideo(hwndVideo,hdc);
	else
		return S_OK;
}
//----------------------------------------------------------
// Method @@@
//				DisplayModeChanged()
// Description @@@
//				Notifies the VMR that the display mode changed.
//				Call this method when the application receives WM_DISPLAYCHANGE.
//				
//------------------------------------------------------------
HRESULT CPlayBack::DisplayModeChanged()
{
	if(pWindowlessControl)
		return pWindowlessControl->DisplayModeChanged();
	else
		return S_OK;
}
//----------------------------------------------------------
// Method @@@
//				CanSeek()
// Description @@@
//				Returns TRUE if the current file is seekable.
//				
//------------------------------------------------------------
BOOL CPlayBack::CanSeek() const
{
	const DWORD dwCaps=AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanGetDuration;
	return ((dwSeekCaps&dwCaps)==dwCaps);
}
//----------------------------------------------------------
// Method @@@
//				SetPosition(REFERENCE_TIME pos)
// Description @@@
//				Seeks to a new position.
//				
//------------------------------------------------------------
HRESULT CPlayBack::SetPosition(REFERENCE_TIME pos)
{
	if(pMediaControl==NULL||pMediaSeek==NULL)
		return E_UNEXPECTED;
	HRESULT hResult=S_OK;
	/* IMediaControl:: Set Position to MediaSeeking */
	hResult=pMediaSeek->SetPositions(&pos,AM_SEEKING_AbsolutePositioning,NULL,AM_SEEKING_NoPositioning);
	if(SUCCEEDED(hResult))
	{	if(pbState==STATE_STOPPED)
			hResult=pMediaControl->StopWhenReady();
	}
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				GetDuration(LONGLONG *pDuration)
// Description @@@
//				Gets the duration of the current file.
//				
//------------------------------------------------------------
HRESULT CPlayBack::GetDuration(LONGLONG *pDuration)
{
	HRESULT hResult=S_OK;
	/* Check Media Seek */
	if(pMediaSeek==NULL)
		return E_UNEXPECTED;
	hResult=pMediaSeek->GetDuration(pDuration);
	if(SUCCEEDED(hResult))
		rDuration=(REFERENCE_TIME)*pDuration;
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				GetCurrentPosition(LONGLONG *pTimeNow)
// Description @@@
//				Gets the current CPlayBack position.
//				
//------------------------------------------------------------
HRESULT CPlayBack::GetCurrentPosition(LONGLONG *pTimeNow)
{
	HRESULT hResult=S_OK;
	/*Check Media Seek */
	if(pMediaSeek==NULL)
		return E_UNEXPECTED;
	hResult=pMediaSeek->GetCurrentPosition(pTimeNow);
	if(SUCCEEDED(hResult))
		rCurrentTime=*pTimeNow;
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				InitializeGraph()
// Description @@@
//				Create a new filter graph. (Tears down the old graph.) 
//				
//------------------------------------------------------------
HRESULT CPlayBack::InitializeGraph()
{
	HRESULT hResult=S_OK;
	TearDownGraph();
	/* Initialize the Filter Graph Manager.*/
	hResult=CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder,(void**)&pGraphBuilder);
	/* Require for Graph Interfaces */
	if(SUCCEEDED(hResult))
		hResult=pGraphBuilder->QueryInterface(IID_IMediaControl,(void**)&pMediaControl);
	if(SUCCEEDED(hResult))
		hResult=pGraphBuilder->QueryInterface(IID_IMediaEventEx,(void**)&pMediaEvent);
	if(SUCCEEDED(hResult))
		hResult=pGraphBuilder->QueryInterface(IID_IMediaSeeking,(void**)&pMediaSeek);
	/* For Event Notification */
	if(SUCCEEDED(hResult))
		/* This method registers a window to process event notification */
		hResult=pMediaEvent->SetNotifyWindow((OAHWND)hwndEvent,uiEventMsg,NULL);
	return hResult;
}
//----------------------------------------------------------
// Method @@@
//				TearDownGraph()
// Description @@@
//				Tear down the filter graph and release resources. 
//				
//------------------------------------------------------------
void CPlayBack::TearDownGraph()
{
	if(pMediaEvent)
		pMediaEvent->SetNotifyWindow((OAHWND)NULL,NULL,NULL);
	/* Release */
	SAFE_RELEASE(pGraphBuilder);
	SAFE_RELEASE(pMediaControl);
	SAFE_RELEASE(pMediaEvent);
	SAFE_RELEASE(pMediaSeek);
	SAFE_RELEASE(pWindowlessControl);
	pbState=STATE_CLOSED;
	dwSeekCaps=0;
}
//-----------------------------------------------------------------------------------------------
// Method @@@	
//			CPlayBack::RenderStreams(IBaseFilter *pSource)
//			*pSource → pointer to the file stream(filter of IBaseFilter) 
//
// Description @@@ 
//			Render the streams from a source filter. 
//
//			   @@@ Configure the VMR for the Windowless Mode @@@
//				1. Add new functionality to the Filter Graph(pGraphBuilder)
//				2. Add and register the VMR filter to the Filter Graph(pGraphBuilder)
//				3. Configure the VMR Filter(pVMRBaseFilter) for Windowless Mode. 
//					This must be done before the VMR is connected.
//				4. Add and register the Audio Renderer Filter(pAudioRenderer) to the Filter Graph(pGraphBuilder)
//				5. Enumerate the pins on the source filter(pSource)→pSource pointer to the file stream
//				6. Remove the un-used Renderer Filter
//---------------------------------------------------------------------------------------------------
HRESULT	CPlayBack::RenderStreams(IBaseFilter* pSource)
{
	HRESULT hResult=S_OK;
	BOOL bRenderedAnyPin=FALSE;
	IFilterGraph2* pFilterGraph2=NULL;
	IEnumPins* pEnum=NULL;
	IBaseFilter* pVMRBaseFilter=NULL;
	/* Add new functionality to the Filter Graph(pGraphBuilder) */
	hResult=pGraphBuilder->QueryInterface(IID_IFilterGraph2,(void**)&pFilterGraph2);
	/* Add and register the VMR filter to the Filter Graph(pGraphBuilder) */
	if(SUCCEEDED(hResult))
		hResult=AddFilterByCLSID(pGraphBuilder,CLSID_VideoMixingRenderer,&pVMRBaseFilter,L"VMR-7");
	/* Configure the VMR Filter(pVMRBaseFilter) for Windowless Mode */
	if(SUCCEEDED(hResult))
		hResult=InitWindowlessVMR(pVMRBaseFilter,hwndVideo,&pWindowlessControl);
	/* Enumerate the pins on the source filter(pSource) */
	if(SUCCEEDED(hResult))
		hResult=pSource->EnumPins(&pEnum);
	if(SUCCEEDED(hResult))
	{	/* Loop through all the pins */
		IPin* pPin=NULL;
		while(S_OK==pEnum->Next(1,&pPin,NULL))
		{	/* Present Pins →　Try to render this pin.It's OK if we fail some pins, if at least one pin renders.*/
			HRESULT hResult_=pFilterGraph2->RenderEx(pPin,AM_RENDEREX_RENDERTOEXISTINGRENDERERS,NULL);
			pPin->Release();
			if(SUCCEEDED(hResult_))
				bRenderedAnyPin=TRUE;
		}
	}
	/* Remove the un-used Source Renderer Filter(VMR) */
	if(SUCCEEDED(hResult))
	{
    	BOOL bRemoved=FALSE;
		hResult=RemoveUnconnectedRenderer(pGraphBuilder,pVMRBaseFilter,&bRemoved);
		/* If we removed the VMR, then we also need to release our pointer to the VMR's windowless control interface.*/
		if(bRemoved)
			SAFE_RELEASE(pWindowlessControl);
	}
	SAFE_RELEASE(pEnum);
	SAFE_RELEASE(pVMRBaseFilter);
	SAFE_RELEASE(pFilterGraph2);
	/* If we succeeded to this point, make sure we rendered at least one stream.*/
	if(SUCCEEDED(hResult)){
		if(!bRenderedAnyPin)
			hResult=VFW_E_CANNOT_RENDER;
	}
	return hResult;
}
//------------------------------------------------------------------------------
// Function @@@
//				RemoveUnconnectedRenderer(IGraphBuilder *pGraph, 
//										  IBaseFilter *pRenderer, BOOL *pbRemoved)
// Description @@@
//				Remove a renderer filter from the graph if the filter is
//				not connected. 
//-------------------------------------------------------------------------------
HRESULT RemoveUnconnectedRenderer(IGraphBuilder* pGraph,IBaseFilter* pRenderer,BOOL* pbRemoved)
{
	IPin* pPin=NULL;
	BOOL bRemoved_=FALSE;
	/* Look for a connected input pin on the renderer. */
	HRESULT hResult=FindConnectedPin(pRenderer,PINDIR_INPUT,&pPin);
	SAFE_RELEASE(pPin);
	/* If this function succeeds, the renderer is connected, so we don't remove it.*/
	/* If it fails, it means the renderer is not connected to anything, so we remove it.*/
	if(FAILED(hResult)){
		hResult=pGraph->RemoveFilter(pRenderer);
		bRemoved_=TRUE;
	}
	if(SUCCEEDED(hResult))
		*pbRemoved=bRemoved_;
	return hResult;
}
//-----------------------------------------------------------------------------
// Function @@@
//				InitWindowlessVMR(IBaseFilter *pVMR, HWND hwnd, IVMRWindowlessControl** ppWC) 
// Description @@@
//				Initialize the VMR-7 for Windowless Mode
//				Process:
//					1.Create the Filter and Add it to the Graph Builder → IBaseFilter::QueryInterface()
//					2.Call the IVMRFilterConfig::SetRenderingMode(VMRMode_Windowless)
//					3.Create the Windowless Control Interfaces to the Graph Builder → IBaseFilter::QueryInterface()
//					4.Create an input pin for each stream→IVMRFilterConfig::SetNumberOfStreams
//					5.To specify the window in which the rendered video will appear.Call IVMRFilterConfig::SetVideoClippingWindow 
//-----------------------------------------------------------------------------
HRESULT InitWindowlessVMR(IBaseFilter* pVMR,HWND hwnd,IVMRWindowlessControl** ppWC) 
{ 
	/* The IVMRFilterConfig interface is used to configure the operating mode */
	/* and video rendering mechanisms of the Video Mixing Renderer Filter 7 (VMR-7).*/
	/* For the VMR-9, use the IVMRFilterConfig9 interface.*/ 
	IVMRFilterConfig* pVMRFilterConfig=NULL; 
	IVMRWindowlessControl* pVMRWindowlessControl=NULL;
	HRESULT hResult=S_OK;
	/* Set the rendering mode as Windowless */ 
	hResult=pVMR->QueryInterface(IID_IVMRFilterConfig,(void**)&pVMRFilterConfig); 
    if(SUCCEEDED(hResult)) 
		hResult=pVMRFilterConfig->SetRenderingMode(VMRMode_Windowless); 
	/* Query for the windowless control interface. */
    if(SUCCEEDED(hResult))
        hResult=pVMR->QueryInterface(IID_IVMRWindowlessControl,(void**)&pVMRWindowlessControl);
	/* Set the clipping window. */
	if (SUCCEEDED(hResult))
		hResult=pVMRWindowlessControl->SetVideoClippingWindow(hwnd);
	/* Preserve aspect ratio by letter-boxing */
	if(SUCCEEDED(hResult))
		hResult=pVMRWindowlessControl->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
	/* Return the IVMRWindowlessControl pointer to the caller.*/
	if(SUCCEEDED(hResult)){
		*ppWC=pVMRWindowlessControl;
		(*ppWC)->AddRef();
	}
	SAFE_RELEASE(pVMRFilterConfig);
	SAFE_RELEASE(pVMRWindowlessControl);
	return hResult; 
}
//------------------------------------------------------------------------------
//Function @@@
//				GetOutPin(IBaseFilter *pBaseFilterOutPin, int iPin)
//Description @@@
//				Get the output pin
//-------------------------------------------------------------------------------
IPin* CPlayBack::GetOutPin(IBaseFilter* pBaseFilterOutPin,int iPin)
{
	IPin* pComPin=0;
	HRESULT hResult=S_OK;
	hResult=GetPin(pBaseFilterOutPin,PINDIR_OUTPUT,iPin,&pComPin);
	return pComPin;	
}
//------------------------------------------------------------------------------
//Function @@@
//				GetInPin(IBaseFilter *pBaseFilterInPin, int iPin)
//Description @@@
//				Get the input pin
//-------------------------------------------------------------------------------
IPin *CPlayBack::GetInPin(IBaseFilter* pBaseFilterInPin,int iPin) 
{
	IPin* pComPin=0;
	HRESULT hResult=S_OK;
	hResult=GetPin(pBaseFilterInPin,PINDIR_INPUT,iPin,&pComPin);
	return pComPin;
}
//------------------------------------------------------------------------------
//Function @@@
//				GetPin(IBaseFilter *pBaseFilterPin, PIN_DIRECTION dirRequired, int iNum, IPin **ppPin)
//Description @@@
//				Get pin
//-------------------------------------------------------------------------------
HRESULT	CPlayBack::GetPin(IBaseFilter* pBaseFilterPin,PIN_DIRECTION dirRequired,int iNum,IPin** ppPin)
{
	IEnumPins* pEnum;
	HRESULT hResult=S_OK;
	*ppPin=NULL;
	hResult=pBaseFilterPin->EnumPins(&pEnum);
	if(FAILED(hResult))
		return hResult;	
	ULONG ulFound;
    IPin* pPin;
	hResult=E_FAIL;
	while(S_OK==pEnum->Next(1,&pPin,&ulFound))
	{
		PIN_DIRECTION pinDirection=(PIN_DIRECTION)3;
		pPin->QueryDirection(&pinDirection);
		if(pinDirection==dirRequired){
			if(iNum==0){
				*ppPin=pPin;
				hResult=S_OK;
				break;
			}
			iNum--;
		}
		pPin->Release();
	}
	return hResult;
}
int CPlayBack::GetGrabRatio(void)
{
	return m_iSaveGrabRatio;
}
void CPlayBack::SetGrabRatio(int iSaveGrabRatio)
{
	m_iSaveGrabRatio=iSaveGrabRatio;
}
void CPlayBack::SetParentWnd(HWND h_wnd)
{
	Parent_hwnd=h_wnd;
}
void CPlayBack::SetRegionWasSetFlag(BOOL fRegionWasSet)
{
	fRegionSet_ = fRegionWasSet;
}
void CPlayBack::GetVideoRect(RECT* pRectSrcVideo,RECT* pRectDstVideo)
{
	*pRectSrcVideo=rectSrcVideo;
	*pRectDstVideo=rectDstVideo;
}