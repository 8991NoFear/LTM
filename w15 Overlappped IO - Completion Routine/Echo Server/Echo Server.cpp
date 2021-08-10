#include <stdio.h>
#include <WinSock2.h>

#define LISTEN_PORT 8888
#define BACKLOG 10

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	SOCKADDR_IN listenAddr;
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(LISTEN_PORT);
	listenAddr.sin_addr.S_un.S_addr = htonl(ADDR_ANY);

	if (bind(listenSocket, (SOCKADDR*)&listenAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	if (listen(listenSocket, BACKLOG) == SOCKET_ERROR) {
		printf("Error occurs: %d", WSAGetLastError());
	}

	WSAEVENT listenEvent = WSACreateEvent();
	WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT);

	while (TRUE) {
		int index = WSAWaitForMultipleEvents(1, &listenEvent, FALSE, INFINITE, FALSE);
		index -= WSA_WAIT_EVENT_0;
		if (index == 0) {
			WSANETWORKEVENTS networkEvents;
			WSAEnumNetworkEvents(listenSocket, listenEvent, &networkEvents);
			if (networkEvents.lNetworkEvents == FD_ACCEPT) {
				if (networkEvents.iErrorCode[FD_ACCEPT_BIT] == 0) {
					SOCKADDR_IN acceptAddr;
					int acceptAddrlen = sizeof(SOCKADDR_IN);
					SOCKET acceptSocket = accept(listenSocket, (SOCKADDR*)&acceptAddr, &acceptAddrlen);
				}
			}
			WSAResetEvent(listenEvent); // reset event
		}
	}
}