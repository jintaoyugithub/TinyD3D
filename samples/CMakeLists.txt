foreach(sample IN LISTS SAMPLES_BUILD_OPTIONS)
    if(sample STREQUAL "D3DHelloTriangle" OR sample STREQUAL "D3DAll")
        add_subdirectory(helloTriangle)
    endif()

    # Add other sampels
endforeach()
