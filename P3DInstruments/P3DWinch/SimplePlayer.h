#pragma once
#include <windows.h>
#include <mfapi.h>     // For MFStartup, MFCreateAudioRendererActivate
#include <mfidl.h>     // For IMFMediaSession, IMFTopology, IActivate
#include <mfreadwrite.h> // For IMFSourceReader (if decoding manually)
#include <shlwapi.h>
#include <mfobjects.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "shlwapi.lib")

class SimplePlayer {
    IMFMediaSession* m_pSession = nullptr;
    IMFSourceResolver* m_pResolver = nullptr;

public:
    void Play(const wchar_t* url) {
        MFStartup(MF_VERSION);

        // 1. Create the Media Session
        MFCreateMediaSession(NULL, &m_pSession);

        // 2. Create the Media Source from URL
        IUnknown* pSourceUnk = nullptr;
        MF_OBJECT_TYPE objType;
        MFCreateSourceResolver(&m_pResolver);
        m_pResolver->CreateObjectFromURL(url, MF_RESOLUTION_MEDIASOURCE, NULL, &objType, &pSourceUnk);

        IMFMediaSource* pSource = nullptr;
        pSourceUnk->QueryInterface(IID_PPV_ARGS(&pSource));

        // 3. Create the Topology (Connecting Source to Audio Renderer)
        IMFTopology* pTopology = nullptr;
        CreatePlaybackTopology(pSource, &pTopology);

        // 4. Set Topology and Start
        m_pSession->SetTopology(0, pTopology);

        PROPVARIANT varStart;
        PropVariantInit(&varStart);
        varStart.vt = VT_EMPTY;
        m_pSession->Start(&GUID_NULL, &varStart);

        // Clean up locals
        pSource->Release();
        pTopology->Release();
    }

private:
    void CreatePlaybackTopology(IMFMediaSource* pSource, IMFTopology** ppTopo) {
        MFCreateTopology(ppTopo);
        IMFPresentationDescriptor* pPD = nullptr;
        pSource->CreatePresentationDescriptor(&pPD);

        // Create a node for the audio stream and one for the audio renderer
        IMFTopologyNode* pSourceNode = nullptr;
        IMFTopologyNode* pOutputNode = nullptr;

        MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pSourceNode);
        pSourceNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);

        MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pOutputNode);

        // Create the standard Windows Audio Renderer (SAR)
        // Use IMFActivate instead of IActivate
        IMFActivate* pActivate = nullptr;

        // This function returns an IMFActivate pointer
        HRESULT hr = MFCreateAudioRendererActivate(&pActivate);

        if (SUCCEEDED(hr)) {
            pOutputNode->SetObject(pActivate);
            pActivate->Release();
        }

        (*ppTopo)->AddNode(pSourceNode);
        (*ppTopo)->AddNode(pOutputNode);
        pSourceNode->ConnectOutput(0, pOutputNode, 0);

        pPD->Release();
        pSourceNode->Release();
        pOutputNode->Release();
    }
};