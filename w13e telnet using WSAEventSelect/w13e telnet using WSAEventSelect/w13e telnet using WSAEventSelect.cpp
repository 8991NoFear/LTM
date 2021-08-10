#include <stdio.h>
#include <WinSock2.h>

#define MAX_CLIENT 1024

WSAEVENT g_events[MAX_CLIENT] = {}; // WSAEVENT chinh la HANDLE ma HANDLE chinh la con tro void
SOCKET g_sockets[MAX_CLIENT] = {};
BOOLEAN g_logined[MAX_CLIENT] = {};
int g_count = 0;

// prototype functions
char* ltrim(char* s);
char* rtrim(char* s);
char* trim(char* s);
bool isMatch(char* path, char* username, char* password);
void readBinaryFile(char* path, char** res);

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
    WSAEventSelect(s, g_events[g_count], FD_ACCEPT); // map socket voi event
    g_count++;

    while (true) {
        int index = WSAWaitForMultipleEvents(g_count, g_events, false, INFINITE, false); // toi da 64 socket --> phai ket hop voi luong (giong nhu select)
        index -= WSA_WAIT_EVENT_0; // bh index la chi so event nho nhat
        for (int i = index; i < g_count; i++) {
            WSANETWORKEVENTS networkEvent;
            WSAEnumNetworkEvents(g_sockets[i], g_events[i], &networkEvent);
            if (networkEvent.lNetworkEvents & FD_ACCEPT) { // co su kien accept tren socket g_sockets[i]
                if (networkEvent.iErrorCode[FD_ACCEPT_BIT] == 0) { // khong loi
                    SOCKADDR_IN caddr;
                    int clen = sizeof(caddr);
                    SOCKET c = accept(g_sockets[i], (sockaddr*)&caddr, &clen);
                    HANDLE e = WSACreateEvent();
                    int j = 0;
                    for (j = 0; j < g_count; j++) { // tai su dung lai cac vi tri dong socket truoc do
                        if (g_sockets[j] == 0) {
                            g_sockets[j] = c;
                            g_events[j] = e;
                            g_logined[i] = false;
                        }
                    }
                    if (j == g_count) {
                        g_sockets[g_count] = c;
                        g_events[g_count] = e;
                        g_logined[i] = false;
                        g_count++;
                    }
                    WSAEventSelect(g_sockets[j], g_events[j], FD_READ | FD_CLOSE);
                    char* welcome = (char*)"Gui username/password theo dinh dang [username password]\n";
                    send(g_sockets[j], welcome, strlen(welcome), 0);
                    WSAResetEvent(g_events[i]);
                    WSAResetEvent(g_events[j]);
                }
            }
            else if (networkEvent.lNetworkEvents & FD_READ) {
                if (networkEvent.iErrorCode[FD_READ_BIT] == 0) {
                    if (!g_logined[i]) {
                        char* buf = (char*)calloc(1024, sizeof(char)); // neu khai bao tinh la char buf[1024] thi sizeof(buf) se ra 1024
                        recv(g_sockets[i], buf, 1024, 0); // nhan toi da 1024 bytes chu chang biet thuc su se nhan bao nhieu

                        buf = trim(buf); // trim 2 dau :((
                        char username[1024] = {}, password[1024] = {};
                        sscanf(buf, "%s%s", username, password); // ham nay tiem tang loi do khong co tham so gioi han kich thuoc username, password
                        free(buf); buf = NULL;

                        // kiem tra thong tin tai khoan mat khau
                        char* users = (char*)"c:\\temp\\users.txt";
                        if (!isMatch(users, username, password)) {
                            char* failed = (char*)"Dang nhap that bai, bye bye :).\n";
                            send(g_sockets[i], failed, strlen(failed), 0);
                            closesocket(g_sockets[i]);
                        }
                        else {
                            char* succeeded = (char*)"Dang nhap thanh cong, xin hay gui lenh:\n";
                            send(g_sockets[i], succeeded, strlen(succeeded), 0);
                            g_logined[i] = true;
                        }
                    }
                    else {
                        // nhan lenh, thuc hien lenh, tra lai ket qua 
                        char* buf = (char*)calloc(1024, sizeof(char));
                        recv(g_sockets[i], buf, 1024, 0);
                        buf = trim(buf);
                        sprintf(buf + strlen(buf), "%s", " > c:\\temp\\logs.txt");

                        system(buf);
                        free(buf); buf = NULL;
                        char* logs = (char*)"c:\\temp\\logs.txt";
                        char* data = NULL;
                        readBinaryFile(logs, &data);

                        send(g_sockets[i], data, strlen(data), 0);
                        free(data); data = NULL;

                        closesocket(g_sockets[i]);
                    }
                    WSAResetEvent(g_events[i]);
                }
            }
            else if (networkEvent.lNetworkEvents & FD_CLOSE) {
                g_sockets[i] = 0;
                WSAResetEvent(g_events[i]);
                g_logined[i] = false;
                printf("A client has disconnected \n");
            }
        }
    }
}

void readBinaryFile(char* path, char** res) {
    FILE* fptr = fopen(path, "rb");
    fseek(fptr, 0, SEEK_END);
    int flen = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    *res = (char*)calloc(flen, sizeof(char) + 1);
    fread(*res, sizeof(char), flen, fptr);
    fclose(fptr);
    return;
}

bool isMatch(char* path, char* username, char* password) {
    FILE* fptr = fopen(path, "rt");
    char _username[1024] = {};
    char _password[1024] = {};
    char line[1024] = {};
    while (!feof(fptr)) {
        memset(_username, 0, 1024);
        memset(_password, 0, 1024);
        memset(line, 0, 1024);
        fgets(line, 1024, fptr);
        sscanf(line, "%s%s", _username, _password);
        if (strcmp(username, _username) == 0
            && strcmp(password, _password) == 0) {
            return true;
        }
    }
    fclose(fptr);
    return false;
}

char* ltrim(char* s)
{
    while (isspace(*s)) s++;
    return s;
}

char* rtrim(char* s)
{
    char* back = s + strlen(s);
    while (isspace(*--back));
    *(back + 1) = '\0';
    return s;
}

char* trim(char* s)
{
    return rtrim(ltrim(s));
}
