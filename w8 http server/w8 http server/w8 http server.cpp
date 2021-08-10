#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include "w8 http server.h"

// prototype

void ConCat(char** phtml, char* str)
{
	// realloc la tao ra vung nho moi roi copy vung nho cu chan len
	int oldlen = (*phtml == NULL) ? 0 : strlen(*phtml);
	int tmplen = strlen(str);
	*phtml = (char*)realloc(*phtml, oldlen + tmplen + 1);
	memset(*phtml + oldlen, 0, tmplen + 1);
	sprintf(*phtml + oldlen, "%s", str);
}

// only C
char* ScanFolder(const char* folder)
{
	char* html = NULL;
	ConCat(&html, (char*)"<html>");
	ConCat(&html, (char*)"<form action=\"/\" method=\"post\" enctype=\"multipart/form-data\"><input type=\"file\" id=\"file\" name=\"file\"><br><br><input type=\"submit\" name=\"submit\" value=\"Submit\"></form>");
	char findPath[1024] = {};
	if (strcmp(folder, (char*)"/") == 0)
	{
		sprintf(findPath, "C:/*.*"); // if folder == "/" ==> findPath == C:/*.*
	}
	else
	{
		sprintf(findPath, "C:%s/*.*", folder); // if folder == "/tmp" ==> findPath == C:/tmp/*.*
	}

	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA(findPath, &FindData); // A la ascii
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// realloc la tao ra vung nho moi roi copy vung nho cu chan len
		// <a href="path">FileName</a>
		ConCat(&html, (char*)"<a href=\"");
		char fullPath[1024] = {};
		if (strcmp(folder, "/") == 0)
		{
			sprintf(fullPath, "/");
		}
		else {
			sprintf(fullPath, "%s/", folder);
		}

		ConCat(&html, fullPath);
		ConCat(&html, FindData.cFileName);
		ConCat(&html, (char*)"\">");
		ConCat(&html, FindData.cFileName);
		ConCat(&html, (char*)"</a>");
		while (FindNextFileA(hFind, &FindData))
		{
			ConCat(&html, (char*)"<br>");
			ConCat(&html, (char*)"<a href=\"");
			ConCat(&html, fullPath);
			ConCat(&html, FindData.cFileName);
			ConCat(&html, (char*)"\">");
			ConCat(&html, FindData.cFileName);
			ConCat(&html, (char*)"</a>");
		}
	}
	//CloseHandle(hFind);
	ConCat(&html, (char*)"</html>");

	return html;
}

bool isFolder(char* path)
{
	if (strcmp(path, "/") == 0)
	{
		return true;
	}

	char findPath[1024] = {};
	sprintf(findPath, "C:");
	sprintf(findPath, "C:%s", path);
	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA(findPath, &FindData); // A la ascii
	if (hFind != INVALID_HANDLE_VALUE) {
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return true;
		}
	}
	return false;
}

void ProcessSpace(char* str)
{
	char* space = NULL;
	do
	{
		space = strstr(str, "%20");
		if (space != NULL)
		{
			space[0] = 32; //Dau space
			strcpy(space + 1, space + 3);
		}
	} while (space != NULL);
}

int find(char* buffer, int len, char* boundary)
{
	int pos = -1;
	for (int i = 0; i < len; i++)
	{
		if (buffer[i] == boundary[0])
		{
			int matched = 1;
			for (int j = 0; j < strlen(boundary); j++)
			{
				if (buffer[i + j] != boundary[j])
				{
					matched = 0;
					break;
				}
			}
			if (matched == 1)
			{
				pos = i;
				break;
			}
		}
	}
	return pos;
}

