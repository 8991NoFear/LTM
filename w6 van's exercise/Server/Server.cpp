#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

const int header_len = 2;
char header[header_len] = {};

int mySend(SOCKET s, char* buf, int len, int flags) {
	// send header first
	memset(header, 0, header_len);
	sprintf(header, "%02d", len);
	if (send(s, header, header_len, flags) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	// then send actual data
	int bSent = 0;
	while (bSent < len) {
		int currentSent = send(s, buf + bSent, len - bSent, flags);
		if (currentSent == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		bSent += currentSent;
	}
	return bSent;
}

int myRecv(SOCKET s, char** buf) {
	// receive header first
	memset(header, 0, header_len);
	if (recv(s, header, header_len, 0) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	// then receive actualy data
	int msg_len = atoi(header);
	*buf = (char*) calloc(msg_len + 1, sizeof(char));
	int received = recv(s, *buf, msg_len, 0);
	return received;
}

DWORD WINAPI MyThread(LPVOID params) {
	// input data then send to and receive from server
	SOCKET s = *((SOCKET*) params);
	const int MAX = 1024;
	char* buf = NULL;
	while (1 == 1) {
		if (myRecv(s, &buf) == SOCKET_ERROR) {
			break;
		}
		int len = strlen(buf);
		char str1[MAX] = {}; int xIdx = 0;
		char str2[MAX] = {}; int yIdx = 0;
		int isErr = false;

		for (int i = 0; i < len; i++) {
			if (isalpha(buf[i])) {
				str1[xIdx++] = buf[i];
			}
			else if (isdigit(buf[i])) {
				str2[yIdx++] = buf[i];
			}
			else {
				isErr = true;
				break;
			}
		}
		free(buf); buf = NULL;

		if (isErr) {
			char msg[] = "Gui xau co ky tu khong hop le";
			if (mySend(s, msg, strlen(msg), 0) == SOCKET_ERROR) {
				break;
			}
		}
		else {
			char* msg = (char*) calloc(len+1, sizeof(char));
			sprintf(msg, "%s\n%s", str1, str2);
			if (mySend(s, msg, strlen(msg), 0) == SOCKET_ERROR) {
				break;
			}
		}
	}
	shutdown(s, SD_BOTH);
	closesocket(s);
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		return -1;
	}

	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khoi tao thu vien WinSock that bai, ma loi: %d", errCode);
		return -1;
	}

	// init socket and address
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khoi tao socket that bai, ma loi: %d", errCode);
		return -1;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;
	saddr.sin_port = htons(atoi(argv[1]));

	// become a server
	bind(s, (SOCKADDR*) &saddr, sizeof(saddr));
	listen(s, 10); // assume that baclog is 10

	// accept
	while (1 == 1) {
		int clen = sizeof(saddr);
		SOCKET cs = accept(s, (SOCKADDR*) &saddr, &clen);
		if (cs == INVALID_SOCKET) {
			int errCode = WSAGetLastError();
			printf("Khong the chap nhan ket noi tu Client, ma loi la: %d\n", errCode);
			return -1;
		}
		CreateThread(NULL, 0, MyThread, (VOID*) &cs, 0, NULL);
	}

	// clean up everything
	shutdown(s, SD_BOTH);
	closesocket(s);
	WSACleanup();
	return 0;
}