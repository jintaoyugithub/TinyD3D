#include <d3d12.h>
#include <memory>
#include <vector>
#include <Windows.h>
#include <assert.h>

namespace tinyd3d {
// TODO: move to global settings
typedef HINSTANCE WindowInstance;
typedef HWND WindowHandler;

struct IAppElement {
    virtual void onAttach() = 0;
    virtual void onDetach() = 0;
    virtual void preRender() = 0;
    virtual void onRender(ID3D12CommandList* cmd) = 0;
    virtual void onUIRender() = 0;
};

struct ApplicationInfoDesc {
    wchar_t* name;
    // window size
    WindowInstance windowInstance;
    int nCmdShow;

    // TODO: add d3d related variables like device, queue etc.
};

class Application {
public:
    Application() = default;
    Application(ApplicationInfoDesc& appInfo) : m_appInfoDesc(appInfo) {};
    virtual ~Application() {assert(m_elements.empty());};

    void init();
    void run();
    void close();

public:
    void addElement(std::shared_ptr<IAppElement> elem);

    // Getters
    inline WindowInstance getWindowInstance() const {return m_hInstance;};

private:
    void initWindow();
    void initImgui();

private:
    std::vector<std::shared_ptr<IAppElement>> m_elements;
    WindowInstance m_hInstance;
    std::vector<WindowHandler> m_windowHanlders;
    ApplicationInfoDesc m_appInfoDesc;

    // TODO: command line arguments
};
}