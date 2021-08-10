#include <stdio.h>
#include <WinSock2.h>
#include <direct.h>

#define MAX_CLIENT 1024

WSAEVENT g_events[MAX_CLIENT] = {}; // WSAEVENT chinh la HANDLE ma HANDLE chinh la con tro void
SOCKET g_sockets[MAX_CLIENT] = {}; // command socket
char g_wd[MAX_CLIENT][1024] = {};
int g_count = 0;
SOCKET g_ftplisten; // socket nghe co dinh o cong 9999
SOCKET g_ftpdata[MAX_CLIENT] = {}; // mang socket duoc accept boi socket nghe o 9999
BOOL g_modes[MAX_CLIENT] = {};

// active & passive mode
unsigned char g_ip1[MAX_CLIENT] = {};
unsigned char g_ip2[MAX_CLIENT] = {};
unsigned char g_ip3[MAX_CLIENT] = {};
unsigned char g_ip4[MAX_CLIENT] = {};
unsigned char g_p1[MAX_CLIENT] = {};
unsigned char g_p2[MAX_CLIENT] = {};

void concat(char** phtml, char* str) {
    // realloc la tao ra vung nho moi roi copy vung nho cu chan len
    int oldlen = (*phtml == NULL) ? 0 : strlen(*phtml);
    int tmplen = strlen(str);
    *phtml = (char*)realloc(*phtml, oldlen + tmplen + 1);
    memset(*phtml + oldlen, 0, tmplen + 1);
    sprintf(*phtml + oldlen, "%s", str);
}

char* scanFiles(char* path) {
    char* res = NULL;
    char oneline[1024];
    // sanitize path
    char findPath[1024] = {};
    if (strcmp(path, "/") == 0)
    {
        sprintf(findPath, "C:/*.*"); // if folder == "/" ==> findPath == C:/*.*
    }
    else
    {
        sprintf(findPath, "C:%s/*.*", path); // if folder == "/tmp" ==> findPath == C:/tmp/*.*
    }
    
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(findPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            sprintf(oneline, "type=dir");
        }
        else {
            sprintf(oneline, "type=file");
        }
        FILETIME fileTime = findData.ftLastWriteTime;
        SYSTEMTIME systemTime;
        FileTimeToSystemTime(&fileTime, &systemTime);
        sprintf(oneline + strlen(oneline), ";modify=%d%02d%02d%02d%02d%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
        DWORD fileSizeHigh = findData.nFileSizeHigh;
        DWORD fileSizeLow = findData.nFileSizeLow;
        UINT64 fileSize = ((fileSizeHigh << 32) | fileSizeLow);
        sprintf(oneline + strlen(oneline), ";%s", fileSize);
        sprintf(oneline + strlen(oneline), "; %s\r\n", findData.cFileName);
        concat(&res, oneline);
        while (FindNextFileA(hFind, &findData)) {
            memset(oneline, 0, sizeof(oneline));
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                sprintf(oneline, "type=dir");
            }
            else {
                sprintf(oneline, "type=file");
            }
            FILETIME fileTime = findData.ftLastWriteTime;
            SYSTEMTIME systemTime;
            FileTimeToSystemTime(&fileTime, &systemTime);
            sprintf(oneline + strlen(oneline), ";modify=%4d%02d%02d%02d%02d%02d", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
            sprintf(oneline + strlen(oneline), "; %s\r\n", findData.cFileName);
            concat(&res, oneline);
        }
    }
    return res;
}

char* to3LetterAbbr(int month) {
    char *res;
    switch(month) {
    case 1:
        res = (char*)"Jan";
        break;
    case 2:
        res = (char*)"Feb";
        break;
    case 3:
        res = (char*)"Mar";
        break;
    case 4:
        res = (char*)"Apr";
        break;
    case 5:
        res = (char*)"May";
        break;
    case 6:
        res = (char*)"Jun";
        break;
    case 7:
        res = (char*)"Jul";
        break;
    case 8:
        res = (char*)"Aug";
        break;
    case 9:
        res = (char*)"Sep";
        break;
    case 10:
        res = (char*)"Oct";
        break;
    case 11:
        res = (char*)"Nov";
        break;
    case 12:
        res = (char*)"Dec";
        break;
    default:
        break;
    }
    return res;
}

