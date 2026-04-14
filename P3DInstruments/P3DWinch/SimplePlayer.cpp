#include <mferror.h>
#include "SimplePlayer.h"

// See https://learn.microsoft.com/en-us/windows/win32/medfound/player-cpp


SimplePlayer::SimplePlayer() {
    HRESULT result = ::MFStartup(MF_VERSION);
    if (result != S_OK) std::cout << "Unable to start up Media Foundation" << result << std::endl;

    // 1. Create the Media Session
    result = ::MFCreateMediaSession(NULL, m_pSession.GetAddressOf());
    if (result != S_OK) std::cout << "Unable to create Media Foundation session" << result << std::endl;

}
SimplePlayer::~SimplePlayer(){

    m_pSession->Shutdown();
    
    ::MFShutdown();
}

HRESULT SimplePlayer::CreateMediaSource(const wchar_t* url, ComPtr<IMFMediaSource>& ppSource) {
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

    ComPtr<IMFSourceResolver> pSourceResolver;
    ComPtr<IUnknown> pSource;

    // Create the source resolver.
    HRESULT hr = MFCreateSourceResolver(pSourceResolver.GetAddressOf());
    if (FAILED(hr))
    {
        std::cout << "Unable to create media source resolver " << std::hex << hr << std::endl;
        return hr;
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
        &ObjectType,                // Receives the created object type. 
        pSource.GetAddressOf()      // Receives a pointer to the media source.
    );

    if (FAILED(hr))
    {
        std::cout << "Unable to create media object from URL " << std::hex << hr << std::endl;
        return hr;
    }

    // Get the IMFMediaSource interface from the media source.
    hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource.GetAddressOf()));

    return hr;

}


void SimplePlayer::Play(const wchar_t* url) {

    ComPtr<IMFMediaSource> pSource;
    HRESULT result = CreateMediaSource(url, pSource);
    if (FAILED(result)) return;

     // Create the Topology (Connecting Source to Audio Renderer)
    ComPtr<IMFTopology> pTopology;
    CreatePlaybackTopology(pSource, pTopology);

    // Set Topology and Start
    result = m_pSession->SetTopology(0, pTopology.Get());
    if (FAILED(result)) {
        std::cout << "Unable to set topology: " << std::hex << result << std::endl;
        switch (result) {
            case MF_E_INVALIDREQUEST:  std::cout << "Invalid request" << std::endl; break;
            case MF_E_TOPO_INVALID_TIME_ATTRIBUTES: std::cout << "Invalid time attributes" << std::endl; break;
            case MF_E_TOPO_MISSING_PRESENTATION_DESCRIPTOR: std::cout << "Missing presentation descriptor" << std::endl; break;
        }
        return;
    }

    PROPVARIANT varStart;
    PropVariantInit(&varStart);
    varStart.vt = VT_EMPTY;
    result = m_pSession->Start(&GUID_NULL, &varStart);
    if (result != S_OK) std::cout << "Unable to start sound" << result << std::endl;

}



