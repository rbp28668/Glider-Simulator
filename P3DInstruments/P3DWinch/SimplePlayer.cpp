#include <mferror.h>
#include "SimplePlayer.h"

// See https://learn.microsoft.com/en-us/windows/win32/medfound/player-cpp


SimplePlayer::SimplePlayer() {
    HRESULT result = ::MFStartup(MF_VERSION);
    if (result != S_OK) std::cout << "Unable to start up Media Foundation" << result << std::endl;

    // 1. Create the Media Session
    result = ::MFCreateMediaSession(NULL, &m_pSession);
    if (result != S_OK) std::cout << "Unable to create Media Foundation session" << result << std::endl;

}
SimplePlayer::~SimplePlayer(){

    m_pSession->Shutdown();
    m_pSession->Release();
    ::MFShutdown();
}

HRESULT SimplePlayer::CreateMediaSource(const wchar_t* url, IMFMediaSource** ppSource) {
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

    IMFSourceResolver* pSourceResolver = NULL;
    IUnknown* pSource = NULL;

    // Create the source resolver.
    HRESULT hr = MFCreateSourceResolver(&pSourceResolver);
    if (FAILED(hr))
    {
        std::cout << "Unable to create media source resolver " << std::hex << hr << std::endl;
        goto done;
    }

    // Use the source resolver to create the media source.

  // Note: For simplicity this sample uses the synchronous method to create 
  // the media source. However, creating a media source can take a noticeable
  // amount of time, especially for a network source. For a more responsive 
  // UI, use the asynchronous BeginCreateObjectFromURL method.

    hr = pSourceResolver->CreateObjectFromURL(
        url,                       // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &ObjectType,        // Receives the created object type. 
        &pSource            // Receives a pointer to the media source.
    );

    if (FAILED(hr))
    {
        std::cout << "Unable to create media object from URL " << std::hex << hr << std::endl;
        goto done;
    }

    // Get the IMFMediaSource interface from the media source.
    hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource));

    done:
    if(pSourceResolver) pSourceResolver->Release();
    if (pSource) pSource->Release();
    return hr;

}


void SimplePlayer::Play(const wchar_t* url) {

    IMFMediaSource* pSource = nullptr;
    HRESULT result = CreateMediaSource(url, &pSource);
    if (FAILED(result)) return;

     // Create the Topology (Connecting Source to Audio Renderer)
    IMFTopology* pTopology = nullptr;
    CreatePlaybackTopology(pSource, &pTopology);

    // Set Topology and Start
    result = m_pSession->SetTopology(0, pTopology);
    if (result != S_OK) std::cout << "Unable to set topology: " << std::hex << result << std::endl;
    if (result == MF_E_INVALIDREQUEST) std::cout << "Invalid request" << std::endl;
    if (result == MF_E_TOPO_INVALID_TIME_ATTRIBUTES) std::cout << "Invalid time attributes" << std::endl;
    if (result == MF_E_TOPO_MISSING_PRESENTATION_DESCRIPTOR) std::cout << "Missing presentation descriptor" << std::endl;
    if (FAILED(result)) return;

    PROPVARIANT varStart;
    PropVariantInit(&varStart);
    varStart.vt = VT_EMPTY;
    result = m_pSession->Start(&GUID_NULL, &varStart);
    if (result != S_OK) std::cout << "Unable to start sound" << result << std::endl;


    // Clean up locals
    pSource->Release();
    pTopology->Release();
}


void SimplePlayer::CreatePlaybackTopology(IMFMediaSource* pSource, IMFTopology** ppTopo) {
    HRESULT result = MFCreateTopology(ppTopo);
    if (FAILED(result)) std::cout << "Unable to create MF topology" << result << std::endl;

    IMFPresentationDescriptor* pPD = nullptr;
    pSource->CreatePresentationDescriptor(&pPD);

    IMFStreamDescriptor* pSD = nullptr;
    BOOL selected;
    pPD->GetStreamDescriptorByIndex(0, &selected, &pSD);

    // Create a node for the audio stream and one for the audio renderer
    IMFTopologyNode* pSourceNode = nullptr;
    IMFTopologyNode* pOutputNode = nullptr;

    result = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pSourceNode);
    if (FAILED(result)) {
        std::cout << "Unable to create source topology node " << std::hex << result << std::endl;
    }

    pSourceNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
   
    result = pSourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
    if (FAILED(result))
    {
        std::cout << "Unable to set presentation descriptor " << std::hex << result << std::endl;
    }

    result = pSourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
    if (FAILED(result)) {
        std::cout << "Unable to set stream descriptor" << result << std::endl;
    }



    result = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pOutputNode);
    if (result != S_OK) std::cout << "Unable to create output topology node" << result << std::endl;

    // Create the standard Windows Audio Renderer (SAR)
    // Use IMFActivate instead of IActivate
    IMFActivate* pActivate = nullptr;

    // This function returns an IMFActivate pointer
    HRESULT hr = MFCreateAudioRendererActivate(&pActivate);

    if (SUCCEEDED(hr)) {
        pOutputNode->SetObject(pActivate);
        pActivate->Release();
    }
    else {
        std::cout << "Unable to active audio renderer" << result << std::endl;
    }

    (*ppTopo)->AddNode(pSourceNode);
    (*ppTopo)->AddNode(pOutputNode);
    pSourceNode->ConnectOutput(0, pOutputNode, 0);

    pSD->Release();
    pPD->Release();
    pSourceNode->Release();
    pOutputNode->Release();
}

