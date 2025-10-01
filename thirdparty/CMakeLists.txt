add_subdirectory(glm)
add_subdirectory(directx-headers)

# compile imgui into a static lib
file(GLOB IMGUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp
)

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