#include <stdio.h>
#include <WinSock2.h>

int main() {
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2, ), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi la: %d\n", errCode);
		return 0;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s != INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi la: %d\n", errCode);
		return 0;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = ADDR_ANY;

	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);

	// thang struct fd_set nay duoc typedef rat mat day :))
	// fd_set gioi han 64 socket thui
	fd_set fdread;
	
	while (true) {
		FD_ZERO(&fdread); // may cai FD_abcxyz la macro
		FD_SET(s, &fdread);
		select(0, &fdread, NULL, NULL, NULL); // NULL o truong timeout thi doi mai mai
		if (FD_ISSET(s, &fdread)) {
			int clen = sizeof(saddr);
			SOCKET c = accept(s, (sockaddr*)&saddr, &clen);
		}
	}

	return 0;
}