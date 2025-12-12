add_subdirectory(glm)
add_subdirectory(directx-headers)

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
add_subdirectory(tinygltf)

# compile imgui into a static lib
file(GLOB IMGUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx12.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx12.cpp
)

file(GLOB D3D12MA_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/D3D12MemoryAllocator/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp
)

# compile dxc
add_library(
    tinyd3ddxc
    SHARED
    IMPORTED
)

add_library(
    tinyd3ddxil
    SHARED
    IMPORTED
)

# tell lib dxc where to find the .dll file
set_target_properties(
    tinyd3ddxc
    PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dxc/lib/x86/dxcompiler.lib
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/dxc/inc
)

set_target_properties(
    tinyd3ddxil
    PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dxc/lib/x86/dxil.lib
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/dxc/inc
)

# Compile imgui lib
add_library(
    imgui
    STATIC
    ${IMGUI_SRC}
)

target_include_directories(
    imgui
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

# Compile D3DMA lib
add_library(
    d3dma
    STATIC
    ${D3D12MA_SRC}
)

target_include_directories(
    d3dma
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/D3D12MemoryAllocator/include
)