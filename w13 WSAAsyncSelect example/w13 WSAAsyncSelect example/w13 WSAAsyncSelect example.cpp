// w13 WSAAsyncSelect example.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "w13 WSAAsyncSelect example.h"
#include <WinSock2.h>
#include <stdio.h>

#define MAX_LOADSTRING 100

#define MAX_CLIENT 1024
// dung tu WM_USER tro di
#define WM_ACCEPT WM_USER + 1
#define WM_SOCKET WM_USER + 2


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND g_hWnd = 0;
SOCKET g_clients[MAX_CLIENT] = {};
int g_count = 0;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_W13WSAASYNCSELECTEXAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))  // tao 1 cua so do hoa
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_W13WSAASYNCSELECTEXAMPLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); // gui message cho cac cua so
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc; // dinh nghia ham dc goi ra de xl message dc gui cho cua so
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_W13WSAASYNCSELECTEXAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_W13WSAASYNCSELECTEXAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // winsock here

   g_hWnd = hWnd;
   WSADATA wsaData;
   int startupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (startupRes != 0) {
       int errCode = WSAGetLastError();
       printf("Khong the khoi tao thu vien WinSock, ma loi: %d", errCode); // ko co console :((
   }

   SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (s == INVALID_SOCKET) {
       int errCode = WSAGetLastError();
       printf("Khong the khoi tao socket, ma loi: %d", errCode); // ko co console :((
   }

   SOCKADDR_IN saddr;
   saddr.sin_family = AF_INET;
   saddr.sin_port = htons(8888);
   saddr.sin_addr.S_un.S_addr = ADDR_ANY;

   if (bind(s, (sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
       int errCode = WSAGetLastError();
       printf("Khong the bind socket, ma loi: %d", errCode); // ko co console :((
   }

   if (listen(s, 10) == SOCKET_ERROR) {
       int errCode = WSAGetLastError();
       printf("Khong the listend, ma loi: %d", errCode); // ko co console :((
   }

   // anh xa su kien accept va su kien tren cua so
   // WM_ACCEPT la so nguyen (tu dinh nghia) dinh nghia 1 su kien tren cua so
   // FD_ACCEPT la su kien tren socket anh xa vao su kien noi o tren
   WSAAsyncSelect(s, hWnd, WM_ACCEPT, FD_ACCEPT); 


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

// hWnd la con tro den cua so
// message la 1 so nguyen dinh danh cho su kien da xay ra o cua so do
// wParam va lParam la tham so cua su kien do
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) // HDH goi ra de xl message gui cho cua so
{
    //SOCKET s = 0;
    //SOCKET c = 0;
    //SOCKADDR_IN caddr;
    //int clen = sizeof(caddr);

    switch (message)
    {
    case WM_ACCEPT:
        {
            SOCKET s = (SOCKET)wParam;
            SOCKADDR_IN caddr;
            int clen = sizeof(caddr);
            SOCKET c = accept(s, (sockaddr*)&caddr, &clen);
            if (c == SOCKET_ERROR) {
                int errCode = WSAGetLastError();
                printf("Khong the accept, ma loi: %d", errCode); // ko co console :((
            }

            int i = 0;
            for (i = 0; i < g_count; i++) {
                if (g_clients[i] == 0) {
                    g_clients[i] = c;
                }
            }

            if (i == g_count) {
                g_clients[g_count++] = c;
            }

            WSAAsyncSelect(c, hWnd, WM_SOCKET, FD_READ | FD_CLOSE); // or cac co voi nhau
            break;
        }
    case WM_SOCKET:
        {
            SOCKET c = (SOCKET)wParam;

            // lay nua thap (2 bytes or 1 word) cua lParam va and voi co tuong ung
            if (LOWORD(lParam) & FD_READ) {
                char buffer[1024] = {};
                recv(c, buffer, 1024, 0);
                for (int i = 0; i < g_count; i++) {
                    if (g_clients[i] != c) {
                        send(g_clients[i], buffer, strlen(buffer), 0);
                    }
                }
            }
            else if (LOWORD(lParam) & FD_CLOSE) {
                for (int i = 0; i < g_count; i++) {
                    if (g_clients[i] == c) {
                        g_clients[i] = 0;
                    }
                }

                MessageBoxA(hWnd, "A client has disconnected", "Warning", MB_OK);
            }
            
            break;
        }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
