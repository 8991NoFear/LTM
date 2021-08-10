#include <stdio.h>
#include <WinSock2.h>

#define N 2 // so luong socket toi da trong 1 luong

SOCKET *clients = NULL;
int connCount = 0; // so luong ket noi, TODO: xl xung dot giua cac luong

DWORD WINAPI myThread(LPVOID params) {
	fd_set fdread;
	int startIdx =  (int) params;
	int numConn = 0;
	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	printf("current thread id: %d, start index: %d\n", GetCurrentThreadId(), startIdx);
	while (true) {
		FD_ZERO(&fdread);
		numConn = connCount - startIdx;
		numConn = (numConn > N) ? N : numConn;
		for (int i = startIdx; i < startIdx + numConn; i++) {
			FD_SET(clients[i], &fdread);
		}
		select(0, &fdread, NULL, NULL, &tv); // trong thoi gian bi treo thi tap fdread chua update
		for (int i = startIdx; (i < startIdx + N); i++) {
			if (FD_ISSET(clients[i], &fdread)) {
				char buffer[1024] = {};
				recv(clients[i], buffer, 1024, 0);
				for (int j = 0; j < connCount; j++) {
					if (j != i) {
						send(clients[j], buffer, strlen(buffer), 0);
					}
				}
			}
		}
	}
	return 0;
}

int main() {
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi: %d\n", errCode);
		return -1;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi: %d\n", errCode);
		return -1;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = ADDR_ANY;

	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);

	fd_set fdread;
	while (true) {
		FD_ZERO(&fdread);
		FD_SET(s, &fdread);
		select(0, &fdread, NULL, NULL, NULL);
		if (FD_ISSET(s, &fdread)) {
			int clen = sizeof(saddr);
			SOCKET tmp = accept(s, (sockaddr*)&saddr, &clen); // muon dung ipv6 thi phai dung cau truc sockaddr_storage
			clients = (SOCKET*)realloc(clients, (connCount + 1) * sizeof(SOCKET));
			clients[connCount] = tmp;
			if (connCount%N == 0) {
				CreateThread(NULL, 0, &myThread, (LPVOID)connCount, 0, NULL);
			}
			connCount++;
		}
	}
}
