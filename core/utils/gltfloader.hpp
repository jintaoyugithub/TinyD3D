#pragma once

#include <wrl.h>
#include "tiny_gltf.h"
#include "pch.hpp"
#include <memory>

namespace tinyd3d {

using Microsoft::WRL::ComPtr;

/// <TODO>
/// Singleton?
/// </TODO>

struct Node {
	// mat4x4 globalTransform; or compute while need?
	// we can use a vector to store the path to specific nodes
	// and compute the product of these nodes' local transformation
	mat4x4 localTransform;
	uint32_t meshID;
	std::vector<std::unique_ptr<Node>> children;
};

struct Material {
	union BaseColor;
};

struct BufferView {
	int bufferId{ -1 };
	int pStart{ -1 };        // pointer to the start position of the raw data
	int lenInByte{ -1 };
	int strideInByte{ -1 };
	int count{ -1 };
	int elemType;
	int dataType;
};

struct SubMesh {
	// We don't store the data in the cpu side
	BufferView positionBV;
	BufferView normalBV;
	BufferView tangentBV;
	BufferView indicesBV;
	std::vector<BufferView> vertColorsBV; // kind rare
	std::vector<BufferView> texCoordsBV;

	// for pbr one mesh can only has one mat
	// otherwise may cause undefine behaviours
	uint32_t matID;
};

struct Mesh {
	std::vector<SubMesh> subMeshes;
};

struct Scene {
public:
	HRESULT uploadMeshData(ID3D12GraphicsCommandList* cmd) {

	}

	// Getters
	const std::vector<std::unique_ptr<Node>>& SceneNodes() const { return m_sceneNodes; };

	inline void AddNodes(std::unique_ptr<Node> node) { m_sceneNodes.push_back(std::move(node)); };
	inline void AddMesh(int idx, Mesh& mesh) { m_meshes.emplace(idx, std::move(mesh)); };

private:
	// Store the scene tree structure
	std::vector<std::unique_ptr<Node>> m_sceneNodes;

	std::map<int, Mesh> m_meshes;
	std::map<int, Material> m_materials;
};

/// <summary>
/// Load gltf file and commit all the res to gpu,
/// so that when rendering, the buffer views can be esaily uesed
/// </summary>
class GLTFLoader {
public:
	GLTFLoader() = default;
	~GLTFLoader() = default;
	GLTFLoader(ComPtr<ID3D12Device> device, ComPtr<ID3D12CommandQueue> queue) : m_device(device), m_cpQueue(queue) {};

public:
	void load(const std::string& filename);
	void reloadScene(int idx);

	// Getters
	inline std::string getErrMsg() const { return m_err; };
	inline std::string getWarnMsg() const { return m_warn; };
	std::shared_ptr<tinygltf::Model> getLoadedModel() const { return m_model; };

private:
	/// <summary>
	/// Obtain the matrix for transformation
	/// </summary>
	void traversalNodes(int sceneID);
	std::unique_ptr<Node> loadNode(int nodeID);
	void buildScene(Scene* scene, int sceneID);
	void loadMeshes(ID3D12GraphicsCommandList* cmd);
	void loadTextures();
	void loadSamplers();
	void loadMaterials();
	void loadLights();

private:
	ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_cpQueue;

	/// tiny gltf
	std::shared_ptr<tinygltf::Model> m_model;
	std::string m_err;
	std::string m_warn;

	// matrix for transformation
	// d3d buffer view for vertex
	// sampler and textures view for the image
	uint32_t m_curScene;
	Scene m_scene;

	// gltf2.0 require specify version
	const int MAJOR_VERSION = 2;
	const int MINOR_VERSION = 0;
};
}
