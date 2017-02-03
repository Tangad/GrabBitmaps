/* Direct Show helper Class */
#include <dshow.h>
#include "dshowUtil.h"
#include <strsafe.h>
#ifndef		SAFE_RELEASE
#define		SAFE_RELEASE(x) if (x) {x->Release(); x=NULL; }
#endif
struct MatchPinName
{
    const WCHAR* m_wszName;
    MatchPinName(const WCHAR* wszName)
    {
        m_wszName=wszName;
    }
    HRESULT operator()(IPin* pPin,BOOL* pResult)
    {
        assert(pResult!=NULL);
        PIN_INFO PinInfo;
        HRESULT hResult=pPin->QueryPinInfo(&PinInfo);
        if(SUCCEEDED(hResult))
        {
            PinInfo.pFilter->Release();
            if(wcscmp(m_wszName,PinInfo.achName)==0)
                *pResult=TRUE;
			else
                *pResult=FALSE;
         }
        return hResult;
	}
};
/* MatchPinDirectionAndConnection */
/* Function object to match a pin by direction and connection */
struct MatchPinDirectionAndConnection
{
    BOOL m_bShouldBeConnected;
    PIN_DIRECTION m_direction;
    MatchPinDirectionAndConnection(PIN_DIRECTION direction, BOOL bShouldBeConnected) 
    : m_bShouldBeConnected(bShouldBeConnected), m_direction(direction)
    {
    }
    HRESULT operator()(IPin *pPin,BOOL *pResult)
    {
        assert(pResult!=NULL);
        BOOL bMatch=FALSE;
        BOOL bIsConnected=FALSE;
        HRESULT hResult=IsPinConnected(pPin,&bIsConnected);
        if(SUCCEEDED(hResult))
        {
            if(bIsConnected==m_bShouldBeConnected)
				hResult=IsPinDirection(pPin,m_direction,&bMatch);
		}
        if(SUCCEEDED(hResult))
			*pResult=bMatch;
        return hResult;
	}
};
struct MatchPinDirectionAndCategory
{
	const GUID* m_pCategory;
    PIN_DIRECTION m_direction;
	MatchPinDirectionAndCategory(PIN_DIRECTION direction,REFGUID guidCategory)
		: m_direction(direction), m_pCategory(&guidCategory)
	{
	}
    HRESULT operator()(IPin* pPin,BOOL* pResult)
    {
        assert(pResult!=NULL);
        BOOL bMatch=FALSE;
		GUID category;
        HRESULT hResult=IsPinDirection(pPin,m_direction,&bMatch);			
        if(SUCCEEDED(hResult)&&bMatch)
        {
			hResult=GetPinCategory(pPin,&category);
			if(SUCCEEDED(hResult))
				bMatch=(category==*m_pCategory);
		}
        if(SUCCEEDED(hResult))
			*pResult=bMatch;
        return hResult;
    }
};
HRESULT AddFilterByCLSID(IGraphBuilder* pGraph,  // Pointer to the Filter Graph Manager.
						 const GUID& clsid,      // CLSID of the filter to create.
						 IBaseFilter **ppF,      // Receives a pointer to the filter.
						 LPCWSTR wszName		  // A name for the filter.
						)				
{
    if(!pGraph||!ppF) 
    {
        return E_POINTER;
    }
    *ppF=0;
    IBaseFilter* pFilter=NULL;
    HRESULT hResult=CoCreateInstance(clsid, 
									 NULL, 
									 CLSCTX_INPROC_SERVER, 
									 IID_IBaseFilter, 
									 (void**)&pFilter
									 );
    if(SUCCEEDED(hResult))
		hResult=pGraph->AddFilter(pFilter,wszName);
    if(SUCCEEDED(hResult)){
        *ppF=pFilter;
        (*ppF)->AddRef();
    }
    SAFE_RELEASE(pFilter);
    return hResult;
}
HRESULT AddFilterFromMoniker(IGraphBuilder* pGraph,          // Pointer to the Filter Graph Manager.
							 IMoniker* pFilterMoniker,		 // Pointer to the moniker.
							 IBaseFilter **ppF,              // Receives a pointer to the filter.
							 LPCWSTR wszName		         // A name for the filter (can be NULL).
							)
{
	if(!pGraph||!pFilterMoniker||!ppF)
		return E_POINTER;
	IBaseFilter* pFilter=NULL;
	/* Use the moniker to create the filter */
    HRESULT hResult=pFilterMoniker->BindToObject(0,0,IID_IBaseFilter,(void**)&pFilter);   
    /* Add the capture filter to the filter graph */
	if(SUCCEEDED(hResult))
		hResult=pGraph->AddFilter(pFilter,wszName);
	/* Return to the caller */
	if(SUCCEEDED(hResult)){
		*ppF=pFilter;
		(*ppF)->AddRef();
	}
	SAFE_RELEASE(pFilter);
	return hResult;
}
/* ConnectFilters: Filter to pin */
HRESULT ConnectFilters(IGraphBuilder* pGraph,	// Filter Graph Manager.
					   IPin* pOut,				// Output pin on the upstream filter.
					   IBaseFilter* pDest)		// Downstream filter.
{
    if((pGraph==NULL)||(pOut==NULL)||(pDest==NULL))
		return E_POINTER;
    /* Find an input pin on the downstream filter */
    IPin* pIn=NULL;
    HRESULT hResult=FindUnconnectedPin(pDest,PINDIR_INPUT,&pIn);
	/* Try to connect them */
	if(SUCCEEDED(hResult))
		hResult = pGraph->Connect(pOut,pIn);
	SAFE_RELEASE(pIn);
	return hResult;
}
/* ConnectFilters: Filter to filter */
HRESULT ConnectFilters(IGraphBuilder* pGraph, 
					   IBaseFilter* pSrc, 
					   IBaseFilter* pDest
					  )
{
    if((pGraph==NULL)||(pSrc==NULL)||(pDest==NULL))
		return E_POINTER;
	/* Find an output pin on the first filter */
    IPin* pOut=NULL;
    HRESULT hResult=FindUnconnectedPin(pSrc,PINDIR_OUTPUT,&pOut);
    if(SUCCEEDED(hResult))
		hResult=ConnectFilters(pGraph,pOut,pDest);
    SAFE_RELEASE(pOut);
    return hResult;
}
/* ConnectFilters: Filter to pin */
HRESULT ConnectFilters(IGraphBuilder* pGraph,IBaseFilter* pSrc,IPin* pIn)
{
    if((pGraph==NULL)||(pSrc==NULL)||(pIn==NULL))
		return E_POINTER;
    /* Find an output pin on the upstream filter */
    IPin* pOut=NULL;
    HRESULT hResult=FindUnconnectedPin(pSrc,PINDIR_OUTPUT,&pOut);
    if(SUCCEEDED(hResult))
		hResult=pGraph->Connect(pOut,pIn);
    SAFE_RELEASE(pOut);
    return hResult;
}
HRESULT CopyFormatBlock(AM_MEDIA_TYPE* pmt,const BYTE* pFormat,DWORD cbSize)
{
    if(pmt==NULL)
		return E_POINTER;
    if(cbSize==0){
        FreeMediaType(*pmt);
        return S_OK;
    }
    if(pFormat==NULL)
		return E_INVALIDARG;
    /* reallocate the format block */
    pmt->pbFormat=(BYTE*)CoTaskMemRealloc(pmt->pbFormat,cbSize);  // OK if pmt->pbFormat is NULL on input.
    if(pmt->pbFormat==NULL)
		return E_OUTOFMEMORY;
    pmt->cbFormat=cbSize;
    CopyMemory(pmt->pbFormat,pFormat,cbSize);
    return S_OK;
}
HRESULT CreateKernelFilter(const GUID& guidCategory,	// Filter category.
						   LPCOLESTR szName,		// The name of the filter.
						   IBaseFilter **ppFilter  // Receives a pointer to the filter.
						  )
{
    HRESULT hResult;
    if(!szName||!ppFilter) 
		return E_POINTER;
    ICreateDevEnum* pDevEnum=NULL;
    IEnumMoniker* pEnum=NULL;
    /* Create the system device enumerator */
    hResult=CoCreateInstance(CLSID_SystemDeviceEnum, 
							 NULL, 
							 CLSCTX_INPROC_SERVER,
							 IID_ICreateDevEnum,
							 (void**)&pDevEnum
							 );
    if(SUCCEEDED(hResult))
    {
        /* Create a class enumerator for the specified category */
        hResult=pDevEnum->CreateClassEnumerator(guidCategory,&pEnum,0);
        if(hResult!=S_OK) // S_FALSE means the category is empty.
			hResult=E_FAIL;
    }
    if(SUCCEEDED(hResult))
    {
        // Enumerate devices within this category.
        bool bFound=false;
        IMoniker* pMoniker=NULL;
        while(!bFound && (S_OK==pEnum->Next(1,&pMoniker,0)))
        {
            /* Get the property bag */
            IPropertyBag* pBag=NULL;
            hResult=pMoniker->BindToStorage(0,0,IID_IPropertyBag,(void **)&pBag);
            if(FAILED(hResult))
            {
                pMoniker->Release();
                continue; // Maybe the next one will work.
            }
            /* Check the friendly name */
            VARIANT var;
            VariantInit(&var);
            hResult=pBag->Read(L"FriendlyName",&var,NULL);
            if(SUCCEEDED(hResult)&&(lstrcmpiW(var.bstrVal,szName)==0))
            {
                /* This is the right filter */
                hResult=pMoniker->BindToObject(0,0,IID_IBaseFilter,
											  (void**)ppFilter);
                bFound=true;
            }
            VariantClear(&var);
            pBag->Release();
            pMoniker->Release();
        } // while
        if(!bFound) 
			hResult=E_FAIL;
	}
    SAFE_RELEASE(pDevEnum);
    SAFE_RELEASE(pEnum);
    return hResult;
}
HRESULT DisconnectPin(IGraphBuilder* pGraph,IPin* pPin)
{
    if(!pGraph||!pPin)
		return E_POINTER;
    IPin* pPinTo=NULL;
    HRESULT hResult=pPin->ConnectedTo(&pPinTo);
    if(hResult==VFW_E_NOT_CONNECTED)
		return S_OK;
    /* Disconnect the first pin */
	if(SUCCEEDED(hResult))
		hResult=pGraph->Disconnect(pPin);
	if(SUCCEEDED(hResult))
		pGraph->Disconnect(pPinTo);
    SAFE_RELEASE(pPinTo);
	return	hResult;
}
HRESULT FindConnectedPin(IBaseFilter* pFilter,		// Pointer to the filter.
						 PIN_DIRECTION PinDir,		// Direction of the pin to find.
						 IPin **ppPin				// Receives a pointer to the pin.
						)
{
    return FindMatchingPin(pFilter,MatchPinDirectionAndConnection(PinDir,TRUE),ppPin);
}

