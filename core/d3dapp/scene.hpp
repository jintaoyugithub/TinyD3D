#pragma once

#include <wrl.h>
#include <common.hpp>
#include <map>
#include <memory>


namespace tinyd3d {

using Microsoft::WRL::ComPtr;

class GLTFLoader;
struct Material;
struct Mesh;
struct Node;

struct MeshConstant {
	// offset in the descriptor heap
	int posOffset{ -1 };
	int normalOffset{ -1 };
	int tangentOffset{ -1 };
	std::vector<int> texCoordOffset;
};

class Scene {
public:
	Scene() = default;
	~Scene() = default;

	inline void AddMesh(const Mesh& mesh) {}
	inline void AddMaterial(const Material& mat) {}

private:
	void UploadResources();

private:
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandAllocator> m_cpAlloc;
	Queue m_cpQueue;
	Fence m_cpFence;

	ComPtr<ID3D12DescriptorHeap> m_meshCbvHeap;

	// All the res from gltf file
	std::map<int, Mesh> m_meshes;
	std::map<int, Material> m_mats;
	std::vector<std::unique_ptr<Node>> m_sceneNodes;
};
}
