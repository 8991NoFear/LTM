#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

DWORD WINAPI myThread(LPVOID cs);
char* ltrim(char* s);
char* rtrim(char* s);
char* trim(char* s);
bool isMatch(char* path, char* username, char* password);
void readBinaryFile(char* path, char** res);

CRITICAL_SECTION cs;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		return -1;
	}

	InitializeCriticalSection(&cs);

	WSAData wsaData;
	int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (startupRes != 0) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao thu vien WinSock, ma loi la: %d\n", errCode);
		return -1;
	}

	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(atoi(argv[1]));
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		int errCode = WSAGetLastError();
		printf("Khong the khoi tao socket, ma loi la: %d\n", errCode);
		return -1;
	}
	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);

	while (true) {
		SOCKADDR_IN caddr;
		int clen = sizeof(caddr); // tuyet doi khong sizeof bien con tro
		SOCKET cs = accept(s, (sockaddr*)&caddr, &clen);
		HANDLE myHandle = CreateThread(NULL, 0, myThread, (LPVOID) &cs, 0, NULL);
	}

	DeleteCriticalSection(&cs);

	return 0;
}

DWORD WINAPI myThread(LPVOID param) {
	SOCKET s = *((SOCKET*) param); // dang la con tro void --> ep ve con tro SOCKET --> tham chieu nguoc
	char* welcome = (char*)"Gui username/password the dinh dang [username password]\n";
	send(s, welcome, strlen(welcome), 0);
	char *buf = (char*)calloc(1024, sizeof(char)) ; // neu khai bao tinh la char buf[1024] thi sizeof(buf) se ra 1024
	recv(s, buf, 1024, 0); // nhan toi da 1024 bytes chu chang biet nhan bao nhieu

	
	buf = trim(buf); // trim 2 dau :((
	char username[1024] = {}, password[1024] = {};
	sscanf(buf, "%s%s", username, password); // ham nay tiem tang loi do khong co tham so gioi han kich thuoc username, password

	// kiem tra thong tin tai khoan mat khau
	char* users = (char*)"c:\\temp\\users.txt";
	if (!isMatch( users, username, password)) {
		char* failed = (char*) "Dang nhap that bai, bye bye.\n";
		send(s, failed, strlen(failed), 0);
		closesocket(s);
		return -1;
	}
	char* succeeded = (char*)"Dang nhap thanh cong, xin hay gui lenh:\n";
	send(s, succeeded, strlen(succeeded), 0);

	// nhan lenh, thuc hien lenh, tra lai ket qua 
	memset(buf, 0, 1024); // tai su dung buf
	recv(s, buf, 1024, 0);
	buf = trim(buf);
	sprintf(buf + strlen(buf), "%s"," > c:\\temp\\logs.txt");
	
	// tranh xung dot
	EnterCriticalSection(&cs);
	system(buf);
	char* logs = (char*)"c:\\temp\\logs.txt";
	char* data = NULL;
	readBinaryFile(logs, &data);
	LeaveCriticalSection(&cs);

	send(s, data, strlen(data), 0);
	free(data);

	closesocket(s);
	return 0;
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