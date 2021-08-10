#include <stdio.h>
#include <WinSock2.h>

#define BACKLOG 10

int main() {
	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) {
		printf("Error occurs: %d", WSAGetLastError());
		exit(-1);
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		printf("Error occurs: %d", WSAGetLastError());
		exit(-1);
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.S_un.S_addr = ADDR_ANY;
	saddr.sin_port = htons(8888);

	if (bind(s, (sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
		printf("Error occurs: %d", WSAGetLastError());
		exit(-1);
	}

	if (listen(s, BACKLOG) == SOCKET_ERROR) {
		printf("Error occurs: %d", WSAGetLastError());
		exit(-1);
	}

	SOCKADDR_IN csaddr;
	int clen = sizeof(csaddr);
	SOCKET cs = accept(s, (sockaddr*)&csaddr, &clen);

	// LISTEN SOCKET (s)
	SOCKADDR_IN addr;
	int len = sizeof(addr);
	getsockname(s, (sockaddr*)&addr, &len);
	printf("local name of listen socket: %s:%d\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	getpeername(s, (sockaddr*)&addr, &len);
	printf("peer name of listen socket: %s:%d\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	// DATA SOCKET (cs)
	getsockname(cs, (sockaddr*)&addr, &len);
	printf("local name of data socket: %s:%d\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	getpeername(cs, (sockaddr*)&addr, &len);
	printf("peer name of data socket: %s:%d\r\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	//closesocket(s);
	char buffer[1024] = {};
	sprintf(buffer, "Welcome Client %s:%d\r\n", inet_ntoa(csaddr.sin_addr), ntohs(csaddr.sin_port));
	printf("%s", buffer);
	send(cs, buffer, strlen(buffer), 0);

	WSACleanup();
	return 0;
}