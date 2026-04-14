#pragma once
#include <windows.h>
#include <mfapi.h>     // For MFStartup, MFCreateAudioRendererActivate
#include <mfidl.h>     // For IMFMediaSession, IMFTopology, IActivate
#include <mfreadwrite.h> // For IMFSourceReader (if decoding manually)
#include <shlwapi.h>
#include <mfobjects.h>

#include <iostream>
#include "../P3DCommon/WideConverter.h"
#include "com_ptr.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "shlwapi.lib")

class SimplePlayer {
    ComPtr<IMFMediaSession> m_pSession;
    ComPtr<IMFSourceResolver>* m_pResolver;

public:

    SimplePlayer();
    ~SimplePlayer();

    void Play(const char* uri) {
        std::wstring str = s2ws(uri);
        Play(str.c_str());
    }

    void Play(const std::string& uri) {
        std::cout << "Playing " << uri << std::endl;
        std::wstring str = s2ws(uri);
        Play(str.c_str());
    }

    HRESULT CreateMediaSource(const wchar_t* url, ComPtr<IMFMediaSource>& ppSource);
    void Play(const wchar_t* url);

    void SetVolume(float volume = 1.0f);

private:
   
    void CreatePlaybackTopology(ComPtr<IMFMediaSource> pSource, ComPtr<IMFTopology>& ppTopo);
};