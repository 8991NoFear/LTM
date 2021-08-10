#include <stdio.h>
#include <WinSock2.h>

#define BACKLOG 10

int g_count = 0;
SOCKET g_sockets[FD_SETSIZE] = {};

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	SOCKADDR_IN saddrin;
	saddrin.sin_family = AF_INET;
	saddrin.sin_port = htons(8888);
	saddrin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (SOCKADDR*)&saddrin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	if (listen(listenSocket, BACKLOG) == SOCKET_ERROR) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	fd_set fdreads;
	while (true) {
		FD_ZERO(&fdreads);
		FD_SET(listenSocket, &fdreads);
		for (int i = 0; i < g_count; i++) {
			FD_SET(g_sockets[i], &fdreads);
		}
		
		select(g_count + 1, &fdreads, NULL, NULL, NULL); // hàm select() có tác động làm thay đổi tập fdreads
		// sau khi đi ra khỏi hàm select() thì tập fdreads chỉ toàn chứa các socket xảy ra sự kiện FD_READ 

		if (FD_ISSET(listenSocket, &fdreads)) {
			SOCKADDR_IN acceptADDR;
			int acceptADDRLen = sizeof(SOCKADDR_IN);
			SOCKET acceptSocket = accept(listenSocket, (SOCKADDR*)&acceptADDR, &acceptADDRLen);
			g_sockets[g_count] = acceptSocket;
			g_count++;
			printf("socket s is in set. ");
		}
		else {
			printf("socket s is not in set. ");
		}
		for (int i = 0; i < g_count; i++) {
			if (FD_ISSET(g_sockets[i], &fdreads)) {
				printf("socket g_sockets[%d] is in set. ", i);
				char buffer[1024] = {};
				// socket có dữ liệu mà ko gọi recv() thì lần sau FD_SET nó vào tập fdreads thì giống như sk FD_READ đã xảy ra sẵn
			}	
			else {
				printf("socket g_sockets[%d] is not in set. ", i);
			}
		}
		printf("\n");
	}
}