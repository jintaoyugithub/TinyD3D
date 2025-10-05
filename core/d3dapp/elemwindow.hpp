#pragma once

#include <common.hpp>
#include "application.hpp"

namespace tinyd3d {
class ElemWindow : public IAppElement {
public:
    ElemWindow() = default;
    ~ElemWindow() = default;
    ElemWindow(WindowConfig& config) : m_config(config) {};

public:
    void onAttach(Application* app) override;
    void onDetach() override;
    void preRender() override;
    void onRender(ID3D12GraphicsCommandList* cmd) override;
    void onUIRender() override;
    void onResize() override;
    void postRender(ID3D12GraphicsCommandList* cmd) override;

    // Getters
    inline WindowHandler getMainWindow() const { return m_mainWindow; };

private:
    void initWindow();

private:
    WindowConfig m_config;
    WindowHandler m_mainWindow{ NULL };
};
}