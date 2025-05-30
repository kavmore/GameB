#include <stdio.h>

#pragma	warning(push,3)

#include <windows.h>

#pragma warning(pop)

#include <stdint.h>

#include "Main.h"

HWND gGameWindow; //Ĭ��ȫ�ֱ���Ϊ0

BOOL gGameIsRunning;

void* Memory;

GAMEBITMAP gBackBuffer;

MONITORINFO gMonitorInfo = { sizeof(MONITORINFO) };

int32_t gMonitorWidth;

int32_t gMonitorHeight;


int WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, int CmdShow)
{
    UNREFERENCED_PARAMETER(Instance);

    UNREFERENCED_PARAMETER(PreviousInstance);

    UNREFERENCED_PARAMETER(CommandLine);

    UNREFERENCED_PARAMETER(CmdShow);

    // ϵͳ���Ѿ�����һ������ʵ�������������󴰿�
    if (GameIsAlreadyRunning() == TRUE)
    {
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    // �����������ʧ�ܣ����������󴰿�
    if (CreateMainGameWindow() != ERROR_SUCCESS)
    {
        goto Exit;
    }

    gBackBuffer.BitmapInfo.bmiHeader.biSize         = sizeof(gBackBuffer.BitmapInfo.bmiHeader);

    gBackBuffer.BitmapInfo.bmiHeader.biWidth        = GAME_RES_WIDTH;

    gBackBuffer.BitmapInfo.bmiHeader.biHeight       = GAME_RES_HEIGHT;

    gBackBuffer.BitmapInfo.bmiHeader.biBitCount     = GAME_BPP;

    gBackBuffer.BitmapInfo.bmiHeader.biCompression  = BI_RGB;

    gBackBuffer.BitmapInfo.bmiHeader.biPlanes       = 1;

    gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.Memory == NULL)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface !", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    memset(gBackBuffer.Memory, 0x7F, GAME_DRAWING_AREA_MEMORY_SIZE);

    MSG Message = { 0 };

    gGameIsRunning = TRUE;

    while (gGameIsRunning == TRUE)  //ֻҪ��Ϸ�ڽ����У��Ͳ��ϵ�ִ��PeekMessageA()�����ϵش�windows��Ϣ�������ҵ���Ϣ������MainWindowProc������
    {                               //ÿ��while��һ��ѭ������һ֡

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) //PM_REMOVE�Ĳ�����˼�� PEEKMessageA��windows��Ϣ�������ҵ�����Ϣ �������Ƴ��� ������MainWindowProc�����У�Ȼ�����ִ��
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RenderFrameGraphics();

        Sleep(1);
            
    }


Exit:

	return (0);
}

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
    LRESULT Result = 0;

    char buf[12] = { 0 };

    _itoa_s(Message, buf, _countof(buf), 10);

    OutputDebugStringA(buf);

    OutputDebugStringA("\n");

    switch (Message)
    {

        // ���û����´��ڵ�X��ʱ���ᷢ��ʲô
        case WM_CLOSE:
        {
            gGameIsRunning = FALSE;

            // ��ִ���·�������������0����ζ������ֹͣ����
            PostQuitMessage(0);

            break;
        }
        default:
        {
           Result =  DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }
        
    return (Result);
}

DWORD CreateMainGameWindow(void)
{
    DWORD Result = ERROR_SUCCESS;

    WNDCLASSEXA WindowClass = { 0 };



    //Step 1: Registering the Window Class
    WindowClass.cbSize = sizeof(WNDCLASSEXA);

    WindowClass.style = 0;

    WindowClass.lpfnWndProc = MainWindowProc;

    WindowClass.cbClsExtra = 0;

    WindowClass.cbWndExtra = 0;

    WindowClass.hInstance = GetModuleHandleA(NULL);

    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);

    WindowClass.hbrBackground = CreateSolidBrush(RGB(255,0,255));

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME"_WINDOWCLASS";

    // ���Էֱ�������
    // SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // ����ô���û��ע��ɹ��������������
    if (RegisterClassExA(&WindowClass) == 0)
    {
        // ��¼���һ�δ��󣬲�������Result����
        Result = GetLastError();

        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        //ֱ����ת��Exit��������Result
        goto Exit;
    }

    gGameWindow = CreateWindowExA(WS_EX_CLIENTEDGE, WindowClass.lpszClassName, "Window Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, GetModuleHandleA(NULL), NULL);

    if (gGameWindow == NULL)
    {
        // ��¼���һ�δ��󣬲�������Result����
        Result = GetLastError();

        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    // ���û�л����ʾ������Ϣ����ת��Exit
    if(GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == 0 )
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto Exit;
    }

    gMonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;

    gMonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

    // ~ ��λȡ��       
    // 1111 --> 0000  |  1010  --> 0101

    // | ��λ��                                                & ��λ��
    // ������Ӧ�Ķ�����λֻҪ��һ��Ϊ1�����λΪ1������Ϊ0         ������Ӧ�Ķ����ƶ�Ϊ1ʱ�����λΪ1������Ϊ0
    // 5(0101) | 3(0011)  --> 7(0111)                          5(0101) & 3(0011) --> 1(0001)

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

    if (SetWindowPos(gGameWindow, HWND_TOPMOST, gMonitorInfo.rcMonitor.left, gMonitorInfo.rcMonitor.top, gMonitorWidth, gMonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

Exit:

    return (Result);
}

BOOL GameIsAlreadyRunning(void)
{
    HANDLE Mutex = NULL;
    
    // ����û������Ѿ�����������ô˵��ϵͳ���Ѿ�����һ������ʵ��
    Mutex = CreateMutexA(NULL,FALSE, GAME_NAME"_GameMutex");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        return(TRUE);
    }
    else {
        return(FALSE);
    }
}

void ProcessPlayerInput(void) 
{   
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0,0);
    }
}

void RenderFrameGraphics(void)
{
    // memset(gBackBuffer.Memory, 0xFF, ((GAME_RES_WIDTH * GAME_RES_HEIGHT) * 4));

    PIXEL32 Pixel = { 0 };

    Pixel.Blue = 0xff;

    Pixel.Green = 0;

    Pixel.Red = 0;

    Pixel.Alpha = 0xff;

    for(int x =0; x < GAME_RES_WIDTH * GAME_RES_HEIGHT; x++)
    { 
        memcpy((PIXEL32*)gBackBuffer.Memory + x, &Pixel, sizeof(PIXEL32));
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext,
        0,
        0,
        gMonitorWidth,
        gMonitorHeight,
        0,
        0,
        GAME_RES_WIDTH,
        GAME_RES_HEIGHT,
        gBackBuffer.Memory,
        &gBackBuffer.BitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);

    ReleaseDC(gGameWindow, DeviceContext);
}