#pragma once

#include <dshow.h>
#include <qedit.h>
#include "CAVIFileInfo.h"

extern		CAVIFileInfo	AVIFileInfoObj;							// This global object was declared in CPlayBack

//------------------------------------------------------------------------------------------------
//Method @@@
//				ISampleGrabberCB
//Description @@@
//				ISampleGrabberCB interface provides callback method for ISampleGrabber::SetCallBack.
//				There are two callback methods BufferCB and SampleCB.
//				We will call the BufferCB method. It receives pointer to the sample buffer.
//				
//-------------------------------------------------------------------------------------------------
class CGrabBitmaps	: public ISampleGrabberCB
{
public:
	// Constructor
	CGrabBitmaps();
	//Destructor
	~CGrabBitmaps();
public: 
	long lWidth;		// Bitmap width
	long lHeight;		// Bitmap height
    STDMETHODIMP_(ULONG) AddRef(){return 2;}
    STDMETHODIMP_(ULONG) Release(){return 1;}
    STDMETHODIMP QueryInterface(REFIID riid,void ** ppv)
    {
		/* CheckPointer(ppv,E_POINTER) */
		/* Check the address in the void** pointer */
		if(*ppv==NULL)
			return E_POINTER;
        if(riid==IID_ISampleGrabberCB||riid==IID_IUnknown) 
        {
    		/* Cast the pointer of **ppv to the Pointer to ISampleGrabberCB */
			*ppv=(void*)static_cast<ISampleGrabberCB*>(this);
            return NOERROR;
        }
		return E_NOINTERFACE;
	}
	/* We do not use it */
	STDMETHODIMP SampleCB(double SampleTime,IMediaSample* pSample)
    {
        return 0;
    }
	//----------------------------------------------------------------------------------------
	//	CallBack FunctionÅ®Sample Grabber will call back this function
	//	The BufferCB method is a callback method that receives a pointer to the sample buffer.
	//	The BufferCB method is a callback method that receives a pointer to the sample buffer.
	//  double Sample : Starting time of the sample
	//  BYTE *pBuffer : Pointer to the buffer that contains the sample data
	//----------------------------------------------------------------------------------------
	long lAccel;						// File Number count
	const WCHAR* pGetDirectoryPath;		// Get Created Directory from CAVIFileInfo
	WCHAR FileSaveDirectory[MAX_PATH];	// Keep Directory
	int iKeepCurrentFileNumber;			// Keep current file number
	int iDirNumber;

