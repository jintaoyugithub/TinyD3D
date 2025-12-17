set(LIB_NAME tinyd3d)

file(GLOB LIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12Backend/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12Backend/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3dapp/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3dapp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/helper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/pch.hpp
    #${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp
)

add_library(
    ${LIB_NAME}
    STATIC
    ${LIB_SRC}
)

target_include_directories(
    ${LIB_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/core/
    ${CMAKE_SOURCE_DIR}/thirdparty/directx-headers/include/
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/tinygltf
    ${CMAKE_SOURCE_DIR}/thirdparty/D3D12MemoryAllocator/include
    ${CMAKE_SOURCE_DIR}/thirdparty/directxTex/DirectXTex
)

target_compile_features(
    ${LIB_NAME}
    PRIVATE 
    cxx_std_20
)