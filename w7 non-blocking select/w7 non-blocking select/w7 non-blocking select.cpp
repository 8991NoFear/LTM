#include <stdio.h>
#include <WinSock2.h>


int main() {
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khoi tao WinSock khong thanh cong, ma loi la: %d", errCode);
		return -1;
	}

	SOCKET c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi: %d\n", errCode);
		return -1;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = inet_addr("10.0.0.1");

	// chuyen sang che do non-blocking
	unsigned long argp = 1;
	int result = ioctlsocket(c, FIONBIO, &argp);

	SOCKET r = connect(c, (sockaddr*)&saddr, sizeof(saddr));

	// chuyen sang che do blocking --> tu kiem soat timeout
	argp = 0;
	result = ioctlsocket(c, FIONBIO, &argp);

	fd_set fdwrite, fderr;
	FD_ZERO(&fdwrite);
	FD_ZERO(&fderr);
	FD_SET(c, &fdwrite);
	FD_SET(c, &fderr);
	timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	select(0, NULL, &fdwrite, &fderr, &timeout);
	if (FD_ISSET(c, &fdwrite)) {
		printf("Ket noi thanh cong");
		closesocket(c);
	} else {
		printf("Ket noi khong thanh cong");
	}
}