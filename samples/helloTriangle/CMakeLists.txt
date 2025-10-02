project(D3DHelloTriangle)

file(GLOB d3dapp_src
    ${CMAKE_SOURCE_DIR}/core/d3dapp/*.hpp
    ${CMAKE_SOURCE_DIR}/core/d3dapp/*.cpp
)

file(GLOB helloTriangle_src
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

file(GLOB helloTriangle_shaders
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl
)

set_source_files_properties(${helloTriangle_shaders} PROPERTIES HEADER_FILE_ONLY ON)

add_executable(
    ${PROJECT_NAME}
    WIN32
    ${d3dapp_src}
    ${helloTriangle_src}
    ${helloTriangle_shaders}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/thirdparty/directx-headers/include
    ${CMAKE_SOURCE_DIR}/core/
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
    imgui
    glm
    d3d12
    d3dcompiler
    dxgi
    DirectX-Headers
)

# TODO: Group files for IDE
