#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

int main()
{
    WSADATA wsaData;
    WORD word = MAKEWORD(2, 2);
    int error = WSAStartup(word, &wsaData); // khoi tao thu vien WinSock
    if (error != 0)
    {
        int errorCode = WSAGetLastError(); // Tools > Error Lookup
        printf("Khoi tao khong thanh cong WinSock, ma loi la: %d\n", errorCode);
    }
    else
    {
        printf("Khoi tao thanh cong WinSock\n");

        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // khoi tao socket
        if (s == INVALID_SOCKET)
        {
            int errorCode = WSAGetLastError();
            printf("Khong khoi tao duoc socket, ma loi la: %d\n", errorCode);
        }
        else
        {
            printf("Gia tri cua socket vua khoi tao: %I64u\n", s); // socket la 1 so unsigned __int64

            SOCKADDR_IN saddrin; // luu thong tin dia chi socket
            saddrin.sin_family = AF_INET;
            saddrin.sin_port = htons(80); // chuyen tu little endian sang big endian
            saddrin.sin_addr.S_un.S_addr = inet_addr("111.65.250.2"); // inet_addr was deprecated

            PADDRINFOA addrinf = NULL;
            // phan giai ten mien
            getaddrinfo("vnexpress.net", "http", NULL, &addrinf); // tu cap phat bo nho cho addrinf --> sau nay phai tu giai phong
            PADDRINFOA rootAddrinf = addrinf;
            while (addrinf != NULL) {
                SOCKADDR_IN saddrinRes;
                if (addrinf->ai_family == AF_INET) {
                    memcpy(&saddrinRes, addrinf->ai_addr, addrinf->ai_addrlen); // ai_addr la con tro nen khong can &
                    printf("Dia chi cua vnexpress.net la: %s\n", inet_ntoa(saddrinRes.sin_addr));
                }
                addrinf = addrinf->ai_next;
            }
            freeaddrinfo(rootAddrinf);
        }

        WSACleanup();
    }
}