char* pasvScanFiles(char* path) {
    char* res = NULL;
    char oneline[1024];
    // sanitize path
    char findPath[1024] = {};
    if (strcmp(path, "/") == 0)
    {
        sprintf(findPath, "C:/*.*"); // if folder == "/" ==> findPath == C:/*.*
    }
    else
    {
        sprintf(findPath, "C:%s/*.*", path); // if folder == "/tmp" ==> findPath == C:/tmp/*.*
    }

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(findPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            sprintf(oneline, "drwxrwxr-- 1 ftp ftp");
        }
        else {
            sprintf(oneline, "-rwxrwxr-- 1 ftp ftp");
        }
        DWORD fileSizeHigh = findData.nFileSizeHigh;
        DWORD fileSizeLow = findData.nFileSizeLow;
        UINT64 fileSize = ((fileSizeHigh<<32)|fileSizeLow);
        sprintf(oneline + strlen(oneline), " %-14llu", fileSize);
        FILETIME fileTime = findData.ftLastWriteTime;
        SYSTEMTIME systemTime;
        FileTimeToSystemTime(&fileTime, &systemTime);
        sprintf(oneline + strlen(oneline), "%s %02d %02d:%02d", to3LetterAbbr(systemTime.wMonth), systemTime.wDay, systemTime.wHour, systemTime.wMinute);
        sprintf(oneline + strlen(oneline), "; %s\n", findData.cFileName);
        concat(&res, oneline);
        while (FindNextFileA(hFind, &findData)) {
            memset(oneline, 0, sizeof(oneline));
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                sprintf(oneline, "drwxrwxr-- 1 ftp ftp");
            }
            else {
                sprintf(oneline, "-rwxrwxr-- 1 ftp ftp");
            }
            DWORD fileSizeHigh = findData.nFileSizeHigh;
            DWORD fileSizeLow = findData.nFileSizeLow;
            UINT64 fileSize = ((fileSizeHigh << 32) | fileSizeLow);
            sprintf(oneline + strlen(oneline), " %-14llu", fileSize);
            FILETIME fileTime = findData.ftLastWriteTime;
            SYSTEMTIME systemTime;
            FileTimeToSystemTime(&fileTime, &systemTime);
            sprintf(oneline + strlen(oneline), "%s %02d %02d:%02d", to3LetterAbbr(systemTime.wMonth), systemTime.wDay, systemTime.wHour, systemTime.wMinute);
            sprintf(oneline + strlen(oneline), "; %s\n", findData.cFileName);
            concat(&res, oneline);
        }
    }
    return res;
}

