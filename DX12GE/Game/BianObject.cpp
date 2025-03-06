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
        aiProcess_FlipUVs);

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
                if (p != "")
                {
                    m_Materials[i].m_ImagePaths[(Material::TextureType)tt] = directory + "/" + p;
                }
            }    

            m_Materials[i].Load(commandList);
        }

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
            m_Meshes[i].OnLoad(commandList, Vector3(0, 0, 0), Vector3(-PI / 2, PI, 0), Vector3(2, 2, 2));
        }


    }
    else
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
    }
}

void BianObject::OnUpdate(double deltaTime)
{
    static float rot_speed = PI / 4;
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].Rotate(Vector3(0, rot_speed * deltaTime, 0));
        m_Meshes[i].OnUpdate(deltaTime);
    }
}

void BianObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Materials[m_Meshes[i].MaterialIndex].Render(commandList);
        m_Meshes[i].OnRender(commandList, viewProjMatrix);
    }
}

void Material::Load(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    auto device = Application::Get().GetDevice();

    // load image data from disk
    ScratchImage image;
    //LoadFromWICFile(L"..\\..\\DX12GE\\Resources\\Models\\garden\\textures\\Material.001_baseColor.png", WIC_FLAGS_NONE, nullptr, image);
    
    //auto 

    string path = " ";

    if (m_ImagePaths.size() != 0)
    {
        path = m_ImagePaths[TextureType::DIFFUSE];
    }
    else
    {
        path = "../../DX12GE/Resources/empty.png";
    }   
    
    wstring widestr = wstring(path.begin(), path.end());
    const wchar_t* widecstr = widestr.c_str();
    LoadFromWICFile(widecstr, WIC_FLAGS_NONE, nullptr, image);

    // generate mip chain
    ScratchImage mipChain;
    ThrowIfFailed(GenerateMipMaps(*image.GetImages(), TEX_FILTER_BOX, 0, mipChain));

    // create resource 
    const auto& chainBase = *mipChain.GetImages();

    D3D12_RESOURCE_DESC texDesc;
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = (UINT)chainBase.width;
    texDesc.Height = (UINT)chainBase.height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = (UINT16)mipChain.GetImageCount();
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

    //m_Texture.Get()->SetName(L"SingleTextue");

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
    
    CD3DX12_HEAP_PROPERTIES heapProps2{ D3D12_HEAP_TYPE_UPLOAD };
    const UINT16 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, (UINT)subresources.size());

    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    ThrowIfFailed(
        device->CreateCommittedResource(&heapProps2, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

    //uploadBuffer.Get()->SetName(L"TextureUploadBuffer");

    // write commands to copy data to upload texture (copying each subresource)
    UpdateSubresources(commandList.Get(), m_Texture.Get(), uploadBuffer.Get(), 0, 0, (UINT)subresources.size(),
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

    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = m_Texture->GetDesc().Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        srvDesc.Texture2D.MipLevels = m_Texture->GetDesc().MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.PlaneSlice = 0;

        device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, srvHandle);
    }
}

void Material::Render(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    commandList->SetDescriptorHeaps(1, m_SRVHeap.GetAddressOf());
    commandList->SetGraphicsRootDescriptorTable(1, m_SRVHeap.Get()->GetGPUDescriptorHandleForHeapStart());
}
