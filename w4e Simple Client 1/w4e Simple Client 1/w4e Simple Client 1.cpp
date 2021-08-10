#include <stdio.h>
#include <WinSock2.h>
#include <regex>
#include <WS2tcpip.h>

bool isValidDomain(std::string domain) {
	const std::regex domain_pattern("^(?!-)[A-Za-z0-9-]+([\\-\\.]{1}[a-z0-9]+)*\\.[A-Za-z]{2,6}$");
	if (std::regex_match(domain, domain_pattern)) {
		return true;
	}
	return false;
}

bool isValidIPv4(std::string ipv4) {
	const std::regex ipv4_pattern("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
	if (std::regex_match(ipv4, ipv4_pattern)) {
		return true;
	}
	return false;
}

bool isValidPort(char *port) {
	int p = atoi(port);
	return (p > 0 && p < 65536) ? true : false;
}

long resolveDomain(std::string domain) {
	SOCKADDR_IN res{};
	PADDRINFOA addrinf = NULL;
	getaddrinfo("vnexpress.net", "http", NULL, &addrinf);
	PADDRINFOA rootAddrinf = addrinf;
	while (addrinf != NULL) {
		if (addrinf->ai_family == AF_INET) {
			memcpy(&res, addrinf->ai_addr, addrinf->ai_addrlen);
			break;
		}
		addrinf = addrinf->ai_next;
	}
	freeaddrinfo(rootAddrinf);
	return res.sin_addr.S_un.S_addr;
}

int mySend(SOCKET s, char* data, int len, int flags) {
	int bytesSent = 0;
	while (bytesSent < len) {
		bytesSent += send(s, data, len, flags);
	}
	return bytesSent;
}

int main(int argc, char* argv[]) {
	// 2) khoi tao thu vien WinSock
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi la: %d\n", errCode);
		return -1;
	}

	// 1) check tham so
	bool check = true;
	if (argc != 3)
	{
		check = false;
	}

	long ipv4 = 0;
	short port = 0;
	
	if (isValidDomain(std::string(argv[1])))
	{
		std::string str_domain(argv[1]);
		ipv4 = resolveDomain(str_domain);
	}
	else if (isValidIPv4(std::string(argv[1]))) {
		ipv4 = inet_addr(argv[1]);
	}
	else {
		check = false;
	}

	if (isValidPort(argv[2])) {
		port = htons(atoi(argv[2]));
	}
	else {
		check = false;
	}

	if (!check) {
		printf("Tham so phai co dang <dia chi ip/ten mien> <cong>\n");
		return -1;
	}

	// 2) khoi tao socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi la: %d\n", errCode);
		return -1;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = port;
	saddr.sin_addr.S_un.S_addr = ipv4;
	
	// 3) ket noi
	int connRes = connect(s, (SOCKADDR*) &saddr, sizeof(saddr));
	if (connRes == SOCKET_ERROR) {
		int errCode = WSAGetLastError();
		printf("Khong the ket noi toi server, ma loi la: %d\n", errCode);
		return -1;
	}

	// 4) lay du lieu tu ban phim va gui len server
	char data[1024] = {};
	gets_s(data);
	int bytesSent = mySend(s, data, strlen(data), 0);
	printf("Da gui %d bytes\n", bytesSent);
	
	// clean up everthing
	closesocket(s);
	WSACleanup();
	return 0;
}