int main() {
    WSADATA wsaData;
    int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (startupRes != 0) {
        int errCode = WSAGetLastError();
        printf("Khong the khoi tao thu vien WinSock, ma loi: %d", errCode);
        exit(-1);
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        printf("Khong the khoi tao socket, ma loi: %d", errCode);
        exit(-1);
    }

    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.S_un.S_addr = ADDR_ANY;

    if (bind(s, (sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        printf("Khong the bind socket, ma loi: %d", errCode);
        exit(-1);
    }

    if (listen(s, 10) == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        printf("Khong the listen, ma loi: %d", errCode);
        exit(-1);
    }

    g_events[g_count] = WSACreateEvent();
    g_sockets[g_count] = s;
    /*
    * moi socket phai anh xa tuong ung voi 1 event
    * sk tham so thu 3 xay ra o socket tham so thu 1 se lam event tham so thu 2 "mo"
    */
    WSAEventSelect(s, g_events[g_count], FD_ACCEPT); // anh xa socket voi event
    g_count++;

    // socket luon nghe co dinh o cong 9999
    SOCKET pasv_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (pasv_s == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        printf("Khong the khoi tao pasv data socket, ma loi: %d\n", errCode);
        exit(-1);
    }

    SOCKADDR_IN pasv_addrin;
    pasv_addrin.sin_family = AF_INET;
    pasv_addrin.sin_port = htons(9999);
    pasv_addrin.sin_addr.S_un.S_addr = ADDR_ANY;

    if (bind(pasv_s, (sockaddr*)&pasv_addrin, sizeof(pasv_addrin)) == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        printf("Khong the bind, ma loi: %d\n", errCode);
        exit(-1);
    }

    if (listen(pasv_s, 10) == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        printf("Khong the bind, ma loi: %d\n", errCode);
        exit(-1);
    }

    g_ftplisten = pasv_s;

    while (true) {
        /*
        * toi da 64 socket --> phai ket hop voi luong (giong nhu select)
        * treo tai day mai mai (INFINITE) cho den khi co it nhat 1 event "mo"
        */
        int index = WSAWaitForMultipleEvents(g_count, g_events, false, INFINITE, false); // client nao do dong thi event cua no la invalid handle --> index bang -1
        index -= WSA_WAIT_EVENT_0; // bh index la chi so event nho nhat trong mang events[]
        for (int i = index; i < g_count; i++) { // co nhung event nao "mo"
            WSANETWORKEVENTS networkEvent;
            WSAEnumNetworkEvents(g_sockets[i], g_events[i], &networkEvent);
            WSAResetEvent(g_events[i]); // chuyen event ve trang thai "dong"
            if (networkEvent.lNetworkEvents & FD_ACCEPT) { // co su kien accept tren socket g_sockets[i] hay khong
                if (networkEvent.iErrorCode[FD_ACCEPT_BIT] == 0) { // co xay ra loi hay khong
                    if (g_sockets[i] == s) { // co yeu cau den cong 21
                        SOCKADDR_IN caddr;
                        int clen = sizeof(caddr);
                        SOCKET c = accept(g_sockets[i], (sockaddr*)&caddr, &clen);
                        HANDLE e = WSACreateEvent();
                        g_sockets[g_count] = c;
                        g_events[g_count] = e;
                        g_count++; // KHONG TAN DUNG DUOC CAC SOCKET DA DONG
                        WSAEventSelect(c, e, FD_READ | FD_CLOSE);

                        char* welcome = (char*)"My FTP Server\r\n220 Ok\r\n";
                        send(c, welcome, strlen(welcome), 0);
                    }
                }
            }
            else if (networkEvent.lNetworkEvents & FD_READ) {
                if (networkEvent.iErrorCode[FD_READ_BIT] == 0) {
                    SOCKET c = g_sockets[i];
                    char buffer[1024] = {};
                    recv(c, buffer, 1024, 0);
                    for (int j = 0; j < strlen(buffer); j++) {
                        if (buffer[j] != ' ') {
                            buffer[j] = toupper(buffer[j]);
                        }
                        else {
                            break;
                        }
                    }
                    printf("%s\n", buffer);
                    if (strncmp(buffer, "USER", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "331 Password required\r\n"); // user nao cung duoc
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "PASS", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "230 Logged on\r\n"); // pass nao cung duoc
                        send(c, buffer, strlen(buffer), 0);
                        memset(g_wd[i], 0, sizeof(g_wd[i]));
                        sprintf(g_wd[i], "%s", "/");
                    }
                    else if (strncmp(buffer, "SYST", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "215 UNIX\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "FEAT", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "211-Features:\n MDTM\n REST STREAM\n SIZE\n MLST type*;size*;modify*;\n MLSD\n UTF8\n CLNT\n MFMT\n EPSV\n EPRT\n211 End\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "CLNT", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 Don't care\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "OPTS", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "202 UTF8 mode on\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "PWD", 3) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "217 \"%s\" is current working directory\r\n", g_wd[i]);
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "TYPE", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 OK\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "PORT", 4) == 0) {
                        g_modes[i] = 1;
                        for (int i = 0; i < strlen(buffer); i++) {
                            if (buffer[i] == ',') {
                                buffer[i] = ' ';
                            }
                        }
                        char cmd[5] = {};
                        int ip[4] = {};
                        int port[2] = {};
                        sscanf(buffer, "%s %d %d %d %d %d %d", &cmd, &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);
                        g_ip1[i] = ip[0];
                        g_ip2[i] = ip[1];
                        g_ip3[i] = ip[2];
                        g_ip4[i] = ip[3];
                        g_p1[i] = port[0];
                        g_p2[i] = port[1];

                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 Port command successful\r\n"); // XL khong hop ly cho lam --> can phai connect duoc thanh cong moi tra loi Ok
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "MLSD", 4) == 0) {
                        unsigned short dataport = g_p1[i] * 256 + g_p2[i];
                        SOCKET ds = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                        if (ds == INVALID_SOCKET) {
                            int errCode = WSAGetLastError();
                            printf("Khong the khoi tao socket, ma loi: %d", errCode);
                            break;
                        }
                        SOCKADDR_IN dsaddr;
                        dsaddr.sin_family = AF_INET;
                        dsaddr.sin_port = htons(dataport);
                        dsaddr.sin_addr.S_un.S_un_b.s_b1 = g_ip1[i];
                        dsaddr.sin_addr.S_un.S_un_b.s_b2 = g_ip2[i];
                        dsaddr.sin_addr.S_un.S_un_b.s_b3 = g_ip3[i];
                        dsaddr.sin_addr.S_un.S_un_b.s_b4 = g_ip4[i];

                        // thong bao mo phien truyen dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "150 Opening data channel\r\n");
                        send(c, buffer, strlen(buffer), 0);
                        if (connect(ds, (sockaddr*)&dsaddr, sizeof(dsaddr)) == SOCKET_ERROR) {
                            int errCode = WSAGetLastError();
                            printf("Khong the ket noi toi kenh truyen du lieu, ma loi la %d", errCode);
                            memset(buffer, 0, 1024);
                            sprintf(buffer, "425 Cannot open data channel\r\n");
                            send(c, buffer, strlen(buffer), 0);
                            break;
                        }

                        // truyen dl o kenh dl
                        char* scanRes = scanFiles(g_wd[i]);
                        if (scanRes == NULL) {
                            scanRes = (char*)calloc(3, sizeof(char)); // cap phat dong de phia duoi free
                            sprintf(scanRes, "\r\n");
                        }
                        send(ds, scanRes, strlen(scanRes), 0);
                        free(scanRes); scanRes = NULL; // nho la khong free duoc "con tro hang"
                        closesocket(ds);

                        // thong bao truyen xong dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "226 Successfully transferred\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "CWD", 3) == 0) {
                        char dir[1024] = {};
                        while (buffer[strlen(buffer) - 1] == '\r' || buffer[strlen(buffer) - 1] == '\n') {
                            buffer[strlen(buffer) - 1] = '\0';
                        }
                        strcpy(dir, buffer + 4);
                        if (dir[0] == '/') {
                            sprintf(g_wd[i], "%s", dir);
                        }
                        else if (strcmp(g_wd[i], "/") == 0) {
                            sprintf(g_wd[i] + strlen(g_wd[i]), "%s", dir);
                        }
                        else {
                            sprintf(g_wd[i] + strlen(g_wd[i]), "/%s", dir);
                        }
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "%s", "250 CWD successful\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "CDUP", 4) == 0) {
                        if (strcmp(g_wd[i], "/") != 0) {
                            for (int j = strlen(g_wd[i]); j >= 0; j--) {
                                if (g_wd[i][j] == '/') {
                                    g_wd[i][j] = '\0';
                                    break;
                                }
                            }
                        }
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "%s", "250 CDUP successful\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "RETR", 4) == 0) {
                        char* fname = buffer + 5;
                        while (fname[strlen(fname) - 1] == '\r' || fname[strlen(fname) - 1] == '\n') {
                            fname[strlen(fname) - 1] = '\0';
                        }
                        char fpath[1024] = {};
                        if (g_wd[i][strlen(g_wd[i]) - 1] != '/') {
                            sprintf(fpath, "C:%s/%s", g_wd[i], fname);
                        }
                        else {
                            sprintf(fpath, "C:%s%s", g_wd[i], fname);
                        }

                        // thong bao mo phien truyen dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "150 Opening data channel\r\n");
                        send(c, buffer, strlen(buffer), 0);

                        if (g_modes[i]) {
                            unsigned short dataport = g_p1[i] * 256 + g_p2[i];
                            SOCKET ds = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                            if (ds == INVALID_SOCKET) {
                                int errCode = WSAGetLastError();
                                printf("Khong the khoi tao socket, ma loi: %d", errCode);
                                break;
                            }
                            SOCKADDR_IN dsaddr;
                            dsaddr.sin_family = AF_INET;
                            dsaddr.sin_port = htons(dataport);
                            dsaddr.sin_addr.S_un.S_un_b.s_b1 = g_ip1[i];
                            dsaddr.sin_addr.S_un.S_un_b.s_b2 = g_ip2[i];
                            dsaddr.sin_addr.S_un.S_un_b.s_b3 = g_ip3[i];
                            dsaddr.sin_addr.S_un.S_un_b.s_b4 = g_ip4[i];

                            if (connect(ds, (sockaddr*)&dsaddr, sizeof(dsaddr)) == SOCKET_ERROR) {
                                int errCode = WSAGetLastError();
                                printf("Khong the ket noi toi kenh truyen du lieu, ma loi la %d", errCode);
                                memset(buffer, 0, 1024);
                                sprintf(buffer, "425 Cannot open data channel\r\n");
                                send(c, buffer, strlen(buffer), 0);
                                break;
                            }

                            // truyen dl o kenh dl
                            memset(buffer, 0, 1024);
                            FILE* f = fopen(fpath, "rb");
                            while (!feof(f)) {
                                int r = fread(buffer, sizeof(char), sizeof(buffer), f);
                                send(ds, buffer, r, 0);
                            }
                            fclose(f);
                            closesocket(ds);
                        }
                        else {
                            // truyen dl o kenh dl
                            memset(buffer, 0, 1024);
                            FILE* f = fopen(fpath, "rb");
                            while (!feof(f)) {
                                int r = fread(buffer, sizeof(char), sizeof(buffer), f);
                                send(g_ftpdata[i], buffer, r, 0);
                            }
                            fclose(f);
                            closesocket(g_ftpdata[i]);
                            g_ftpdata[i] = INVALID_SOCKET;
                        }

                        // thong bao truyen xong dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "226 Successfully transferred\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "STOR", 4) == 0) {
                        char* fname = buffer + 5;
                        while (fname[strlen(fname) - 1] == '\r' || fname[strlen(fname) - 1] == '\n') {
                            fname[strlen(fname) - 1] = '\0';
                        }
                        char fpath[1024] = {};
                        if (g_wd[i][strlen(g_wd[i]) - 1] != '/') {
                            sprintf(fpath, "C:%s/%s", g_wd[i], fname);
                        }
                        else {
                            sprintf(fpath, "C:%s%s", g_wd[i], fname);
                        }

                        // thong bao mo phien truyen dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "150 Opening data channel\r\n");
                        send(c, buffer, strlen(buffer), 0);

                        if (g_modes[i]) {
                            unsigned short dataport = g_p1[i] * 256 + g_p2[i];
                            SOCKET ds = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                            if (ds == INVALID_SOCKET) {
                                int errCode = WSAGetLastError();
                                printf("Khong the khoi tao socket, ma loi: %d", errCode);
                                break;
                            }
                            SOCKADDR_IN dsaddr;
                            dsaddr.sin_family = AF_INET;
                            dsaddr.sin_port = htons(dataport);
                            dsaddr.sin_addr.S_un.S_un_b.s_b1 = g_ip1[i];
                            dsaddr.sin_addr.S_un.S_un_b.s_b2 = g_ip2[i];
                            dsaddr.sin_addr.S_un.S_un_b.s_b3 = g_ip3[i];
                            dsaddr.sin_addr.S_un.S_un_b.s_b4 = g_ip4[i];

                            if (connect(ds, (sockaddr*)&dsaddr, sizeof(dsaddr)) == SOCKET_ERROR) {
                                int errCode = WSAGetLastError();
                                printf("Khong the ket noi toi kenh truyen du lieu, ma loi la %d", errCode);
                                memset(buffer, 0, 1024);
                                sprintf(buffer, "425 Cannot open data channel\r\n");
                                send(c, buffer, strlen(buffer), 0);
                                break;
                            }

                            // truyen dl o kenh dl
                            memset(buffer, 0, 1024);
                            FILE* f = fopen(fpath, "wb");
                            while (true) {
                                int r = recv(ds, buffer, sizeof(buffer), 0);
                                if (r > 0) {
                                    fwrite(buffer, sizeof(char), r, f);
                                }
                                else {
                                    break;
                                }
                            }
                            fclose(f);
                            closesocket(ds);
                        }
                        else {
                            // truyen dl o kenh dl
                            memset(buffer, 0, 1024);
                            FILE* f = fopen(fpath, "wb");
                            while (true) {
                                int r = recv(g_ftpdata[i], buffer, sizeof(buffer), 0);
                                if (r > 0) {
                                    fwrite(buffer, sizeof(char), r, f);
                                }
                                else {
                                    break;
                                }
                            }
                            fclose(f);
                            closesocket(g_ftpdata[i]);
                            g_ftpdata[i] = INVALID_SOCKET;
                        }

                        // thong bao truyen xong dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "226 Successfully transferred\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "SIZE", 4) == 0) {
                        char* fname = buffer + 5;
                        while (fname[strlen(fname) - 1] == '\r' || fname[strlen(fname) - 1] == '\n') {
                            fname[strlen(fname) - 1] = '\0';
                        }
                        char fpath[1024] = {};
                        if (g_wd[i][strlen(g_wd[i]) - 1] != '/') {
                            sprintf(fpath, "C:%s/%s", g_wd[i], fname);
                        }
                        else {
                            sprintf(fpath, "C:%s%s", g_wd[i], fname);
                        }

                        FILE* f = fopen(fpath, "rb");
                        fseek(f, 0, SEEK_END);
                        long size = ftell(f);
                        fclose(f);

                        memset(buffer, 0, 1024);
                        sprintf(buffer, "213 %d\r\n", size);
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "DELE", 4) == 0) {
                        char* fname = buffer + 5;
                        while (fname[strlen(fname) - 1] == '\r' || fname[strlen(fname) - 1] == '\n') {
                            fname[strlen(fname) - 1] = '\0';
                        }
                        char fpath[1024] = {};
                        if (g_wd[i][strlen(g_wd[i]) - 1] != '/') {
                            sprintf(fpath, "C:%s/%s", g_wd[i], fname);
                        }
                        else {
                            sprintf(fpath, "C:%s%s", g_wd[i], fname);
                        }

                        remove(fpath); // xoa file

                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 Ok\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "PASV", 4) == 0) {
                        g_modes[i] = 0;
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "227 Entering Passive Mode (127,0,0,1,39,15)\r\n");
                        send(c, buffer, strlen(buffer), 0);

                        // sau khi nhan "227 Entering Passive Mode (127,0,0,1,39,15)" --> client se ket noi ngay
                        SOCKADDR_IN pasv_data_saddr;
                        int pasv_data_addrlen = sizeof(pasv_data_saddr);
                        SOCKET pasv_data_s = accept(g_ftplisten, (sockaddr*)&pasv_data_saddr, &pasv_data_addrlen);
                        g_ftpdata[i] = pasv_data_s;
                    }
                    else if (strncmp(buffer, "LIST", 4) == 0) {
                        // thong bao mo phien truyen dl
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "150 Opening data channel\r\n");
                        send(c, buffer, strlen(buffer), 0);
                        if (g_ftpdata[i] == INVALID_SOCKET) {
                            memset(buffer, 0, 1024);
                            sprintf(buffer, "425 Cannot open data channel\r\n");
                            send(c, buffer, strlen(buffer), 0);
                            break;
                        }

                        // gui dl tren datasockets da accept
                        char* scanRes = pasvScanFiles(g_wd[i]);
                        if (scanRes == NULL) {
                            scanRes = (char*)calloc(2, sizeof(char));
                            sprintf(scanRes, "\n");
                        }
                        send(g_ftpdata[i], scanRes, strlen(scanRes), 0);
                        free(scanRes); scanRes = NULL;
                        closesocket(g_ftpdata[i]);
                        g_ftpdata[i] = INVALID_SOCKET;

                        // thong bao truyen xong dl o kenh lenh
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "226 Successfully transferred\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "MKD", 3) == 0) {
                        char* fname = buffer + 4;
                        while (fname[strlen(fname) - 1] == '\r' || fname[strlen(fname) - 1] == '\n') {
                            fname[strlen(fname) - 1] = '\0';
                        }
                        char fpath[1024] = {};
                        if (g_wd[i][strlen(g_wd[i]) - 1] != '/') {
                            sprintf(fpath, "C:%s/%s", g_wd[i], fname);
                        }
                        else {
                            sprintf(fpath, "C:%s%s", g_wd[i], fname);
                        }

                        mkdir(fpath);

                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 Ok\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "RMD", 3) == 0) { // chi chay tren total commander
                        char* fname = buffer + 4;
                        while (fname[strlen(fname) - 1] == '\r' || fname[strlen(fname) - 1] == '\n') {
                            fname[strlen(fname) - 1] = '\0';
                        }
                        char fpath[1024] = {};
                        if (g_wd[i][strlen(g_wd[i]) - 1] != '/') {
                            sprintf(fpath, "C:%s/%s", g_wd[i], fname);
                        }
                        else {
                            sprintf(fpath, "C:%s%s", g_wd[i], fname);
                        }

                        rmdir(fpath);
                        
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 Ok\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else if (strncmp(buffer, "NOOP", 4) == 0) {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "200 Ok\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                    else {
                        memset(buffer, 0, 1024);
                        sprintf(buffer, "%s", "202 Command not implemented\r\n");
                        send(c, buffer, strlen(buffer), 0);
                    }
                }
            }
            else if (networkEvent.lNetworkEvents & FD_CLOSE) {
                WSACloseEvent(g_events[i]);
                printf("A client has disconnected %d \n", i);
            }
        }
    }
}