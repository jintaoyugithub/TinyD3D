#include "elemwindow.hpp"
#include "imgui.h"

// Function actually handle the window inputs and events
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        return 0;
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

void tinyd3d::ElemWindow::onRender(ID3D12GraphicsCommandList* cmd)
{
}

void tinyd3d::ElemWindow::onUIRender()
{
}

void tinyd3d::ElemWindow::onResize()
{
}

void tinyd3d::ElemWindow::postRender(ID3D12GraphicsCommandList* cmd)
{
}
