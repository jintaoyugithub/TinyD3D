#include "elemwindow.hpp"

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

void tinyd3d::ElemWindow::initWindow() 
{
    // register a window class with the os
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = m_config.title;
    wc.hInstance = m_config.windowInstance;

    RegisterClass(&wc);

    m_mainWindow = CreateWindowEx(
        0,
        m_config.title,
        m_config.title,
        WS_OVERLAPPEDWINDOW,
        m_config.upperLeftPos.x,
        m_config.upperLeftPos.y,
        m_config.windowSize.x,
        m_config.windowSize.y,
        nullptr,
        nullptr,
        m_config.windowInstance,
        nullptr
    );

    if (m_mainWindow == NULL) return;

    ShowWindow(m_mainWindow, m_config.nCmdShow);
}

void tinyd3d::ElemWindow::onAttach(Application* app)
{
    initWindow();
}

void tinyd3d::ElemWindow::onDetach()
{
}

void tinyd3d::ElemWindow::preRender()
{
}

void tinyd3d::ElemWindow::onRender(ID3D12CommandList* cmd)
{
}

void tinyd3d::ElemWindow::onUIRender()
{
}

void tinyd3d::ElemWindow::onResize()
{
}
