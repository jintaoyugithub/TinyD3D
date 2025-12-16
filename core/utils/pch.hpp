#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>
#include <utils/helper.hpp>
#include <dxgi1_6.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp> 
#include <glm/ext/matrix_clip_space.hpp> 

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

namespace tinyd3d {
using namespace Microsoft::WRL;
// Window handler
using WindowInstance = HINSTANCE;
using WindowHandler = HWND;

// Math
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using ivec2 = glm::ivec2;
using uvec2 = glm::uvec2;
using mat4x4 = glm::mat4;

// TODO: better way?
//#define translate glm::translate

struct Vertex {
	vec3 position;
	vec4 color;
};

struct Fence {
    ComPtr<ID3D12Fence> fence;
    inline static uint64_t fenceValue = 0;
};

}
