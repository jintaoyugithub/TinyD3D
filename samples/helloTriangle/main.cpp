#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloTriangle.hpp"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    tinyd3d::ContextInfo ctxInfo{};
    ctxInfo.addQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    ctxInfo.addQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    tinyd3d::D3DContext context;
    context.Init(ctxInfo);

    tinyd3d::ApplicationInfoDesc appConfig{};
    appConfig.context = context;
    appConfig.windowConfig.title = "Hello Triangle"; // doesn't matter if there is space in between in window class registeration?
    appConfig.windowConfig.nCmdShow = nCmdShow;
    appConfig.windowConfig.windowInstance = hInstance;

    auto app = std::make_unique<tinyd3d::Application>();
    auto helloTriangle = std::make_shared<ElemHelloTriangle>();

    // TODO: read config from the json file
    app->init(appConfig);
    app->addElement(helloTriangle);

    app->run();

    return 0;
}
