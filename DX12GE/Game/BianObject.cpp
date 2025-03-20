#include "BianObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "DirectXTex.h"
#include "../Engine/Application.h"

void BianObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath)
{
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices
    );

    if (pScene)
    {
        int meshesCount = pScene->mNumMeshes;
        int materialsCount = pScene->mNumMaterials;

        string directory;
        const size_t last_slash_idx = filePath.rfind('/');
        if (string::npos != last_slash_idx)
        {
            directory = filePath.substr(0, last_slash_idx);
        }

        for (unsigned int i = 0; i < materialsCount; i++)
        {
            m_Materials.push_back(Material());
            for (int tt = 0; tt <= 27; tt++)
            {
                aiString path;
                pScene->mMaterials[i]->GetTexture((aiTextureType)tt, 0, &path, NULL, NULL, NULL, NULL, NULL);
                
                string p = path.C_Str();

                if (pScene->mName.C_Str() == "blockbench_export")
                {
                    p = "gltf_embedded_0.png";
                }
                else if (filePath == "../../DX12GE/Resources/Katamari Objects/low-poly-cup-with-lemon-tea/Cup.fbx")
                {
                    p = "MadeiraHouse_Atlas_T.png";
                }
                else if (filePath == "../../DX12GE/Resources/Katamari Objects/low-poly-stylized-juice/model.dae")
                {
                    p = "Juice Cup_albedo.jpg";
                }
                else if (filePath == "../../DX12GE/Resources/Katamari Objects/old-wooden-chair-low-poly/chair.fbx")
                {
                    p = "chair_Albedo.png";
                }
                else if (filePath == "../../DX12GE/Resources/Katamari Objects/toothbrush/model.dae")
                {
                    p = "1_albedo.jpg";
                }
                else if (filePath == "../../DX12GE/Resources/Katamari Objects/bulb/Low_Poly_Light_Bulb.fbx")
                {
                    p = "#LMP0003_Textures_COL_2k.png";
                }
                
                if (p != "")
                {
                    {
                        int slashInd = p.rfind('\\');
                        if (string::npos != slashInd)
                        {
                            p = p.substr(slashInd + 1, p.length() - slashInd);
                        }
                    }

                    {
                        int slashInd = p.rfind('/');
                        if (string::npos != slashInd)
                        {
                            p = p.substr(slashInd + 1, p.length() - slashInd);
                        }
                    }

                    m_Materials[i].m_ImagePaths[(Material::TextureType)tt] = directory + "/" + p;
                }
            }    

            m_Materials[i].Load(commandList);
        }

        float yOffset = 0.0f;

        for (unsigned int i = 0; i < meshesCount; i++)
        {
            m_Meshes.push_back(BaseObject());
            const aiMesh* paiMesh = pScene->mMeshes[i];

            vector<VertexStruct> Vertices;
            vector<WORD> Indices;

            const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
            for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) 
            {
                const aiVector3D* pPos = &(paiMesh->mVertices[i]);
                const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : & Zero3D;
                const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

                VertexStruct v({ XMFLOAT3(pPos->x, pPos->y, pPos->z), XMFLOAT3(pNormal->x, pNormal->y, pNormal->z), XMFLOAT2(pTexCoord->x, pTexCoord->y)});

                yOffset = pPos->y < yOffset ? pPos->y : yOffset;
                
                Vertices.push_back(v);
            }

            m_Meshes[i].MaterialIndex = paiMesh->mMaterialIndex;

            for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) 
            {
                const aiFace& Face = paiMesh->mFaces[i];

                Indices.push_back(Face.mIndices[0]);
                Indices.push_back(Face.mIndices[1]);
                Indices.push_back(Face.mIndices[2]);
            }

            m_Meshes[i].CreateMesh(Vertices, Indices);
            m_Meshes[i].OnLoad(commandList, Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(1, 1, 1));
        }

        Move(0, -yOffset, 0);
    }
    else
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
    }

    Position = Vector3(0, 0, 0);
}

void BianObject::OnUpdate(double deltaTime )
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].OnUpdate(deltaTime);
    }
}

void BianObject::OnUpdateRotMat(double deltaTime, XMMATRIX rotMat)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].OnUpdateByRotMat(deltaTime, rotMat);
    }
}

void BianObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        if (m_Materials[m_Meshes[i].MaterialIndex].CanDrawIt())
        {
            m_Materials[m_Meshes[i].MaterialIndex].Render(commandList);
            m_Meshes[i].OnRender(commandList, viewProjMatrix);
        }        
    }
}