void SimplePlayer::CreatePlaybackTopology(ComPtr<IMFMediaSource> pSource, ComPtr<IMFTopology>& ppTopo) {
    
    HRESULT result = MFCreateTopology(ppTopo.GetAddressOf());
    if (FAILED(result)) {
        std::cout << "Unable to create MF topology" << result << std::endl;
        return;
    }

    ComPtr<IMFPresentationDescriptor> pPD;
    result = pSource->CreatePresentationDescriptor(pPD.GetAddressOf());
    if (FAILED(result)) {
        std::cout << "Unable to create presentation descriptor" << result << std::endl;
        return;
    }

    ComPtr<IMFStreamDescriptor> pSD;
    BOOL selected;
    pPD->GetStreamDescriptorByIndex(0, &selected, pSD.GetAddressOf());

    // Create a node for the audio stream and one for the audio renderer
    ComPtr<IMFTopologyNode> pSourceNode;
    ComPtr<IMFTopologyNode> pOutputNode;

    result = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, pSourceNode.GetAddressOf());
    if (FAILED(result)) {
        std::cout << "Unable to create source topology node " << std::hex << result << std::endl;
        return;
    }

    result = pSourceNode->SetUnknown(MF_TOPONODE_SOURCE, pSource.Get());
    if (FAILED(result))
    {
        std::cout << "Unable to set source " << std::hex << result << std::endl;
        return;
    }

    result = pSourceNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD.Get());
    if (FAILED(result))
    {
        std::cout << "Unable to set presentation descriptor " << std::hex << result << std::endl;
        return;
    }

    result = pSourceNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD.Get());
    if (FAILED(result)) {
        std::cout << "Unable to set stream descriptor" << result << std::endl;
        return;
    }





    result = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, pOutputNode.GetAddressOf());
    if (result != S_OK) std::cout << "Unable to create output topology node" << result << std::endl;

    // Create the standard Windows Audio Renderer (SAR)
    // Use IMFActivate instead of IActivate
    ComPtr<IMFActivate> pActivate;
    HRESULT hr = MFCreateAudioRendererActivate(pActivate.GetAddressOf());
    if (SUCCEEDED(hr)) {
        pOutputNode->SetObject(pActivate.Get());
    }
    else {
        std::cout << "Unable to active audio renderer" << result << std::endl;
    }

         
    ppTopo->AddNode(pSourceNode.Get());
    ppTopo->AddNode(pOutputNode.Get());
    pSourceNode->ConnectOutput(0, pOutputNode.Get(), 0);


   
    //// Start up the event generator before trying to create the topology
    //// https://learn.microsoft.com/en-us/windows/win32/medfound/mesessiontopologyset
    //ComPtr<IMFMediaEventGenerator> mediaEventGenerator;
    //m_pSession->QueryInterface<IMFMediaEventGenerator>(mediaEventGenerator.GetAddressOf());
    //// Wait for topology to be created.
    //for (; ; )
    //{
    //    ComPtr<IMFMediaEvent> MediaEvent;
    //    HRESULT hr = mediaEventGenerator->GetEvent(0, MediaEvent.GetAddressOf());
    //    MediaEventType Type;
    //    hr = MediaEvent->GetType(&Type);
    //    
    //    //std::cout << Type << std::endl;
    //    if (Type == MESessionTopologySet)
    //        break;
    //}


}


// https://learn.microsoft.com/en-us/windows/win32/medfound/service-interfaces
// https://learn.microsoft.com/en-us/windows/win32/api/mfidl/nf-mfidl-imfgetservice-getservice

void SimplePlayer::SetVolume(float level)
{
    HRESULT hr;


    ComPtr<IMFGetService> pGetService;
    hr = m_pSession->QueryInterface<IMFGetService>(pGetService.GetAddressOf());
    if (FAILED(hr)) {
        std::cout << "Unable to get service interface " << std::hex << hr << std::endl;
        return;
    }


    ComPtr<IMFSimpleAudioVolume> pVolume;

    // currently returning E_NOINERFACE
    // https://stackoverflow.com/questions/70790637/mfgetservice-doesnt-get-imfsimpleaudiovolume-on-first-try
    // https://github.com/krkrz/krkrz/blob/master/movie/win32/MFPlayer.cpp for event handling example

    hr = pGetService->GetService(MR_POLICY_VOLUME_SERVICE, IID_IMFSimpleAudioVolume, (LPVOID*)pVolume.GetAddressOf());
    if (FAILED(hr)) {
        std::cout << "Unable to get volume service " << std::hex << hr << std::endl;
        return;
    }


    hr = pVolume->SetMasterVolume(level);
    if (FAILED(hr)) {
        std::cout << "Unable to set volume level " << std::hex << hr << std::endl;
        return;
    }
}


