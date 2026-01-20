#include "../../Graphics/AssimpModelLoader.h"
#include "../../Graphics/ResourceStorage.h"
#include "../../Base/Singleton.h"

Object3DNode::Object3DNode() : Node3D(), m_ComponentId(-1)
{
    m_Type = NODE_TYPE_OBJECT3D;
    IsVisible = false;
    Rename("Object3DNode");
}

bool Object3DNode::Create(ComPtr<ID3D12GraphicsCommandList2> commandList, const std::string& filePath)
{
    AssimpModelLoader modelLoader;
    float yOffset = 0.0f;

    uint32_t id = modelLoader.LoadModelData(commandList, filePath, yOffset);
    if (id == -1) return false;
    SetComponentId(id);
    IsVisible = true;
    return true;
}

void Object3DNode::Render(ComPtr<ID3D12GraphicsCommandList2> commandList, const DirectX::XMMATRIX& viewProjMatrix)
{
    if (!IsValid() || !IsVisible) return;

	std::shared_ptr<Object3DComponent> component = ResourceStorage::GetObject3D(m_ComponentId);
    if (!component || !component->IsInitialized()) return;

    XMMATRIX wvp = GetWorldMatrix();
    XMMATRIX mvp = XMMatrixMultiply(wvp, viewProjMatrix);

    if (Singleton::GetCurrentPass()->Get() == CurrentPass::Shadow)
    {
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvp, 0);
    }
    else if (Singleton::GetCurrentPass()->Get() == CurrentPass::Geometry)
    {
        ShaderResources::GetObjectCB()->WorldViewProjection = wvp;
        ShaderResources::GetObjectCB()->ModelViewProjection = mvp;
        ShaderResources::SetGraphicsObjectCB(commandList, 0);
    }
    else {}

    for (size_t i = 0; i < component->GetMeshCount(); i++)
    {
		Mesh3DComponent* mesh = component->GetMesh(i);

        if (Singleton::GetCurrentPass()->Get() != CurrentPass::Shadow && Singleton::GetCurrentPass()->Get() != CurrentPass::Lighting)
        {
            if (i < MaterialsOverride.size() && MaterialsOverride[i] && MaterialsOverride[i]->CanDrawIt())
            {
                MaterialsOverride[i]->Render(commandList);
            }
            else if (mesh->Material && mesh->Material->CanDrawIt())
            {
                mesh->Material->Render(commandList);
            }
            else
            {
                printf("Warning: Material for mesh %d in Object3DNode '%s' is missing or invalid. Skipping rendering of this mesh.\n", (int)i, m_Name.c_str());
                continue;
            }
        }

        mesh->OnRender(commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}

void Object3DNode::Destroy(bool keepComponent)
{
    if (m_ComponentId != -1 && !(keepComponent || TreeHasObjects3DWithComponentId(m_ComponentId)))
    {
        ResourceStorage::DeleteObject3DComponentForever(m_ComponentId);
    }
    m_ComponentId = -1;

    ClearMaterialsOverride();
    Node3D::Destroy(keepComponent);
}

void Object3DNode::SetComponentId(uint32_t newId)
{
    if (newId < 0 || newId >= ResourceStorage::ObjectsCount())
    {
        printf("Error: Component ID of 3D object is outside the array size in ResourceStorage\n");
        return;
    }
    m_ComponentId = newId;

    ClearMaterialsOverride();
    MaterialsOverride.resize(ResourceStorage::GetObject3D(m_ComponentId)->GetMeshCount());
}

void Object3DNode::UpdateTransform(SimpleMath::Matrix InTransform)
{
    Vector3 scale, translation;
    Quaternion rotation;
    
    if (m_Parent)
    {
        SimpleMath::Matrix ParentTransform = m_Parent->GetWorldMatrix();
        InTransform = InTransform * ParentTransform.Invert();
    }
    
    InTransform.Decompose(scale, rotation, translation);
    Transform.SetPosition(translation);
    Transform.SetRotation(rotation.ToEuler());
}

const std::string Object3DNode::GetObjectFilePath()
{
    if (!IsValid()) return "";
    return ResourceStorage::GetObject3D(m_ComponentId)->ResourcePath;
}

const CollisionBox& Object3DNode::GetCollisionBox()
{
    if (!IsValid())
    {
        return DefaultCollisionBox;
    }
    return ResourceStorage::GetObject3D(m_ComponentId)->Box;
}

Node3D* Object3DNode::Clone(Node3D* newParent, bool cloneChildrenRecursive, Node3D* cloneNode)
{
    if (!cloneNode)
    {
        cloneNode = new Object3DNode();
    }

    Node3D::Clone(newParent, cloneChildrenRecursive, cloneNode);

    if (cloneNode)
    {
        Object3DNode* obj3D = dynamic_cast<Object3DNode*>(cloneNode);
        obj3D->SetComponentId(m_ComponentId);
		obj3D->IsVisible = IsVisible;

        for (int i = 0; i < MaterialsOverride.size(); i++)
        {
            if (MaterialsOverride[i])
            {
                obj3D->MaterialsOverride[i] = MaterialsOverride[i]->Duplicate();
            }
        }
    }

    return cloneNode;
}

void Object3DNode::DrawDebug()
{
    Node3D::DrawDebug();
    Singleton::GetDebugRender()->DrawBoundingBox(GetCollisionBox(), GetWorldMatrix());
}

void Object3DNode::CreateJsonData(json& j)
{
	Node3D::CreateJsonData(j);

    j["file_path"] = GetObjectFilePath();
    j["is_visible"] = IsVisible;

    if (MaterialsOverride.size() == 0) return;
    
    j["materials_override"] = json::array();

    for (size_t i = 0; i < MaterialsOverride.size(); i++)
    {
        MaterialEntity* material = MaterialsOverride[i];;

        if (material)
        {
            json mat;
            mat["index"] = i;

            mat["name"] = material->Name;

            if (material->m_DiffuseTextureId >= 0 && material->m_DiffuseTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_DiffuseTextureId);
                mat["diffuse_map"] = texture->GetResourcePath();
            }
            
            if (material->m_EmissiveTextureId >= 0 && material->m_EmissiveTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_EmissiveTextureId);
                mat["emissive_map"] = texture->GetResourcePath();
            }

            if (material->m_NormalTextureId >= 0 && material->m_NormalTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_NormalTextureId);
                mat["normal_map"] = texture->GetResourcePath();
            }

            if (material->m_MetallicTextureId >= 0 && material->m_MetallicTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_MetallicTextureId);
                mat["metallic_map"] = texture->GetResourcePath();
            }

            if (material->m_RoughnessTextureId >= 0 && material->m_RoughnessTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_RoughnessTextureId);
                mat["roughness_map"] = texture->GetResourcePath();
            }

            if (material->m_GltfMetallicRoughnessTextureId >= 0 && material->m_GltfMetallicRoughnessTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_GltfMetallicRoughnessTextureId);
                mat["metallic_roughness_map"] = texture->GetResourcePath();
            }

            if (material->m_AOTextureId >= 0 && material->m_AOTextureId < ResourceStorage::TexturesCount())
            {
                std::shared_ptr<TextureComponent> texture = ResourceStorage::GetTexture(material->m_AOTextureId);
                mat["ao_map"] = texture->GetResourcePath();
            }

			j["materials_override"].push_back(mat);
        }
	}
}

