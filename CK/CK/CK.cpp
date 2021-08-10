#include <stdio.h>
#include <WinSock2.h>

#define BACKLOG 10
#define MAX_CLIENT 100

DWORD WINAPI MyThread(LPVOID param);

struct client {
	SOCKET clientSocket;
	unsigned int roomID;
	bool isActive;
};
typedef struct client CLIENT;

CLIENT g_clients[MAX_CLIENT] = {}; // auto memset zeros
unsigned int g_count = 0;

CRITICAL_SECTION cs;

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	SOCKADDR_IN saddrin;
	saddrin.sin_family = AF_INET;
	saddrin.sin_port = htons(5555);
	saddrin.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (SOCKADDR*)&saddrin, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	if (listen(listenSocket, BACKLOG) == SOCKET_ERROR) {
		printf("Error occurs: %d\n", WSAGetLastError());
	}

	InitializeCriticalSection(&cs);
	while (true) {
		bool hasAMiddleSlot = false;
		if (g_count < MAX_CLIENT) {
			SOCKADDR_IN caddrin;
			int clen = sizeof(SOCKADDR_IN);
			SOCKET clientSocket = accept(listenSocket, (sockaddr*)&caddrin, &clen);
			EnterCriticalSection(&cs);
			for (int i = 0; i < g_count; i++) {
				if (!g_clients[i].isActive) {
					hasAMiddleSlot = true;
					g_clients[i].clientSocket = clientSocket;
					g_clients[i].isActive = true;
					CreateThread(NULL, 0, MyThread, (LPVOID)i, 0, NULL);
					break;
				}
			}
			if (!hasAMiddleSlot) {
				g_clients[g_count].clientSocket = clientSocket;
				g_clients[g_count].isActive = true;
				HANDLE myHandle = CreateThread(NULL, 0, MyThread, (LPVOID)g_count, 0, NULL);
				g_count++;
			}
			LeaveCriticalSection(&cs);
		}
	}
	DeleteCriticalSection(&cs);
	return 0;
}

DWORD WINAPI MyThread(LPVOID param) {
	unsigned int index = (unsigned int)param;
	while (true) {
		CLIENT *pclient = &g_clients[index];
		char buffer[1024] = {}; // auto memset zeros
		if ((*pclient).isActive) {
			recv((*pclient).clientSocket, buffer, 1024, 0);
			char cmd[1024] = {}; // auto memset zeros
			char response[1024] = {}; // auto memset zeros
			
			if (strncmp(buffer, "ROOM:", 5) == 0) {
				unsigned int roomID = 0;
				sscanf(buffer, "%s %d", cmd, &roomID);
				if (roomID < 1 || roomID > 100) { // Invalid roomID
					sprintf(response, (char*)"Invalid room number, please try again.\n");
				} else {
					EnterCriticalSection(&cs);
					(*pclient).roomID = roomID;
					LeaveCriticalSection(&cs);
					sprintf(response, (char*)"You joined room: %d\n", roomID);
				}
				send((*pclient).clientSocket, response, strlen(response), 0);
			}
			else if (strncmp(buffer, "QUIT", 4) == 0) {
				closesocket((*pclient).clientSocket);
				memset(pclient, 0, sizeof(CLIENT)); // clear client
			}
			else if (strncmp(buffer, "MSG:", 4) == 0) {
				if ((*pclient).roomID < 1 || (*pclient).roomID > 100) {
					sprintf(response, (char*)"Please use ROOM command to join first.\n");
					send((*pclient).clientSocket, response, strlen(response), 0);
				}
				else {
					for (unsigned int i = 0; i < g_count; i++) {
						if (i != index) {
							EnterCriticalSection(&cs);
							CLIENT otherClient = g_clients[i];
							LeaveCriticalSection(&cs);
							sprintf(response, buffer + 5);
							if (otherClient.isActive && (otherClient.roomID == (*pclient).roomID)) { // send messsage to all other client in 
								send(otherClient.clientSocket, response, strlen(response), 0);
							}
						}
					}
				}
			}
			else {
				sprintf(response, (char*)"Invalid command!\n");
				send((*pclient).clientSocket, response, strlen(response), 0);
			}
		}
		else {
			break;
		}
	}
	return 0;
}