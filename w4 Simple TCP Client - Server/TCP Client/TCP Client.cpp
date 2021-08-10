#include <stdio.h>
#include <WinSock2.h>

int mySend(SOCKET s, char* buf, int len, int flag) {
	int bytesSent = 0;
	while (bytesSent < len) {
		bytesSent += send(s, buf + bytesSent, len - bytesSent, flag);
	}
	return bytesSent;
}

int myRecv(SOCKET s, char** buf, int flags) {
	int totalBytesReceived = 0;
	int bytesReceived = 0;
	const int MAX = 1024;
	char tmp[MAX] = {};
	do {
		bytesReceived = recv(s, tmp, MAX, flags); // bi treo neu so bytes server gui bang 1 so nguyen lan MAX
		if (bytesReceived > 0) {
			*buf = (char*)realloc(*buf, totalBytesReceived + bytesReceived + 1);
			memset(*buf + totalBytesReceived, 0, bytesReceived + 1);
			memcpy(*buf + totalBytesReceived, tmp, bytesReceived);
			totalBytesReceived += bytesReceived;
		}
	} while (bytesReceived == MAX); // gia dinh nhan duoc MAX bytes thi co the nhan duoc tiep
	return totalBytesReceived;
}

int main() {
	// 1) khoi tao thu vien WinSock
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi la: %d\n", errCode);
		return -1;
	}

	// 2) khoi tao socket va dien thong tin dia chi cua server
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) { // o day phai check la INVALID_SOCKET chu khong phai SOCKET_ERROR
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi la: %d\n", errCode);
		return -1;
	}
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(7777);
	saddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// 3) ket noi den server
	int connRes = connect(s, (SOCKADDR*)&saddr, sizeof(saddr));
	if (connRes == SOCKET_ERROR) { // o day phai check la SOCKET_ERROR chu khong phai INVALID_SOCKET
		int errCode = WSAGetLastError();
		printf("Khong the ket noi toi server, ma loi la: %d\n", errCode);
		return -1;
	}

	// 4) gui du lieu
	char* hello = (char*)"Hello Network Programming!";
	int bytesSent = mySend(s, hello, strlen(hello), 0);
	printf("Da gui %d bytes\n", bytesSent);

	// 5) nhan du lieu
	char* data = NULL;
	int bytesReceived = myRecv(s, &data, 0);
	printf("Da nhan %d bytes\n", bytesReceived);
	printf("Du lieu nhan: %s\n", data);
	free(data); data = NULL;

	// ) clean up everything
	closesocket(s);
	WSACleanup();
	return 0;
}