#include "stdafx.h"
#include "CAVIFileInfo.h"
/* Constructor */
CAVIFileInfo::CAVIFileInfo()
{
	ZeroMemory(FileData,FILENUMBER);
	iCurrentFileNumber=-1;
	pFileDirectory[0]=L'\0';
	lpBuffer[MAX_PATH-1]=0;
}
/* Destructor */
CAVIFileInfo::~CAVIFileInfo() 
{
}
//---------------------------------------------------------
// Method @@@
//					AVIFileProcessing(const WCHAR *sFileName)
// Description @@@
//					Handle AVI file using Video for Windows
//---------------------------------------------------------
HRESULT CAVIFileInfo::AVIFileProcessing(const WCHAR *sFileName)
{
	HRESULT hResult=S_OK;
	PAVISTREAM hAVIStream;
	PAVIFILE hPAVIFile;
	AVIFILEINFO sAviFileInfo;
	AVISTREAMINFO sAviStreamInfo;					  
	DWORD fccType=streamtypeVIDEO;
	DWORD dFourCC=formtypeAVI;
	int iFileNumber=0;
	int sdtResult=0;
	long lParam=NULL;
	/* Initilaizes the AVIFile library */
	AVIFileInit();
	/* Open the AVI File */
	hResult=AVIFileOpen(&hPAVIFile,sFileName,OF_SHARE_DENY_READ,NULL);
	if(SUCCEEDED(hResult)){
		long lSize=sizeof(AVIFILEINFO);
		sdtResult=AVIFileInfo(hPAVIFile,&sAviFileInfo,lSize);
	}else{
		return hResult;
	}	
	if(sdtResult==0)
		hResult=AVIFileGetStream(hPAVIFile,&hAVIStream,fccType,lParam);
	if(sdtResult==0){	
		long lSize=sizeof(AVISTREAMINFO);
		sdtResult=AVIStreamInfo(hAVIStream,&sAviStreamInfo,lSize);
	}
	if(sdtResult==0)
		sdtResult=hAVIStream->Info(&sAviStreamInfo,sizeof(AVISTREAMINFO));
	/* Set Current File Directory */
	SetCurrentFileDirectory(sFileName);
	/* There is a AVI file with Video stream */
	if(sdtResult==0){
		int	iFileNumber;
		int iRet=GetCurrentFileNumber(&iFileNumber);
		if(iFileNumber==-1)
			iFileNumber=0;
		else
			iFileNumber=iFileNumber+1;
		SetCurrentFileNumber(iFileNumber);	
		iRet=SetData(&sAviStreamInfo);
	}
	/* Release the Video Stream and AVI file */
	AVIStreamRelease(hAVIStream);
	AVIFileRelease(hPAVIFile);
	/* Release the AVIFile Library */
	AVIFileExit();
	if(sdtResult!=0)
		hResult=S_FALSE;
	return hResult;
}
//---------------------------------------------------------
// Method @@@
//					SetData(AVISTREAMINFO *SetRate)
// Description @@@
//					Set the AVI video stream data to array
//---------------------------------------------------------
int CAVIFileInfo::SetData(AVISTREAMINFO *SetRate)
{
	int iRet=0;
	int iSetFileNumber;
	iRet=GetCurrentFileNumber(&iSetFileNumber);
	FileData[iSetFileNumber].lRate=SetRate->dwRate	;
	FileData[iSetFileNumber].lScale=SetRate->dwScale  ;
	return(0);
}
//---------------------------------------------------------
// Method @@@
//					GetData(Rate *GetRate) 
// Description @@@
//					Get the AVI video stream data
//---------------------------------------------------------
int CAVIFileInfo::GetData(Rate* GetRate) 
{
	int iRet=0;
	int	iSetFileNumber;
	iRet=GetCurrentFileNumber(&iSetFileNumber);
	GetRate->lRate=FileData[iSetFileNumber].lRate;
	GetRate->lScale=FileData[iSetFileNumber].lScale ;
	return 0;
}
//---------------------------------------------------------
// Method @@@
//					SetCurrentFileNumber(int iSetFileNumber)
// Description @@@
//					Set the File number that was opened
//---------------------------------------------------------
void CAVIFileInfo::SetCurrentFileNumber(int iSetFileNumber)
{
	iCurrentFileNumber=iSetFileNumber;
}
//---------------------------------------------------------
// Method @@@
//				GetCurrentFileNumber(int *pGetFileNumber) 
// Description @@@
//				Get the File number that was opened
//---------------------------------------------------------
int CAVIFileInfo::GetCurrentFileNumber(int *pGetFileNumber) 
{
	*pGetFileNumber=iCurrentFileNumber;
	return 0;
}
//Set Current File Directory
void CAVIFileInfo::SetCurrentFileDirectory(const WCHAR* pSetDirectoryName)
{
	wcsncpy_s(pFileDirectory,MAX_PATH-1,pSetDirectoryName,MAX_PATH-1);
}
// Get Current File Directory
int CAVIFileInfo::GetCurrentFileDirectory(WCHAR* pGetDirectoryName)
{
	*pGetDirectoryName=*pFileDirectory;
	return 0;
}
WCHAR *CAVIFileInfo::CreateNewDirectory()
{
	HRESULT hResult=S_OK;
	DWORD nBufferLength;
	nBufferLength=MAX_PATH-1;
	/* Get Current Directory */
	DWORD dwRet=GetCurrentDirectory(nBufferLength,lpBuffer);
	/* Get the length of the string */
	int iRet=wcslen(pFileDirectory);
	pFileDirectory[iRet]=0;
	int iiStart=0;
	int iiKeepSymbol;
	/* Get file name and Create Directory named FileName */
	for(int ii=0;ii<iRet;ii++)
	{
		if(pFileDirectory[ii]==L'\\')
		{
			iiStart=ii;
			iiKeepSymbol=ii;
		}
		if((pFileDirectory[ii]==L'.')&&(pFileDirectory[ii+1]==L'a')&&(pFileDirectory[ii+2]==L'v')&&(pFileDirectory[ii+3]==L'i'))
		{
			int iiCount=0;
			WCHAR FileName[MAX_PATH];
			FileName[MAX_PATH-1]=0;
			iiStart++;
			int iiEnd=ii;
			for(int jj=iiStart;jj<iiEnd;jj++)
			{
				FileName[iiCount]=pFileDirectory[jj];
				iiCount++;
			}
			FileName[iiCount]=0;
			pFileDirectory[iiKeepSymbol+1]=0;
			wcscat_s(lpBuffer,&pFileDirectory[iiKeepSymbol]);
			wcscat_s(lpBuffer,FileName);
			/* Get the length of the string */
			ii=iRet;
			int iRet=wcslen(lpBuffer);
			lpBuffer[iRet]=0;
		}
	}
	/* Create Directory */
	BOOL bRet=CreateDirectory(lpBuffer,NULL);
	return (&lpBuffer[0]);
}
WCHAR* CAVIFileInfo::GetDirectory() 
{
	return(&lpBuffer[0]);
}