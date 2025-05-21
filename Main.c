#include <stdio.h>

#pragma	warning(push,3)

#include <windows.h>

#pragma warning(pop)

#include "Main.h"

HWND gGameWindow; //Ĭ��ȫ�ֱ���Ϊ0

BOOL gGameIsRunning;

void* Memory;

GAMEBITMAP gDrawingSurface;


int WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, int CmdShow)
{

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

    gDrawingSurface.BitmapInfo.bmiHeader.biSize = sizeof(gDrawingSurface.BitmapInfo.bmiHeader);

    gDrawingSurface.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;

    gDrawingSurface.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;

    gDrawingSurface.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;

    gDrawingSurface.BitmapInfo.bmiHeader.biCompression = BI_RGB;

    gDrawingSurface.BitmapInfo.bmiHeader.biPlanes = 1;

    gDrawingSurface.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gDrawingSurface.Memory == NULL)
    {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface !", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }




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

    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    WindowClass.lpszMenuName = NULL;

    WindowClass.lpszClassName = GAME_NAME"_WINDOWCLASS";


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
    short EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0,0);
    }
}

void RenderFrameGraphics(void)
{

}