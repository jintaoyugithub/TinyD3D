#include <directx/d3dx12.h>
#include <d3d12.h>
#include "gltfloader.hpp"
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

static tinygltf::TinyGLTF m_gltfLoader;

void tinyd3d::GLTFLoader::load(const std::string& filename)
{
	if (!std::filesystem::exists(filename))
		throw std::runtime_error("gltf filename does not exist!");

	bool ret = false;
	if (filename.find(".glb") != std::string::npos) {
		ret = m_gltfLoader.LoadBinaryFromFile(m_model.get(), &m_err, &m_warn, filename);
	}
	else {
		ret = m_gltfLoader.LoadASCIIFromFile(m_model.get(), &m_err, &m_warn, filename);
	}

	if (!m_err.empty()) throw std::runtime_error(m_err);
	if (!ret) throw std::runtime_error("Fail to load gltf model");

	ComPtr<ID3D12GraphicsCommandList> tempCmd;
	ComPtr<ID3D12CommandAllocator> tempAlloc;
	// upload all the buffers
	for (auto& buffer : m_model->buffers) {
		tempAlloc->Reset();

		CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_HEAP_PROPERTIES uploadHeap(D3D12_HEAP_TYPE_UPLOAD);

		ComPtr<ID3D12Resource> GPURes;
		ComPtr<ID3D12Resource> CPURes;
		auto bufferSize = buffer.data.size() * sizeof(unsigned char);

		m_device->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&GPURes)
		);

		m_device->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&CPURes)
		);

		D3D12_SUBRESOURCE_DATA bufferData{};
		bufferData.pData = buffer.data.data();
		bufferData.RowPitch = bufferSize;
		bufferData.SlicePitch = bufferData.RowPitch;

		m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&tempAlloc));
		auto hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, tempAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmd));

		UpdateSubresources(
			tempCmd.Get(),
			GPURes.Get(),
			CPURes.Get(),
			0,
			0,
			1,
			&bufferData
		);

		tempCmd->Close();

		ID3D12CommandList* cmds = { tempCmd.Get() };
		m_cpQueue->ExecuteCommandLists(1, &cmds);

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc{};
		desc.BufferLocation = CPURes->GetGPUVirtualAddress();
		desc.SizeInBytes = bufferSize;
		
		D3D12_SHADER_RESOURCE_VIEW_DESC desc2{};

		ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc{};
		//m_device->CreateDescriptorHeap;

		//CD3DX12_GPU_DESCRIPTOR_HANDLE

		//m_device->CreateConstantBufferView(&desc, );
		

		// sync
	}
}

void tinyd3d::GLTFLoader::traversalNodes(int sceneID)
{
	for (auto& child : m_model->scenes[sceneID].nodes) m_scene.AddNodes(loadNode(child));
}

std::unique_ptr<tinyd3d::Node> tinyd3d::GLTFLoader::loadNode(int nodeID)
{
	auto node = std::make_unique<tinyd3d::Node>();

	mat4x4 trans, scale, rotation;
	trans = scale = rotation = mat4x4(1.0f);
	tinygltf::Node curNode = m_model->nodes[nodeID];
	node->meshID = curNode.mesh;

	// don't know if it's row based or col based
	//if (!curNode.matrix.empty()) localTransform = mat4x4(curNode.matrix);

	auto& s = curNode.scale;
	auto& r = curNode.rotation;
	auto& t = curNode.translation;

	// obtain the transform matrices
	scale = glm::scale(scale, vec3(s[0], s[1], s[2]));
	auto q = glm::quat(r[0], r[1], r[2], r[3]);
	rotation = glm::toMat4(q);
	trans = glm::translate(trans, vec3(t[0], t[1], t[2]));

	node->localTransform = trans * rotation * scale;

	for (auto childID : curNode.children) node->children.push_back(loadNode(childID));

	return node;
}

void tinyd3d::GLTFLoader::buildScene(Scene* scene, int sceneID)
{
	if (sceneID > m_model->scenes.size()) throw std::runtime_error("Scene index out of range");

	//if (sceneID == m_curScene) return;

	traversalNodes(sceneID);
}


