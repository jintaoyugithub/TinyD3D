project(D3DHelloTriangle)

file(GLOB framework
    ${CMAKE_SOURCE_DIR}/core/d3dapp/*.hpp
    ${CMAKE_SOURCE_DIR}/core/d3dapp/*.cpp
    ${CMAKE_SOURCE_DIR}/core/utils/*.hpp
    ${CMAKE_SOURCE_DIR}/core/utils/*.cpp
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
    ${framework}
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
    DirectX-Guids
)

target_compile_features(
    ${PROJECT_NAME} 
    PRIVATE 
    cxx_std_17
)

# TODO: Group files for IDE
