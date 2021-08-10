#include <stdio.h>
#include <WinSock2.h>

int mySend(SOCKET s, char* buf, int len, int flags) {
	int bytesSent = 0;
	while (bytesSent < len) {
		bytesSent += send(s, buf + bytesSent, len - bytesSent, flags);
	}
	return bytesSent;
}

int myRecv(SOCKET s, char** buf, int flags) {
	const int MAX = 1024;
	int bytesReceived = 0;
	int totalBytesReceived = 0;
	char tmp[MAX];
	do {
		bytesReceived = recv(s, tmp, MAX, flags); // moi lan nhan MAX bytes --> bi treo neu client gui mot so nguyen lan MAX
		if (bytesReceived > 0) {
			*buf = (char*)realloc(*buf, totalBytesReceived + bytesReceived + 1); // doi ra 1 byte cho ky tu \0
			// buf = (char**) realloc(*buf, totalBytesReceived + bytesReceived + 1); SAI
			memset(*buf + totalBytesReceived, 0, bytesReceived + 1);
			memcpy(*buf + totalBytesReceived, tmp, bytesReceived);
			totalBytesReceived += bytesReceived;
		}
	} while (bytesReceived == MAX); // ang chung neu nhan duoc MAX bytes thi co the nhan duoc tiep
	return totalBytesReceived;
}

int main() {
	// 1) khoi tao thu vien WinSock
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi la: %d", errCode);
		return -1;
	}

	// 2) khoi tao socket nghe
	SOCKET ls = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ls == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao duoc socket, ma loi la: %d\n", errCode);
		return -1;
	}

	// 3) dien thong tin dia chi
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // lang nghe tren tat ca cac card mang

	// 4) rang buoc socket nghe voi thong tin dia chi
	bind(ls, (SOCKADDR*)&saddr, sizeof(saddr));

	// 5) cho ket noi
	listen(ls, 10);

	// 6) chap nhan ket noi tu client
	SOCKADDR_IN caddr;
	int clen = sizeof(caddr); // IN: so bytes dc phep dung & OUT: 
	printf("before: clen = %d\n", clen);
	SOCKET cs = accept(ls, (SOCKADDR*)&caddr, &clen);
	if (cs == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao duoc socket, ma loi la: %d\n", errCode);
		return -1;
	}
	printf("after: clen = %d\n", clen);

	// 7) gui du lieu cho client
	char hello[] = "Hello Network Programming!\n";
	int bytesSent = mySend(cs, hello, sizeof(hello), 0);
	printf("Da gui: %d bytes\n", bytesSent);

	// 8) nhan du lieu tu 
	//char data[1024] = {}; // co the ben gui khong gui byte \0
	// memset(data, 0, sizeof(data));
	char* data = NULL;
	int bytesReceived = myRecv(cs, &data, 0);
	printf("Da nhan: %d bytes\n", bytesReceived);
	printf("Du lieu da nhan: %s\n", data);
	free(data); data = NULL;

	// clean up everything
	closesocket(ls);
	closesocket(cs);
	WSACleanup();
	return 0;
}