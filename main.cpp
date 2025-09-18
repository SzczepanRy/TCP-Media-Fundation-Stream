
//g++ main.cpp -o main.exe -lole32 -lmfplat -lmf -lmfreadwrite -luuid -lshlwapi -lwmcodecdspuuid -lws2_32

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
#include <windows.h>     // Core Windows types

#pragma comment(lib, "mfplat.lib")         // Core Media Foundation
#pragma comment(lib, "mf.lib")             // Media Foundation
#pragma comment(lib, "mfreadwrite.lib")    // Reading/writing media files
#pragma comment(lib, "mfuuid.lib")         // Media Foundation GUIDs
#pragma comment(lib, "shlwapi.lib")        // Optional, for path handling
#pragma comment(lib, "wmcodecdspuuid.lib") // For H.264 encoder CLSID

template <typename T> // making a generic com interface type
void SafeRelease(T **ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = nullptr;
  }
} // realeace COM objects , sets to nullptr to avoid dangeling pointers
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

int main() {

  Server server;

  // Initialize Media Foundation
  HRESULT hr = MFStartup(MF_VERSION);
  if (FAILED(hr)) {
    std::cout << "error on startup";
    return 0;
  }

  IMFMediaSource *pSource = nullptr;
  hr = CreateVideoDeviceSource(&pSource);
  if (SUCCEEDED(hr)) {
    std::cout << "Video device source created!\n";

    // read the source ,
    IMFSourceReader *pReader = nullptr;
    hr = MFCreateSourceReaderFromMediaSource(
        pSource, nullptr, &pReader); // prepares for reading frames
    if (FAILED(hr)) {
      std::cout << "Failed to create SourceReader: " << std::hex << hr << "\n";
      SafeRelease(&pSource);
      return -1;
    }

    // seting the media output media type ,
    std::cout << "seting the media output media type ,\n";
    //
    //
    while (true) {
      Sleep(50);

      IMFMediaType *pType = nullptr;
      MFCreateMediaType(&pType);
      pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video); // video frames
      pType->SetGUID(
          MF_MT_SUBTYPE,
          MFVideoFormat_NV12); // NV12 format , preffared for h264 encoding
      pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                   nullptr, pType);
      SafeRelease(&pType);

      // capture video frame
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
  } else {
    std::cout << "Failed to create video device source: " << std::hex << hr
              << "\n";
  }
  SafeRelease(&pSource);

  // Later, when done:
  MFShutdown();

  return 0;
}
