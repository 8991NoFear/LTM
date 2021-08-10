#include <stdio.h>
#include <WinSock2.h>

WSABUF databuf;
char buffer[1024] = {};
DWORD bytesReceived = 0;
DWORD flags = 0;
OVERLAPPED overlapped;
SOCKET s;

void CALLBACK MyCompletionRoutine(IN DWORD dwError, IN DWORD cbTransferred, IN LPWSAOVERLAPPED lpOverlapped, IN DWORD dwFlags);

// CLIENT PROGRAM
int main() {
    WSADATA wsaData;
    int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupRes != 0) {
        printf("Error occurs: %d", WSAGetLastError());
        exit(-1);
    }

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        printf("Error occurs: %d", WSAGetLastError());
        exit(-1);
    }

    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    if (connect(s, (sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
        printf("Error occurs: %d", WSAGetLastError());
        exit(-1);
    }

    databuf.buf = buffer;
    databuf.len = sizeof(buffer);
    memset(&overlapped, 0, sizeof(overlapped));
    WSARecv(s, &databuf, 1, &bytesReceived, &flags, &overlapped, &MyCompletionRoutine); // tham so thu 3 la so luong buffer
    while (TRUE) { // vi do cac ham WSA deu la bat dong bo --> khong duoc ket thuc chuong trinh ngay
        SleepEx(1000, TRUE); // ngu nhung van danh thuc luong neu co thao tac vao ra
    }
}

void CALLBACK MyCompletionRoutine(IN DWORD dwError, IN DWORD cbTransferred, IN LPWSAOVERLAPPED lpOverlapped, IN DWORD dwFlags) {
    if (dwError == 0) { // khong co loi
        printf("Number of bytes received: %lu\n", cbTransferred); // unsinged long --> "%lu"
        printf("Data: %s", buffer);

        memset(buffer, 0, sizeof(buffer));
        WSARecv(s, &databuf, 1, &bytesReceived, &flags, &overlapped, &MyCompletionRoutine);
    }
}