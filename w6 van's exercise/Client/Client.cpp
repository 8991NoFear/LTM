#include <stdio.h>
#include <WinSock2.h>


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

int myRecv(SOCKET s) {
	// receive header first
	memset(header, 0, header_len);
	if (recv(s, header, header_len, 0) == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	// then receive actual data
	int msg_len = atoi(header);
	char* msg = (char*) calloc(msg_len + 1, sizeof(char));
	int received = recv(s, msg, msg_len, 0);
	printf("%s\n", msg);
	return received;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
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
	saddr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	saddr.sin_port = htons(atoi(argv[2]));

	// connect to server
 	int connRes = connect(s, (sockaddr*) &saddr, sizeof(saddr));
	if (connRes == SOCKET_ERROR) {
		int errCode = WSAGetLastError();
		printf("Ket noi toi server that bai, ma loi: %d", errCode);
		return -1;
	}

	// input data then send to and receive from server
	const int MAX = 1024; // limit to 1024 bytes each input
	char buf[MAX] = {};
	while (strlen(gets_s(buf)) != 0) {
		if (mySend(s, buf, strlen(buf), 0) == SOCKET_ERROR) {
			break;
		}
		if (myRecv(s) == SOCKET_ERROR) {
			break;
		}
	}

	// clean up everything
	shutdown(s, SD_BOTH);
	closesocket(s);
	WSACleanup();
	return 0;
}