void Material::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    auto device = Application::Get().GetDevice();

    // load image data from disk
    string path = " ";
    if (m_ImagePaths.size() != 0)
    {
        path = m_ImagePaths[TextureType::DIFFUSE];
    }
    else
    {
        DrawIt = false;
        return;
    }

    ScratchImage image;
    TexMetadata metadata;
    ThrowIfFailed(
        LoadFromWICFile(
            (wstring(path.begin(), path.end())).c_str(), 
            WIC_FLAGS_FORCE_RGB, 
            nullptr, 
            image));

    // generate mip chain
    ScratchImage mipChain;
    ThrowIfFailed(
        GenerateMipMaps(
            *image.GetImages(), 
            TEX_FILTER_BOX, 
            0,
            mipChain));

    // create resource 
    const auto& chainBase = *mipChain.GetImages();

    D3D12_RESOURCE_DESC texDesc;
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = (UINT)chainBase.width;
    texDesc.Height = (UINT)chainBase.height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 0; // (UINT16)mipChain.GetImageCount();
    texDesc.Format = chainBase.format;
    texDesc.SampleDesc = DXGI_SAMPLE_DESC();
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    texDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

    CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };

    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_Texture)));

    // collect subresource data
    vector<D3D12_SUBRESOURCE_DATA> subresources((int)mipChain.GetImageCount());
    for (int i = 0; i < (int)mipChain.GetImageCount(); i++)
    {
        const auto img = mipChain.GetImage(i, 0, 0);
        auto& subresource = subresources[i];
        subresource.pData = img->pixels;
        subresource.RowPitch = (LONG_PTR)img->rowPitch;
        subresource.SlicePitch = (LONG_PTR)img->slicePitch;
    }

    // create the intermediate upload buffer
    //CD3DX12_HEAP_PROPERTIES heapProps2{ D3D12_HEAP_TYPE_UPLOAD };
    CD3DX12_HEAP_PROPERTIES heapProps2{ D3D12_HEAP_TYPE_UPLOAD };
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, static_cast<uint32_t>(subresources.size()));
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ThrowIfFailed(
        device->CreateCommittedResource(
            &heapProps2, 
            D3D12_HEAP_FLAG_NONE, 
            &resourceDesc, 
            D3D12_RESOURCE_STATE_GENERIC_READ, 
            nullptr, 
            IID_PPV_ARGS(&uploadBuffer)));

    // write commands to copy data to upload texture (copying each subresource)
    UpdateSubresources(
        commandList.Get(), 
        m_Texture.Get(), 
        uploadBuffer.Get(), 
        0, 
        0, 
        static_cast<uint32_t>(subresources.size()), 
        subresources.data());

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList->ResourceBarrier(1, &barrier);

    // DESCRIPTOR HEAP FOR SHADER RESOURCE VIEW
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { };
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(
        device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SRVHeap)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_SRVHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
    srvDesc.Format = m_Texture->GetDesc().Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = m_Texture->GetDesc().MipLevels;

    device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, srvHandle);
}

void Material::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetDescriptorHeaps(1, m_SRVHeap.GetAddressOf());
    commandList->SetGraphicsRootDescriptorTable(1, m_SRVHeap.Get()->GetGPUDescriptorHandleForHeapStart());
}


void BianObject::SetPosition(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetPosition(x, y, z);
    }
    Position = Vector3(x, y, z);
}

void BianObject::SetPosition(Vector3 PositionVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetPosition(PositionVector);
    }

    Position = PositionVector;
}

void BianObject::Move(float dx, float dy, float dz)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Move(dx, dy, dz);
    }

    Position = Position + Vector3(dx, dy, dz);
}

void BianObject::Move(Vector3 MoveVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Move(MoveVector);
    }
    Position = Position + MoveVector;
}

void BianObject::SetRotation(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotation(x, y, z);
    }
    Rotation = Vector3(x, y, z);
}

void BianObject::SetRotation(Vector3 RotationVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotation(RotationVector);
    }
    Rotation = RotationVector;
}

void BianObject::SetRotationX(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotationX(value);
    }
    Rotation = Vector3(value, Rotation.Y, Rotation.Z);
}

void BianObject::SetRotationY(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotationY(value);
    }
    Rotation = Vector3(Rotation.X, value, Rotation.Z);
}

void BianObject::SetRotationZ(float value)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetRotationZ(value);
    }
    Rotation = Vector3(Rotation.X, Rotation.Y, value);
}

void BianObject::Rotate(Vector3 RotateVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Rotate(RotateVector);
    }
    Rotation = Rotation + RotateVector;
}

void BianObject::SetScale(float x, float y, float z)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetScale(x, y, z);
    }
}

void BianObject::SetScale(Vector3 ScaleVector)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].SetScale(ScaleVector);
    }
}

void BianObject::Expand(float ExpandValue)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Expand(ExpandValue);
    }
}
