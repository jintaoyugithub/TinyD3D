function(add_sample sample_name)
    project(${sample_name})

    file(GLOB SHADERS_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.hlsl
    )

    file(GLOB SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

    file(GLOB FRAMEWORK
        ${CMAKE_SOURCE_DIR}/core/d3dapp/*.hpp
        ${CMAKE_SOURCE_DIR}/core/d3dapp/*.cpp
        ${CMAKE_SOURCE_DIR}/core/utils/*.hpp
        ${CMAKE_SOURCE_DIR}/core/utils/*.cpp
    )

    set_source_files_properties(${SHADERS_SRC} PROPERTIES HEADER_FILE_ONLY ON)

    add_executable(
        ${sample_name}
        WIN32
        ${FRAMEWORK}
        ${SRC}
        ${SHADERS_SRC}
    )

    target_include_directories(
        ${sample_name}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/thirdparty/directx-headers/include
        ${CMAKE_SOURCE_DIR}/core/
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui
        ${CMAKE_SOURCE_DIR}/thirdparty/glm
    )

    target_link_libraries(
        ${sample_name}
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
        ${sample_name} 
        PRIVATE 
        cxx_std_17
    )
endfunction()

foreach(sample IN LISTS SAMPLES_BUILD_OPTIONS)
    if(sample STREQUAL "D3DHelloTriangle" OR sample STREQUAL "D3DAll")
        add_subdirectory(helloTriangle)
    endif()

    # Add other sampels
    if(sample STREQUAL "D3DHelloWorkGraph" OR sample STREQUAL "D3DAll")
        add_subdirectory(helloWorkGraph)
    endif()
endforeach()
