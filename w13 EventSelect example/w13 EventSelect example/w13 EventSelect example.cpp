#include <stdio.h>
#include <WinSock2.h>

#define MAX_CLIENT 1024

WSAEVENT g_events[MAX_CLIENT] = {}; // WSAEVENT chinh la HANDLE ma HANDLE chinh la con tro void
SOCKET g_sockets[MAX_CLIENT] = {};
int g_count = 0;

int main() {
    WSADATA wsaData;
    int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupRes != 0) {
        int errCode = WSAGetLastError();
        printf("Khong the khoi tao thu vien WinSock, ma loi: %d", errCode);
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        printf("Khong the khoi tao socket, ma loi: %d", errCode);
    }

    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.S_un.S_addr = ADDR_ANY;

    if (bind(s, (sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        printf("Khong the bind socket, ma loi: %d", errCode);
    }

    if (listen(s, 10) == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        printf("Khong the listen, ma loi: %d", errCode);
    }

    g_events[g_count] = WSACreateEvent();
    g_sockets[g_count] = s;
    /*
    * moi socket phai anh xa tuong ung voi 1 event
    * sk tham so thu 3 xay ra o socket tham so thu 1 se lam event tham so thu 2 "mo"
    */
    WSAEventSelect(s, g_events[g_count], FD_ACCEPT); // anh xa socket voi event
    g_count++;

    while (true) {
        /*
        * toi da 64 socket --> phai ket hop voi luong (giong nhu select)
        * treo tai day mai mai (INFINITE) cho den khi co it nhat 1 event "mo"
        */
        int index = WSAWaitForMultipleEvents(g_count, g_events, false, INFINITE, false);
        index -= WSA_WAIT_EVENT_0; // bh index la chi so event nho nhat trong mang events[]
        for (int i = index; i < g_count; i++) { // co nhung event nao "mo"
            WSANETWORKEVENTS networkEvent;
            WSAEnumNetworkEvents(g_sockets[i], g_events[i], &networkEvent);
            WSAResetEvent(g_events[i]); // chuyen event ve trang thai "dong"
            if (networkEvent.lNetworkEvents & FD_ACCEPT) { // co su kien accept tren socket g_sockets[i] hay khong
                if (networkEvent.iErrorCode[FD_ACCEPT_BIT] == 0) { // co xay ra loi hay khong
                    SOCKADDR_IN caddr;
                    int clen = sizeof(caddr);
                    SOCKET c = accept(g_sockets[i], (sockaddr*)&caddr, &clen);
                    HANDLE e = WSACreateEvent();
                    g_sockets[g_count] = c;
                    g_events[g_count] = e;
                    WSAEventSelect(g_sockets[g_count], g_events[g_count], FD_READ | FD_CLOSE);
                    g_count++;
                }
            }
            else if (networkEvent.lNetworkEvents & FD_READ) {
                if (networkEvent.iErrorCode[FD_READ_BIT] == 0) {
                    char buffer[1024] = {};
                    recv(g_sockets[i], buffer, 1024, 0);
                    printf("%s\n", buffer);
                }
            }
            else if (networkEvent.lNetworkEvents & FD_CLOSE) {
                printf("A client has disconnected \n");
            }
        }
    }
}