HRESULT FindPinByCategory(IBaseFilter* pFilter, 
						  REFGUID guidCategory,
						  PIN_DIRECTION	PinDir,
						  IPin **ppPin
						 )
{
	return FindMatchingPin(pFilter,MatchPinDirectionAndCategory(PinDir,guidCategory),ppPin);
}
HRESULT FindFilterInterface(IGraphBuilder* pGraph,	// Pointer to the Filter Graph Manager.
							REFGUID iid,			// IID of the interface to retrieve.
							void **ppUnk			// Receives the interface pointer.
							)			            // Receives the interface pointer.
{
    if(!pGraph||!ppUnk) 
		return E_POINTER;
    bool bFound=false;
    IEnumFilters* pEnum=NULL;
    IBaseFilter* pFilter=NULL;
    HRESULT hResult=pGraph->EnumFilters(&pEnum);
    if(FAILED(hResult))
		return hResult;
    /* Query every filter for the interface */
    while(S_OK==pEnum->Next(1,&pFilter,0))
    {
        hResult=pFilter->QueryInterface(iid,ppUnk);
        pFilter->Release();
        if(SUCCEEDED(hResult)){
            bFound=true;
            break;
        }
	}
    SAFE_RELEASE(pEnum);
    SAFE_RELEASE(pFilter);
    return (bFound ? S_OK : VFW_E_NOT_FOUND);
}
HRESULT FindGraphInterface(IGraphBuilder* pGraph, 
						   REFGUID iid, 
						   void **ppUnk
						   )
{
    if(!pGraph||!ppUnk) 
		return E_POINTER;
    bool bFound=false;
    IEnumFilters* pEnum=NULL;
    IBaseFilter* pFilter=NULL;
    HRESULT hResult=pGraph->EnumFilters(&pEnum);
    if(FAILED(hResult))
		return hResult;
    /* Loop through every filter in the graph */
    while(S_OK==pEnum->Next(1,&pFilter,0))
    {
        hResult=pFilter->QueryInterface(iid,ppUnk);
        if(FAILED(hResult))
            /* The filter does not expose the interface, but maybe one of its pins does */
            hResult=FindPinInterface(pFilter,iid,ppUnk);
        pFilter->Release();
        if(SUCCEEDED(hResult)){
            bFound=true;
            break;
        }
    }
    SAFE_RELEASE(pEnum);
    SAFE_RELEASE(pFilter);
    return (bFound ? S_OK : VFW_E_NOT_FOUND);
}
/* FindPinByIndex: Get the Nth pin with a specified direction */
HRESULT FindPinByIndex(IBaseFilter* pFilter, 
					   PIN_DIRECTION PinDir,
					   UINT nIndex, 
					   IPin **ppPin
					   )
{
	if(!pFilter||!ppPin)
		return E_POINTER;
	IEnumPins* pEnum=NULL;
	IPin* pPin=NULL;
	HRESULT hResult=pFilter->EnumPins(&pEnum);
	if(FAILED(hResult))
		return hResult;
	bool bFound=false;
	UINT count=0;
	while(S_OK==(hResult=pEnum->Next(1,&pPin,NULL)))
	{
		PIN_DIRECTION ThisDir;
		hResult=pPin->QueryDirection(&ThisDir);
		if(FAILED(hResult)){
			pPin->Release();
			break;
		}
		if(ThisDir==PinDir){
			if(nIndex==count){
				*ppPin=pPin;			// return to caller
				bFound=true;
				break;
			}
			count++;
		}
		pPin->Release();
	}
    SAFE_RELEASE(pPin);
	SAFE_RELEASE(pEnum);
    return (bFound ? S_OK : VFW_E_NOT_FOUND);
}
HRESULT FindPinByName(IBaseFilter* pFilter, 
					  const WCHAR* wszName, 
					  IPin	**ppPin
					 )
{
    if(!pFilter||!wszName||!ppPin)
		return E_POINTER;
    /* Verify that wszName is not longer than MAX_PIN_NAME */
    size_t cch;
    HRESULT hResult=StringCchLengthW(wszName,MAX_PIN_NAME, &cch);

    if(SUCCEEDED(hResult))
		hResult = FindMatchingPin(pFilter,MatchPinName(wszName),ppPin);
    return hResult;
}
HRESULT FindPinInterface(IBaseFilter* pFilter,	// Pointer to the filter to search.
						 REFGUID iid,	        // IID of the interface.
						 void **ppUnk			// Receives the interface pointer.
						 )						// Receives the interface pointer.
{
    if(!pFilter || !ppUnk) 
		return E_POINTER;
    bool bFound=false;
    IEnumPins* pEnum=NULL;
    IPin* pPin=NULL;
    HRESULT hResult=pFilter->EnumPins(&pEnum);
    if(FAILED(hResult))
		return hResult;
    /* Query every pin for the interface */
    while (S_OK == pEnum->Next(1, &pPin, 0))
    {
        hResult=pPin->QueryInterface(iid, ppUnk);
        pPin->Release();
        if(SUCCEEDED(hResult))
        {
            bFound=true;
            break;
        }
    }
    SAFE_RELEASE(pEnum);
    SAFE_RELEASE(pPin);
    return (bFound ? S_OK : VFW_E_NOT_FOUND);
}
HRESULT FindUnconnectedPin(IBaseFilter* pFilter,   // Pointer to the filter.
						   PIN_DIRECTION PinDir,   // Direction of the pin to find.
						   IPin	**ppPin            // Receives a pointer to the pin.
						   )
{
    return FindMatchingPin(pFilter,MatchPinDirectionAndConnection(PinDir,FALSE),ppPin);
}
HRESULT GetConnectedFilter(IPin* pPin,IBaseFilter **ppFilter)
{
    if(!pPin||!ppFilter)
		return E_POINTER;
    IPin* pPeer=NULL;
    PIN_INFO info;
    ZeroMemory(&info,sizeof(info));
    HRESULT hResult=pPin->ConnectedTo(&pPeer);
    if(SUCCEEDED(hResult))
		hResult=pPeer->QueryPinInfo(&info);
    if(SUCCEEDED(hResult))
    {
        assert(info.pFilter!=NULL);
        if(info.pFilter){
            *ppFilter=info.pFilter;   // Return pointer to caller.
            (*ppFilter)->AddRef();
            hResult=S_OK;
        }else{
            hResult=E_UNEXPECTED;  // Pin does not have an owning filter! That's weird! 
        }
    }
    SAFE_RELEASE(pPeer);
    SAFE_RELEASE(info.pFilter);
    return hResult;
}
// Get the first upstream or downstream filter
HRESULT GetNextFilter(IBaseFilter* pFilter,		// Pointer to the starting filter
					  GraphDirection Dir,		// Direction to search (upstream or downstream)
					  IBaseFilter **ppNext		// Receives a pointer to the next filter.
					  ) 
{
    PIN_DIRECTION PinDirection=(Dir== UPSTREAM ? PINDIR_INPUT : PINDIR_OUTPUT); 
    if(!pFilter||!ppNext) 
		return E_POINTER;
    IPin* pPin=NULL;
    HRESULT hResult=FindConnectedPin(pFilter,PinDirection,&pPin);
    if(SUCCEEDED(hResult))
    {
        hResult=GetConnectedFilter(pPin,ppNext);
    }
    SAFE_RELEASE(pPin);
    return hResult;
}
/* GetPinCategory: Return the pin category */
HRESULT GetPinCategory(IPin* pPin,GUID* pPinCategory)
{
    if(pPin==NULL)
		return E_POINTER;
    if(pPinCategory==NULL)
		return E_POINTER;
    IKsPropertySet* pKs=NULL;
    HRESULT hResult=pPin->QueryInterface(IID_IKsPropertySet,(void**)&pKs);
    if(SUCCEEDED(hResult))
    {
        // Try to retrieve the pin category.
        DWORD	cbReturned;
        hResult=pKs->Get(AMPROPSETID_Pin,AMPROPERTY_PIN_CATEGORY,NULL,0,pPinCategory,sizeof(GUID),&cbReturned);
    }
    SAFE_RELEASE(pKs);
    return hResult;
}
HRESULT IsPinConnected(IPin* pPin,BOOL* pResult)
{
    if(pPin==NULL||pResult==NULL)
		return E_POINTER;
    IPin* pTmp=NULL;
    HRESULT hResult=pPin->ConnectedTo(&pTmp);
	if(SUCCEEDED(hResult)){
		*pResult=TRUE;
	}else if (hResult==VFW_E_NOT_CONNECTED){
        // The pin is not connected. This is not an error for our purposes.
        *pResult=FALSE;
        hResult=S_OK;
    }
    SAFE_RELEASE(pTmp);
    return hResult;
}
HRESULT IsPinDirection(IPin* pPin,PIN_DIRECTION dir,BOOL* pResult)
{
    if(pPin==NULL||pResult==NULL)
		return E_POINTER;
    PIN_DIRECTION pinDir;
    HRESULT hResult=pPin->QueryDirection(&pinDir);
    if(SUCCEEDED(hResult))
		*pResult=(pinDir==dir);
    return hResult;
}
HRESULT IsPinUnconnected(IPin* pPin,BOOL* pResult)
{
    HRESULT hResult=IsPinConnected(pPin,pResult);
    if(SUCCEEDED(hResult))
    *pResult=!(*pResult);
    return hResult;
}
HRESULT IsSourceFilter(IBaseFilter* pFilter,BOOL* pResult)
{
    if(pFilter==NULL||pResult==NULL)
		return E_POINTER;
    IAMFilterMiscFlags* pFlags=NULL;
    IFileSourceFilter* pFileSrc=NULL;
    BOOL bIsSource=FALSE;
   /* First try IAMFilterMiscFlags */
    HRESULT hResult=pFilter->QueryInterface(IID_IAMFilterMiscFlags,(void**)&pFlags);
    if(SUCCEEDED(hResult))
    {
        ULONG flags=pFlags->GetMiscFlags();
        if(flags & AM_FILTER_MISC_FLAGS_IS_SOURCE)
			bIsSource=TRUE;
    }else{
        /* Next, look for IFileSourceFilter */
        hResult=pFilter->QueryInterface(IID_IFileSourceFilter,(void**)&pFileSrc);
        if(SUCCEEDED(hResult))
			bIsSource=TRUE;
   }
	if(SUCCEEDED(hResult))
		*pResult=bIsSource;
    SAFE_RELEASE(pFlags);
    SAFE_RELEASE(pFileSrc);
    return hResult;
}
///////////////////////////////////////////////////////////////////////
//
// Name: AddSourceFilter
// Desc: Load a specified source filter for a file.
//
///////////////////////////////////////////////////////////////////////
HRESULT AddSourceFilter(IGraphBuilder* pGraph,			// Pointer to the filter graph manager.
						const WCHAR* szFile,            // File name
						const GUID&	clsidSourceFilter,  // CLSID of the source filter to use
						IBaseFilter	**ppSourceFilter)   // receives a pointer to the filter.
{
    if(!pGraph||!szFile||!ppSourceFilter)
		return E_POINTER;
    IBaseFilter* pSource=NULL;
    IFileSourceFilter* pFileSource=NULL;
    /* Create the source filter and add it to the graph */
    HRESULT hResult=CoCreateInstance(clsidSourceFilter, 
									 NULL, 
									 CLSCTX_INPROC_SERVER,
									 IID_IBaseFilter,
									 (void**)&pSource);
    if(SUCCEEDED(hResult))
		hResult = pGraph->AddFilter(pSource,szFile);
    if (SUCCEEDED(hResult))
		hResult = pSource->QueryInterface(IID_IFileSourceFilter,(void**)&pFileSource);
    if (SUCCEEDED(hResult))
		hResult = pFileSource->Load(szFile,NULL);
    if (SUCCEEDED(hResult)){
        /* Return the filter pointer to the caller */
        *ppSourceFilter=pSource;
        (*ppSourceFilter)->AddRef();
    }else{
        /* FAILED, remove the filter */
        if(pSource!=NULL)
			RemoveFilter(pGraph,pSource);
    }
    SAFE_RELEASE(pSource);
    SAFE_RELEASE(pFileSource);
    return hResult;
}
///////////////////////////////////////////////////////////////////////
//
// Name: AddSourceFilter
// Desc: Add a file-writer filter.
//
///////////////////////////////////////////////////////////////////////
HRESULT AddWriterFilter(IGraphBuilder* pGraph,
						const WCHAR* szFile,
						const GUID& clsid,
						BOOL bOverwrite,
						IBaseFilter **ppFilter
						)
{
    if(pGraph==NULL)
		return E_POINTER;
    if(szFile==NULL)
		return E_POINTER;
    if(ppFilter==NULL)
        return E_POINTER;
    IBaseFilter* pFilter=NULL;
    IFileSinkFilter* pSink=NULL;
	IFileSinkFilter2* pSink2=NULL;
    HRESULT hResult=AddFilterByCLSID(pGraph,clsid,&pFilter,szFile);
    if(SUCCEEDED(hResult))
		hResult = pFilter->QueryInterface(IID_IFileSinkFilter,(void**)&pSink);
	if(SUCCEEDED(hResult))
		hResult = pSink->SetFileName((LPCOLESTR)szFile,NULL);
    if(SUCCEEDED(hResult) && bOverwrite){
		hResult=pFilter->QueryInterface(IID_IFileSinkFilter2,(void**)&pSink2);
		if(SUCCEEDED(hResult)){
			hResult=pSink2->SetMode(AM_FILE_OVERWRITE);
		}
	}
    if(SUCCEEDED(hResult)){
        *ppFilter=pFilter;
        (*ppFilter)->AddRef();
    }else{
        RemoveFilter(pGraph,pFilter);
    }
	SAFE_RELEASE(pFilter);
	SAFE_RELEASE(pSink);
	SAFE_RELEASE(pSink2);
    return hResult;
}
HRESULT RemoveFilter(IGraphBuilder* pGraph,IBaseFilter* pFilter)
{
    if(!pGraph||!pFilter)
		return E_POINTER;
    IEnumPins* pEnum=NULL;
    HRESULT hResult=pFilter->EnumPins(&pEnum);
    if(SUCCEEDED(hResult))
    {
		/* Disconnect all the pins */
        IPin* pPin=NULL;
        while(S_OK==pEnum->Next(1,&pPin,0))
        {
            hResult=DisconnectPin(pGraph, pPin);
            pPin->Release();
            if(FAILED(hResult))
				break;
        }
    }
    if(SUCCEEDED(hResult))
        hResult=pGraph->RemoveFilter(pFilter);
    SAFE_RELEASE(pEnum);
    return hResult;
}