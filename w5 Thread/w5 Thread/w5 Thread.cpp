#include <stdio.h>
#include <Windows.h>

int sum = 0;

DWORD WINAPI MyThread(LPVOID params) {
	int threadId = GetCurrentThreadId();
	printf("Current Thread Id is: %d\n", threadId);
	sum += 1;
	Sleep(1000);
	return 0;
}

int main(int argc, char* argv[]) {
	int threadId = GetCurrentThreadId();
	printf("Current Thread Id is: %d\n", threadId);
	//HANDLE hThread = CreateThread(NULL, 0, MyThread, NULL, 0, NULL);
	//WaitForSingleObject(hThread, INFINITE);
	//CloseHandle(hThread);

	const int max_thread = 64;
	HANDLE hThread[max_thread];
	for (int i = 0; i < max_thread; i++) {
		hThread[i] = CreateThread(NULL, 0, MyThread, NULL, 0, NULL);
	}
	WaitForMultipleObjects(max_thread, hThread, true, INFINITE); // doi duoc toi da 64 objects
	for (int i = 0; i < max_thread; i++) {
		CloseHandle(hThread[i]);
	}
	printf("sum = %d", sum);
	return 0;

	// dang dung o 1h50' 26/3
}