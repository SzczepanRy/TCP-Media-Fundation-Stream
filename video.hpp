#pragma once

#include "serv.hpp"

#include <initguid.h>
#include <iostream>
#include <mfapi.h>       // Media Foundation basic API
#include <mferror.h>     // MF HRESULT error codes
#include <mfidl.h>       // Interfaces like IMFMediaSource, IMFTransform
#include <mfobjects.h>   // COM objects for MF
#include <mfreadwrite.h> // For sink writers (writing H.264 to MP4)
#include <shlwapi.h>     // Optional: path utilities
#include <synchapi.h>
#include <windows.h> // Core Windows types

#pragma comment(lib, "mfplat.lib")         // Core Media Foundation
#pragma comment(lib, "mf.lib")             // Media Foundation
#pragma comment(lib, "mfreadwrite.lib")    // Reading/writing media files
#pragma comment(lib, "mfuuid.lib")         // Media Foundation GUIDs
#pragma comment(lib, "shlwapi.lib")        // Optional, for path handling
#pragma comment(lib, "wmcodecdspuuid.lib") // For H.264 encoder CLSID

template <typename T>
void SafeRelease(T **ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = nullptr;
  }
};



class VideoCapture {
public:
  // video source
  IMFMediaSource *pSource = nullptr;
  // reading source
  IMFSourceReader *pReader = nullptr;
  // output type
  IMFMediaType *pType = nullptr;


  VideoCapture() {

    Server server;

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
      std::cout << "error on startup";
      abort();
    }
    // video source
    hr = CreateVideoDeviceSource(&pSource);
    if (FAILED(hr)) {
      std::cout << "create video device source faled";
      abort();
    }
    // reading source
    hr = MFCreateSourceReaderFromMediaSource(
        pSource, nullptr, &pReader); // prepares for reading frames
    if (FAILED(hr)) {
      std::cout << "Failed to create SourceReader: " << std::hex << hr << "\n";
      SafeRelease(&pSource);
    }

    // media output media type ,
    SettingOutputType();

    // capture video frame
    //
    //
    while (true) {
      std::cout << " capture video frame\n";
      IMFSample *pSample = nullptr;
      DWORD streamIndex, flags;
      LONGLONG timestamp;
      hr = pReader->ReadSample(
          (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
          0, // read sample reads one frame form the video stream
          &streamIndex, &flags, &timestamp, &pSample);
      /*
       * (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM - the first video stream
       * index 0 - flags (none) &stream index , whitch stream returned the
       * sample frame &flags metadata like endOfStream &timestamp - fame
       * timestamp in 100-nanoseconds units &pSample - the actual frame sample
       * object after this call , pSample holds a single video frame
       */
      if (FAILED(hr)) {
        std::cout << "ReadSample failed: " << std::hex << hr << "\n";
      } else if (flags & MF_SOURCE_READERF_STREAMTICK) {
        // No new frame yet
      } else if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
        std::cout << "End of stream reached\n";
      } else if (pSample) {
        std::cout << "succeded in loading frame \n";
        IMFMediaBuffer *pBuffer = nullptr;
        pSample->ConvertToContiguousBuffer(
            &pBuffer); // some frames may have multiple buffers internally; this
                       // consolidates them into one contiguous buffer. (fucked
                       // this thing is)

        BYTE *pData = nullptr;
        DWORD maxLength = 0, currentLength = 0;
        pBuffer->Lock(&pData, &maxLength,
                      &currentLength); // gives us a pointer (pData) to the raw
                                       // pixel data.

        // pData now contains the raw frame
        std::cout << "Captured frame of " << currentLength << " bytes\n";
        server.sendData(pData, currentLength);

        pBuffer->Unlock();
        SafeRelease(&pBuffer);
        SafeRelease(&pSample);
      }
    }
  }

  ~VideoCapture() {
    SafeRelease(&pSource);
    MFShutdown();
  }
  void SettingOutputType() { // this could not be right
    MFCreateMediaType(&pType);
    pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video); // video frames
    pType->SetGUID(
        MF_MT_SUBTYPE,
        MFVideoFormat_NV12); // NV12 format , preffared for h264 encoding
    pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                 nullptr, pType);
    SafeRelease(&pType);
  }

  HRESULT CreateVideoDeviceSource(IMFMediaSource **ppSource) {
    *ppSource = NULL;

    IMFMediaSource *pSource = NULL;
    IMFAttributes *pAttributes = NULL;
    IMFActivate **ppDevices = NULL;

    // Create an attribute store to specify the enumeration parameters.
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr)) {
      goto done;
    }

    // Source type: video capture devices
    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                              MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
      goto done;
    }

    // Enumerate devices.
    UINT32 count;
    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
    if (FAILED(hr)) {
      goto done;
    }

    if (count == 0) {
      hr = E_FAIL;
      goto done;
    }

    // Create the media source object.
    hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pSource));
    if (FAILED(hr)) {
      goto done;
    }

    *ppSource = pSource;
    (*ppSource)->AddRef();

  done: // realece the interface pointers , and free the mem for the arr
    SafeRelease(&pAttributes);

    for (DWORD i = 0; i < count; i++) {
      SafeRelease(&ppDevices[i]);
    }
    CoTaskMemFree(ppDevices);
    SafeRelease(&pSource);
    return hr;
  }
};
