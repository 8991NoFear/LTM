#include <iostream>
#include <vector>
#include <WinSock2.h>
#include <Windows.h>

#define M_LISTEN_PORT 8888
#define M_TIMEOUT 64
#define M_BUFFER_SIZE 1024

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (s == INVALID_SOCKET) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(M_LISTEN_PORT);
	saddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (connect(s, (SOCKADDR*)&saddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	WSABUF buffer;
	char buf[M_BUFFER_SIZE] = {};
	buffer.buf = buf;
	buffer.len = M_BUFFER_SIZE;
	DWORD bytesRecv = 0;
	DWORD flags = 0;
	OVERLAPPED overlap;
	ZeroMemory(&overlap, sizeof(OVERLAPPED));
	WSAEVENT event = WSACreateEvent();
	overlap.hEvent = event;
	if (WSARecv(s, &buffer, 1, &bytesRecv, &flags, &overlap, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			exit(-1);
		}
	}

	while (TRUE) {
		if (WSAWaitForMultipleEvents(1, &event, FALSE, M_TIMEOUT, FALSE) == WSA_WAIT_TIMEOUT) {
			continue;
		}
		WSAResetEvent(event);
		WSAGetOverlappedResult(s, &overlap, &bytesRecv, FALSE, &flags);
		if (bytesRecv != 0) {
			buffer.buf[bytesRecv] = 0;
			printf("Data received: %s", buffer.buf);
			if (WSARecv(s, &buffer, 1, &bytesRecv, &flags, &overlap, NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					exit(-1);
				}
			}
		}
		else {
			printf("Server is down\n");
			closesocket(s);
			exit(0);
		}
	}
}