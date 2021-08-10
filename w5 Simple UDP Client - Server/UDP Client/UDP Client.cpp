#include <stdio.h>
#include <WinSock2.h>

int main() {
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi: %d", errCode);
		return -1;
	}

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi: %d", errCode);
		return -1;
	}

	char* welcome = (char*)"Hello UDP Programming\n";
	SOCKADDR_IN toAddr;
	toAddr.sin_family = AF_INET;
	toAddr.sin_port = htons(8888);
	toAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	int bytesSent = sendto(s, welcome, strlen(welcome), 0, (SOCKADDR*)&toAddr, sizeof(toAddr));
	printf("Da gui %d bytes", bytesSent);
}