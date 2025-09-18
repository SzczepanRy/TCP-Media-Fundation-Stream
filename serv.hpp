#pragma once

#include <winsock2.h>

#include <iostream>
#include <minwindef.h>
#include <ws2tcpip.h>

class Server {
public:
  WSADATA wsaData;
  SOCKET sock;
  sockaddr_in serverAddr;

  Server() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      std::cout << "WSAStartup failed\n";
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
      std::cout << "Socket creation failed\n";
      WSACleanup();
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // server port
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
        SOCKET_ERROR) {
      std::cout << "Connect failed\n";
      closesocket(sock);
      WSACleanup();
    }
  }

  void sendData(BYTE *pData, DWORD currentLength) {
    if (!pData || currentLength == 0)
      return; // safety check

    uint32_t frameSize = htonl(currentLength);

    // Check return values of send
    int sent =
        send(sock, reinterpret_cast<char *>(&frameSize), sizeof(frameSize), 0);
    if (sent == SOCKET_ERROR) {
      std::cerr << "Failed to send frame size\n";
      return;
    }

    sent = send(sock, reinterpret_cast<char *>(pData), currentLength, 0);
    if (sent == SOCKET_ERROR) {
      std::cerr << "Failed to send frame data\n";
      return;
    }
  }

  ~Server() {
    closesocket(sock);
    WSACleanup();
  }
};
