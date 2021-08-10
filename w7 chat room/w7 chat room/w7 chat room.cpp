#include <stdio.h>
#include <WinSock2.h>

#define MAX 1024

SOCKET clients[MAX] = {};

int connCount = 0; // so luong ket noi

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
		FD_ZERO(&fdread); // xoa het socket trong tap tham do
		FD_SET(s, &fdread); // dua s vao tap tham do
		for (int i = 0; i < connCount; i++) {
			FD_SET(clients[i], &fdread);
		}

		select(0, &fdread, NULL, NULL, NULL); // tra ve so luong socket co su kien, treo tai day
		if (FD_ISSET(s, &fdread)) {
			int clen = sizeof(saddr);
			SOCKET tmp = accept(s, (sockaddr*)&saddr, &clen); // muon dung ipv6 thi phai dung cau truc sockaddr_storage
			clients[connCount++] = tmp;
		}

		for (int i = 0; i < connCount; i++) {
			if (FD_ISSET(clients[i], &fdread)) {
				char buffer[MAX] = {};
				recv(clients[i], buffer, MAX, 0);
				for (int j = 0; j < connCount; j++) {
					if (j != i) {
						send(clients[j], buffer, strlen(buffer), 0);
					}
				}
			}
		}
	}
}

// dang dung lai o 22'30"
