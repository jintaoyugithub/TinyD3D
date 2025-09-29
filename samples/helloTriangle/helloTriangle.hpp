#include "application.hpp"

class ElemHelloTriangle : public tinyd3d::IAppElement {
public:
    ElemHelloTriangle();
    ~ElemHelloTriangle();

public:
    void onAttach() override;
    void onDetach() override;
    void preRender() override;
    void onRender(ID3D12CommandList* cmd) override;
    void onUIRender() override;
};