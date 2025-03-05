#include "BianObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void BianObject::OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath)
{
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        //aiProcess_GenNormals |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs);

    if (pScene)
    {
        int meshesCount = pScene->mNumMeshes;
        int materialsCount = pScene->mNumMaterials;

        /*XMFLOAT3 colors[8] =
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
        int colorIndex = 0;   */     

        for (unsigned int i = 0; i < materialsCount; i++)
        {
            m_Materials.push_back(Material());
            for (int tt = 0; tt <= 27; tt++)
            {
                aiString path;
                pScene->mMaterials[i]->GetTexture((aiTextureType)tt, 0, &path, NULL, NULL, NULL, NULL, NULL);
                m_Materials[i].m_ImagePaths[(Material::TextureType)tt] = path.C_Str();
            }            
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

            //colorIndex++;
            //colorIndex = colorIndex % 8;

            for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) 
            {
                const aiFace& Face = paiMesh->mFaces[i];
                //assert(Face.mNumIndices == 3);
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
        m_Meshes[i].OnRender(commandList, viewProjMatrix);
    }
}

Material::Material()
{
    for (int i = 0; i <= 27; i++)
    {
        m_ImagePaths.insert({ (TextureType)i, ""});
    }
}
