#include "../../Base/Singleton.h"
#include "../../Graphics/ResourceStorage.h"

SkyBoxNode::SkyBoxNode() : Object3DNode()
{
	m_Type = NODE_TYPE_SKYBOX;
	Rename("SkyBoxNode");
    m_TextureId = -1;
    Transform.SetScale(500.0f);
}

bool SkyBoxNode::Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    std::vector<XMFLOAT3> cubeVertices =
    {
        {-1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f,  1.0f},
        {-1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f,  1.0f},
        {-1.0f, -1.0f,  1.0f}
    };

    std::vector<WORD> indices =
    {
        // Front face
        0, 1, 2,
        0, 2, 3,

        // Back face
        5, 4, 7,
        5, 7, 6,

        // Left face
        4, 0, 3,
        4, 3, 7,

        // Right face
        1, 5, 6,
        1, 6, 2,

        // Top face
        4, 5, 1,
        4, 1, 0,

        // Bottom face
        3, 2, 6,
        3, 6, 7
    };

	m_TextureId = ResourceStorage::AddTexture(filePath);
	auto textureComponent = ResourceStorage::GetTexture(m_TextureId);
    textureComponent->OnLoadCubemap(commandList, filePath);
    m_BoxMesh.OnLoad(commandList, cubeVertices, indices);

    if (!textureComponent->IsInitialized() || !m_BoxMesh.IsInitialized())
    {
        m_TextureId = -1;
		return false;
    }
    IsVisible = true;
	return true;
}

void SkyBoxNode::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix)
{
    if (!IsValid() || !IsVisible) return;

    XMMATRIX wvp = Transform.GetLocalScaleMatrix();
    XMMATRIX mvp = XMMatrixMultiply(wvp, viewProjMatrix);
    commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);

    RenderTexture(commandList, 1);
    m_BoxMesh.OnRender(commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SkyBoxNode::RenderTexture(ComPtr<ID3D12GraphicsCommandList2> commandList, int slot)
{
    if (!IsValid() || !IsVisible) return;
    ResourceStorage::GetTexture(m_TextureId)->OnRender(commandList, slot);
}

void SkyBoxNode::Destroy(bool keepComponent)
{
    m_BoxMesh.Destroy();
    if (m_TextureId != -1 && !(keepComponent || TreeHasSkyboxesWithComponentId(m_TextureId)))
    {
        ResourceStorage::DeleteTextureComponentForever(m_TextureId);
    }
    m_TextureId = -1;
    Object3DNode::Destroy(keepComponent);
}

void SkyBoxNode::SetComponentId(uint32_t newId)
{
    if (newId < 0 || newId >= ResourceStorage::TexturesCount())
    {
        printf("Ошибка: Id текстуры за пределами размера массива в ResourceStorage\n");
        return;
    }
    m_TextureId = newId;
}

const std::string SkyBoxNode::GetObjectFilePath()
{
    if (!IsValid()) return "";
    return ResourceStorage::GetTexture(m_TextureId)->GetResourcePath();
}

Node3D* SkyBoxNode::Clone(Node3D* newParrent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
    if (!cloneNode)
    {
        cloneNode = new SkyBoxNode();
    }

    Object3DNode::Clone(newParrent, cloneChildrenRecursive, cloneNode);

    if (cloneNode)
    {
        SkyBoxNode* obj3D = dynamic_cast<SkyBoxNode*>(cloneNode);
        obj3D->m_TextureId = m_TextureId;
        obj3D->m_BoxMesh = m_BoxMesh;
    }

    return cloneNode;
}

void SkyBoxNode::DrawDebug()
{
	Node3D::DrawDebug();
}

void SkyBoxNode::CreateJsonData(json& j)
{
	Object3DNode::CreateJsonData(j);

    if (IsCurrent())
    {
        j["is_current"] = true;
    }
}

void SkyBoxNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    Object3DNode::LoadFromJsonData(nodeData);

    if (nodeData.isCurrent)
    {
        SetCurrent();
    }
}

void SkyBoxNode::SetCurrent()
{
    if (IsInsideTree())
    {
        Singleton::GetNodeGraph()->m_CurrentSkyBox = this;
    }
    else
    {
        printf("Внимание! Невозможно сделать SkyBoxNode::%s активным! Узел не находится в дереве сцены!\n", m_Name.c_str());
    }
}

bool SkyBoxNode::IsCurrent()
{
    return Singleton::GetNodeGraph()->m_CurrentSkyBox == this;
}

bool SkyBoxNode::TreeHasSkyboxesWithComponentId(uint32_t id, Node3D* current)
{
    current = current == nullptr ? Singleton::GetNodeGraph()->GetRoot() : current;

    if (SkyBoxNode* sky = dynamic_cast<SkyBoxNode*>(current))
    {
        if (sky->GetComponentId() == id && sky != this)
            return true;
    }

    for (auto child : current->GetChildren())
    {
        if (TreeHasSkyboxesWithComponentId(id, child))
            return true;
    }

    return false;
}
