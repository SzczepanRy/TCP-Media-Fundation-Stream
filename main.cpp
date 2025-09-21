
// g++ main.cpp -o main.exe -lole32 -lmfplat -lmf -lmfreadwrite -luuid -lshlwapi
// -lwmcodecdspuuid -lws2_32

#include "serv.hpp"
#include "video.hpp"
/*
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
                                           */
int main() {

  Server server;
  VideoCapture video;

  return 0;
}