DWORD WINAPI myThread(LPVOID params)
{
	SOCKET c = (SOCKET)params;
	char* buffer = (char*) calloc(1024, sizeof(char));
	recv(c, buffer, 1024, 0);

	char action[1024] = {};
	char path[1024] = {};
	sscanf(buffer, "%s%s", action, path);
	printf("%s", action);
	printf(" %s\n", path);
	ProcessSpace(path);

	if (strcmp(action, "GET") == 0)
	{
		char* html = NULL;
		if (isFolder(path)) // get folder
		{
			html = ScanFolder(path);
			char* response = NULL;
			ConCat(&response, (char*)"HTTP/1.1 200 OK\r\nServer: MyLocalFileServer\r\nContent-Length: ");
			char len[16] = {};
			itoa(strlen(html), len, 10); // interger base 10 to string
			ConCat(&response, len);
			ConCat(&response, (char*)"\r\nContent-Type: text/html\r\n\r\n");
			ConCat(&response, html);
			send(c, response, strlen(response), 0);
			closesocket(c); // session <=> "request - response"
			free(response); response = NULL;
			free(html); html = NULL;
		}
		else // get a file
		{
			char type[1024] = {};
			if (strlen(path) > 4)
			{
				// MIME Types: https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
				if (stricmp(path + strlen(path) - 4, ".jpg") == 0) // stricmp --> compare without case-sensitive
				{
					strcpy(type, "image/jpeg");
				}
				else if (stricmp(path + strlen(path) - 4, ".mp4") == 0)
				{
					strcpy(type, "video/mp4");
				}
				else if (stricmp(path + strlen(path) - 4, ".mp3") == 0)
				{
					strcpy(type, "audio/mpeg");
				}
				else if (stricmp(path + strlen(path) - 4, ".pdf") == 0)
				{
					strcpy(type, "application/pdf");
				}
				else if (stricmp(path + strlen(path) - 5, ".docx") == 0)
				{
					strcpy(type, "application/msword");
				}
				else if (stricmp(path + strlen(path) - 5, ".xlsx") == 0)
				{
					strcpy(type, "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
				}
				else
				{
					strcpy(type, "application/octet-stream");
				}
			}
			else
			{
				strcpy(type, "application/octet-stream");
			}
			char fullPath[1024] = {};
			sprintf(fullPath, "C:%s", path);
			FILE* f = fopen(fullPath, "rb");
			if (f == NULL) {
				char* response = (char*)"HTTP/1.1 404 Not Found\r\nServer: MyLocalFileServer\r\n\r\n";
				send(c, response, strlen(response), 0);
				return 0;
			}
			fseek(f, 0, SEEK_END);
			int flen = ftell(f);
			fseek(f, 0, SEEK_SET);

			char response[1024] = {};
			sprintf(response, "HTTP/1.1 200 OK\r\nServer: MyLocalFileServer\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", type, flen);
			send(c, response, strlen(response), 0);

			// send file
			char data[1024] = {};
			while (!feof(f)) {
				int r = fread(data, sizeof(char), sizeof(data), f);
				send(c, data, r, 0);
			}
			fclose(f);
		}
	}
	else if (strcmp(action, "POST") == 0)
	{
		char* tmp = strstr(buffer, (char*)"Content-Length:");
		if (tmp != NULL)
		{
			int contentLength = 0;
			tmp += 15; // dich con tro ra sau "Content-Length:"
			sscanf(tmp, "%d", &contentLength);
			printf("Prepare to received: %d bytes\r\n", contentLength);
			char* filedata = NULL;
			int filelen = 0;
			char* content = (char*)calloc(contentLength, 1);
			int received = 0;
			while (received < contentLength)
			{
				received += recv(c, content + received, contentLength - received, 0);
				printf("\tReceived: %d bytes\r\n", received);
			}
			char* fnamestart = strstr(content, "filename=\"") + 10;
			char* fnamestop = strstr(fnamestart, "\"") - 1;
			char fname[1024] = {};
			strcpy(fname, "c:\\temp\\uploads\\");
			memcpy(fname + strlen(fname), fnamestart, fnamestop - fnamestart + 1);

			char boundary[1024] = {};
			sscanf(content, "%s", boundary); // -----falkewajfpodjnaf
			tmp = strstr(content, "\r\n\r\n");
			if (tmp)
			{
				tmp += 4;
				int pos = find(tmp, contentLength - (tmp - content), boundary); // pos-3 la kich thuoc file :))
				if (pos != -1)
				{
					char* next = content + pos + (tmp - content);
					filelen = (next - tmp + 1) - 3; // tru di 3 boi vi 3 ki tu {\r, \n, -}
					filedata = (char*)calloc(filelen, 1);
					memcpy(filedata, tmp, filelen);
					FILE* f1 = fopen(fname, "wb");
					fwrite(filedata, 1, filelen, f1);
					fclose(f1);

					char* html = (char*)"<html>File has been uploaded</html>";
					char response[1024];
					memset(response, 0, sizeof(response));
					sprintf(response, "HTTP/1.1 200 OK\r\nServer: MYLOCAL\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n%s", strlen(html), html);
					send(c, response, strlen(response), 0);
					closesocket(c);
					free(filedata); filedata = NULL;
				}
			}
			free(content);
			closesocket(c);
		}
	}

	return 0;
}

int main() {
	WSADATA wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi: %d\n", errCode);
		return -1;
	}

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi: %d\n", errCode);
		return -1;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = ADDR_ANY;

	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);

	while (true) {
		SOCKADDR caddr;
		int clen = sizeof(caddr);
		SOCKET c = accept(s, (sockaddr*)&caddr, &clen);
		if (c == INVALID_SOCKET) {
			int errCode = WSAGetLastError();
			printf("Khong the chap nhan ket noi, ma loi la: %d\n", errCode);
			return -1;
		}
		CreateThread(NULL, 0, &myThread, (LPVOID)c, 0, NULL);
	}
}