void tinyd3d::GLTFLoader::loadMeshes(ID3D12GraphicsCommandList* cmd)
{
	// how to store the same vertices data if different nodes
	// share the same primitives
	for (auto& node : m_scene.SceneNodes()) {
		auto& curMesh = m_model->meshes[node->meshID];

		Mesh mesh;
		for (auto& primitive : curMesh.primitives) {
			SubMesh sm;
			sm.matID = primitive.material;

			for (const auto& [attr, idx] : primitive.attributes) {
				const auto& tempAC = m_model->accessors[idx];
				const auto& tempBV = m_model->bufferViews[tempAC.bufferView];

				if (attr == "POSITION") {
					sm.positionBV.bufferId = tempBV.buffer;
					sm.positionBV.pStart = m_model->buffers[tempBV.buffer].data[tempBV.byteOffset + tempAC.byteOffset];
					sm.positionBV.count = tempAC.count;
					sm.positionBV.lenInByte = tempAC.count * sizeof(float);
					sm.positionBV.strideInByte = tempBV.byteStride;
					sm.positionBV.dataType = tempAC.type;
					sm.positionBV.elemType = tempAC.componentType;
					continue;
				}

				if (attr == "NORMAL") {
					sm.normalBV.bufferId = tempBV.buffer;
					sm.normalBV.pStart = m_model->buffers[tempBV.buffer].data[tempBV.byteOffset + tempAC.byteOffset];
					sm.normalBV.count = tempAC.count;
					sm.normalBV.lenInByte = tempAC.count * sizeof(float);
					sm.normalBV.strideInByte = tempBV.byteStride;
					sm.normalBV.dataType = tempAC.type;
					sm.normalBV.elemType = tempAC.componentType;
					continue;
				}

				if (attr == "TANGENT") {
					sm.tangentBV.bufferId = tempBV.buffer;
					sm.tangentBV.pStart = m_model->buffers[tempBV.buffer].data[tempBV.byteOffset + tempAC.byteOffset];
					sm.tangentBV.count = tempAC.count;
					sm.tangentBV.lenInByte = tempAC.count * sizeof(float);
					sm.tangentBV.strideInByte = tempBV.byteStride;
					sm.tangentBV.dataType = tempAC.type;
					sm.tangentBV.elemType = tempAC.componentType;
					continue;
				}

				// There could be multiple texture coordinates
				if (attr.rfind("TEXCOORD_", 0) == 0) {
					BufferView tempView;
					tempView.bufferId = tempBV.buffer;
					tempView.pStart = m_model->buffers[tempBV.buffer].data[tempBV.byteOffset + tempAC.byteOffset];
					tempView.count = tempAC.count;
					tempView.lenInByte = tempAC.count * sizeof(float);
					tempView.strideInByte = tempBV.byteStride;
					tempView.dataType = tempAC.type;
					tempView.elemType = tempAC.componentType;
					sm.texCoordsBV.push_back(std::move(tempView));
				}
			}

			// Check if the primitive has indices buffer
			if (primitive.indices != -1) {
				const auto& tempAC = m_model->accessors[primitive.indices];
				const auto& tempBV = m_model->bufferViews[tempAC.bufferView];

				sm.indicesBV.bufferId = tempBV.buffer;
				sm.tangentBV.pStart = m_model->buffers[tempBV.buffer].data[tempBV.byteOffset + tempAC.byteOffset];
				sm.tangentBV.count = tempAC.count;
				sm.tangentBV.lenInByte = tempAC.count * sizeof(float);
				sm.tangentBV.strideInByte = tempBV.byteStride;
				sm.tangentBV.dataType = tempAC.type;
				sm.tangentBV.elemType = tempAC.componentType;
			}

			mesh.subMeshes.push_back(std::move(sm));
		}

		m_scene.AddMesh(node->meshID, std::move(mesh));
	}
}
