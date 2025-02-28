#include "BianObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void BianObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath)
{
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs);

    if (pScene)
    {
        int meshesCount = pScene->mNumMeshes;
        int materialsCount = pScene->mNumMaterials;

        XMFLOAT3 colors[8] =
        {
             XMFLOAT3(1.0f, 1.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 0.0f),
             XMFLOAT3(0.0f, 1.0f, 0.0f),
             XMFLOAT3(0.0f, 0.0f, 1.0f),
             XMFLOAT3(1.0f, 1.0f, 0.0f),
             XMFLOAT3(0.0f, 1.0f, 1.0f),
             XMFLOAT3(1.0f, 0.0f, 1.0f),
             XMFLOAT3(1.0f, 1.0f, 1.0f),
        };

        int colorIndex = 0;

        for (unsigned int i = 0; i < meshesCount; i++)
        {
            const aiMesh* paiMesh = pScene->mMeshes[i];

            //m_Meshes[Index].MaterialIndex = paiMesh->mMaterialIndex;

            m_Meshes.push_back(BaseObject());

            vector<VertexPosColor> Vertices;
            vector<WORD> Indices;

            const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
            for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) 
            {
                const aiVector3D* pPos = &(paiMesh->mVertices[i]);
                const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
                const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ?
                    &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

                VertexPosColor v({ XMFLOAT3(pPos->x, pPos->y, pPos->z), XMFLOAT3(pNormal->x, pNormal->y, pNormal->z), colors[colorIndex] } );
                
                Vertices.push_back(v);
            }

            colorIndex++;
            colorIndex = colorIndex % 8;

            for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) 
            {
                const aiFace& Face = paiMesh->mFaces[i];
                assert(Face.mNumIndices == 3);
                Indices.push_back(Face.mIndices[0]);
                Indices.push_back(Face.mIndices[1]);
                Indices.push_back(Face.mIndices[2]);
            }

            m_Meshes[i].CreateMesh(Vertices, Indices);
            m_Meshes[i].OnLoad(commandList, Vector3(0, 0, 0), Vector3(0, PI, 0), Vector3(2, 2, 2), Vector3(0, 0, 0));
        }
    }
    else
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
    }
}

void BianObject::OnUpdate(double deltaTime)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].OnUpdate(deltaTime);
    }
}

void BianObject::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix)
{
    for (int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].OnRender(commandList, viewProjMatrix);
    }
}