	STDMETHODIMP BufferCB(double dSampleTime,BYTE* pBuffer,long lBufferSize)
    {
		int iSizeOfBuffer;
		iSizeOfBuffer=sizeof(pBuffer);
		//----------------------------------
		// Convert the buffer into a bitmap
		//----------------------------------
		int iCurrentFileNumber_	;
		/* Get Current loaded file number */
		AVIFileInfoObj.GetCurrentFileNumber(&iCurrentFileNumber_);
		if(AVIFileInfoObj.FileData[iCurrentFileNumber_].bFileDirFlag==FALSE)
		{
			/* Create New Directory for the File */
			WCHAR* pGetDirectory;
			pGetDirectory=AVIFileInfoObj.CreateNewDirectory();
			if(pGetDirectory==NULL)
				return E_POINTER;
			/* Set New Directory that will include the Bitmaps */
			BOOL bRet=SetCurrentDirectory(pGetDirectory);
			if(bRet==TRUE)
				AVIFileInfoObj.FileData[iCurrentFileNumber_].bFileDirFlag=TRUE;
			if(iKeepCurrentFileNumber!=iCurrentFileNumber_)
				lAccel=0;
			iKeepCurrentFileNumber=iCurrentFileNumber_;
		}else{
			if(AVIFileInfoObj.FileData[iCurrentFileNumber_].bGrabBitmapFlag==TRUE)
			{
				/* Add New File Directory because once this file was grabbed */
				WCHAR* pGetDirectory;
				pGetDirectory=AVIFileInfoObj.GetDirectory();
				if(pGetDirectory==NULL)
					return E_POINTER;
				/* Take the length */
				int iRet=wcslen(pGetDirectory);
				/* Copy */ 
				wcsncpy_s(FileSaveDirectory,MAX_PATH-1, pGetDirectory,iRet);
				FileSaveDirectory[iRet]=0;
				if(iCurrentFileNumber_==iKeepCurrentFileNumber)
					iDirNumber++;
				(void)StringCchPrintf(&FileSaveDirectory[iRet],iRet+3,TEXT("%3.3d\0"),iDirNumber);
				/* The file wasn`t grabbed before , You should create new directory */
				BOOL bRet=CreateDirectory(FileSaveDirectory,NULL);
				if(bRet==TRUE)
				{			
					/* Set New Directory that will include the Bitmaps */
					BOOL bRet_=SetCurrentDirectory(FileSaveDirectory);
					if(bRet_==TRUE)
						AVIFileInfoObj.FileData[iCurrentFileNumber_].bFileDirFlag=TRUE;
				}
				AVIFileInfoObj.FileData[iCurrentFileNumber_].bGrabBitmapFlag=FALSE;
				lAccel=0;				
			}
			iKeepCurrentFileNumber=iCurrentFileNumber_;		
		}	
		/* Format the full path for the source folder */
		WCHAR wchFilename[MAX_PATH];
		wchFilename[MAX_PATH-1]=0;
		lAccel++;
		(void)StringCchPrintf(wchFilename,NUMELMS(wchFilename),TEXT("sample %7.7d.bmp\0"),lAccel);	
		int iRet=wcslen(wchFilename);
		wchFilename[iRet]=0;		
		/* Create a file to hold the bitmap */
		HANDLE hFile=CreateFile(wchFilename, 
								GENERIC_WRITE, 
								FILE_SHARE_READ, 
								NULL, 
								CREATE_ALWAYS, 
								NULL, 
								NULL );
		if(hFile==INVALID_HANDLE_VALUE)
			return 0;
		/* Write formatted data to a string */
		_tprintf(TEXT("Found a sample at time %ld ms\t[%s]\r\n"),long(lAccel*1000),wchFilename);

		/* Write out the file header */
		BITMAPFILEHEADER bfh;
		memset(&bfh,0,sizeof(bfh));
		bfh.bfType='MB';
		bfh.bfSize=sizeof(bfh)+lBufferSize+sizeof(BITMAPINFOHEADER);
		bfh.bfOffBits=sizeof(BITMAPINFOHEADER)+sizeof(BITMAPFILEHEADER);
		DWORD Written=0;
		/* Write out the bitmap file header to the .bmp file */
		WriteFile(hFile,&bfh,sizeof(bfh),&Written,NULL);
		/* Write the bitmap format */
		BITMAPINFOHEADER bih;
		memset(&bih,0,sizeof(bih));
		bih.biSize=sizeof(bih);
		bih.biWidth=lWidth;
		bih.biHeight=lHeight;
		bih.biPlanes=1;
		bih.biBitCount=24;
		Written=0;
		WriteFile(hFile,&bih,sizeof(bih),&Written,NULL);
#if 0
		unsigned long lHeaderSize = sizeof(bih)+sizeof(bfh);
		unsigned long lWritingSize=4096;
		long lLoop=lBufferSize/lWritingSize;
		long lRemain=0;
		lRemain=lBufferSize-(lLoop*lWritingSize);
		BYTE* pBuffer_=NULL;
		pBuffer_=pBuffer;
		/* Write the bitmap bits */
		//SetFilePointer(hFile,lHeaderSize,NULL,FILE_BEGIN);
		//unsigned long lMovingSize;
		for(int ii=0;ii<lLoop;ii++){
			Written=0;
			WriteFile(hFile,pBuffer,lWritingSize,&Written,NULL);
			//lMovingSize=lHeaderSize+((ii+1)*lWritingSize);
			//SetFilePointer(hFile,lMovingSize,NULL,FILE_BEGIN);
			pBuffer=pBuffer+lWritingSize;			
		}
		if(lRemain>0){
			Written=0;
			pBuffer_=pBuffer_+lLoop*lWritingSize;
			WriteFile(hFile,pBuffer_,lRemain,&Written,NULL);
		}
#endif

#if 1

		/* Write the bitmap bits */
		Written=0;
		WriteFile(hFile,pBuffer,lBufferSize,&Written,NULL);
		/* Close file handle */
#endif
		/* Close file handle */
		CloseHandle(hFile);
		return 0;
    }
public:
	void NotifyError(HWND hwnd,TCHAR* sMessage,HRESULT hrStatus)
	{
		TCHAR sTmp[512];
		HRESULT hResult=StringCchPrintf(sTmp,512,TEXT("%s hr=0x%X"),sMessage,hrStatus);
		if(SUCCEEDED(hResult))
			MessageBox(hwnd,sTmp,TEXT("Error"),MB_OK|MB_ICONERROR);
	}
	RECT	rcBitmapSize;
	void SetSizeOfBitmap(RECT* pBitmapSize)
	{
		rcBitmapSize=*pBitmapSize;
	}
};
