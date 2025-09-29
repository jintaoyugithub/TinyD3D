#include "application.hpp"

namespace tinyd3d {
// Function actually handle the window inputs and events
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_PAINT:
        break;
    
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Application::initWindow()
{
    //const wchar_t CLASS_NAME[] = L"Test";
    LPCSTR CLASS_NAME = "Test";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // create the root window
    WindowHandler rootHwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Title",
        WS_OVERLAPPEDWINDOW,

        // size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,         // Parent window    
        NULL,         // Menu
        m_hInstance,  // Instance handle
        NULL          // Additional application data
    );

    if (rootHwnd == NULL) return;

    m_windowHanlders.push_back(rootHwnd);

    // TODO
    int nCmdShow = 1;
    ShowWindow(rootHwnd, nCmdShow);
}

void Application::init()
{
    initWindow();
    //initImgui();
}

void Application::run()
{
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Application::addElement(std::shared_ptr<IAppElement> elem)
{
}
}