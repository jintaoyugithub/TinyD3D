#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloTriangle.hpp"
#include <Windows.h>
#include <memory>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    tinyd3d::ApplicationInfoDesc desc{};
    desc.name = L"Test";
    desc.nCmdShow = nCmdShow;
    desc.windowInstance = hInstance;

    auto app = std::make_unique<tinyd3d::Application>(desc);
    app->init();

    std::shared_ptr<ElemHelloTriangle> helloTriangle;

    app->addElement(helloTriangle);

    app->run();

    return 0;
}