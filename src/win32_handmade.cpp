#include <Windows.h>

#define INTERNAL static
#define GLOBAL static
#define LOCAL_PERSIST static

GLOBAL bool g_running{};

GLOBAL BITMAPINFO bitmapInfo;
GLOBAL void *bitmapMemory;
GLOBAL HBITMAP bitmapHandle;
GLOBAL HDC bitmapDeviceContext;

INTERNAL void win32ResizeDIBSection(int width, int height)
{
    if (bitmapHandle)
    {
        DeleteObject(bitmapHandle);
    }

    if (!bitmapDeviceContext)
    {
        bitmapDeviceContext = CreateCompatibleDC(0);
    }

    bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
    bitmapInfo.bmiHeader.biWidth = width;
    bitmapInfo.bmiHeader.biHeight = height;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    bitmapHandle = CreateDIBSection(bitmapDeviceContext, &bitmapInfo, DIB_RGB_COLORS, &bitmapMemory, 0, 0);
}

INTERNAL void win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
    StretchDIBits(deviceContext, x, y, width, height, x, y, width, height, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

	switch (message)
	{
	case WM_SIZE:
		{
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;
            win32ResizeDIBSection(width, height);
			OutputDebugStringA("VM_SIZE\n");
		}
        break;
	case WM_DESTROY:
        g_running = false;
        OutputDebugStringA("VM_DESTROY\n");
        break;
	case WM_CLOSE:
        g_running = false;
        OutputDebugStringA("VM_CLOSE\n");
        break;
	case WM_ACTIVATEAPP:
        OutputDebugStringA("VM_ACTIVATEAPP\n");
        break;
	case WM_PAINT:
		{
			PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);

			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;

            win32UpdateWindow(deviceContext, x, y, width, height);

			EndPaint(window, &paint);
		}
        break;
	default:
        result = DefWindowProc(window, message, wParam, lParam);
        break;
	}

    return result;
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode)
{
    MessageBox(0, "Hello handmade", "Handmade Hero", MB_OK | MB_ICONINFORMATION);

    WNDCLASS windowClass{};
    windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = win32MainWindowCallback;
    windowClass.hInstance = instance;
    //windowClass.hIcon = ;
    //windowClass.lpszMenuName = ;
    windowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&windowClass))
    {
        HWND windowHandle = CreateWindowEx(0, windowClass.lpszClassName, "Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, instance, nullptr);

        if (windowHandle)
        {
            g_running = true;
            while (g_running)
            {
                MSG message;
                BOOL msgResult = GetMessage(&message, nullptr, 0, 0);
                if (msgResult > 0)
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
	        // TODO: logging error
        }
    }
    else
    {
	    // TODO: logging error
    }
    return 0;
}