void Object3DNode::LoadFromJsonData(const NodeSerializingData& nodeData)
{
    Node3D::LoadFromJsonData(nodeData);
    IsVisible = nodeData.isVisible;
}

void Object3DNode::LoadOverrideMaterials(ComPtr<ID3D12GraphicsCommandList2> commandList, const NodeSerializingData& nodeData)
{
    for (MaterialSerializingData materialData : nodeData.materials_override)
    {
        if (materialData.index < 0 || materialData.index >= MaterialsOverride.size())
            continue;

        MaterialEntity* newMaterial = ResourceStorage::GetObject3D(m_ComponentId)->GetMesh(materialData.index)->Material->Duplicate();

        std::map<TextureType, std::string> imagePaths;
        imagePaths[TextureType::DIFFUSE] = materialData.Diffuse;
        imagePaths[TextureType::NORMALS] = materialData.Normal;
        imagePaths[TextureType::EMISSIVE] = materialData.Emissive;
        imagePaths[TextureType::METALNESS] = materialData.Metallic;
        imagePaths[TextureType::DIFFUSE_ROUGHNESS] = materialData.Roughness;
        imagePaths[TextureType::GLTF_METALLIC_ROUGHNESS] = materialData.GltfMetallicRoughness;
        imagePaths[TextureType::AMBIENT_OCCLUSION] = materialData.AmbientOcclusion;
        
        newMaterial->Name = materialData.Name != "" ? materialData.Name : "NewMaterial";
        newMaterial->Load(commandList, imagePaths);

        MaterialsOverride[materialData.index] = newMaterial;
    }
}

bool Object3DNode::TreeHasObjects3DWithComponentId(uint32_t id, Node3D* current)
{
    current = current == nullptr ? Singleton::GetNodeGraph()->GetRoot() : current;

    if (Object3DNode* obj3D = dynamic_cast<Object3DNode*>(current))
    {
        if (obj3D->GetComponentId() == id && obj3D != this)
            return true;
    }

    for (auto child : current->GetChildren())
    {
        if (TreeHasObjects3DWithComponentId(id, child)) 
            return true;
    }

    return false;
}

void Object3DNode::ClearMaterialsOverride()
{
    for (size_t i = 0; i < MaterialsOverride.size(); i++)
    {
        if (MaterialsOverride[i])
        {
            delete MaterialsOverride[i];
            MaterialsOverride[i] = nullptr;
        }        
    }
	MaterialsOverride.clear();
}

