#include "AssimpModelLoader.h"

#include "Application.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "VertexStructures.h"

bool AssimpModelLoader::LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath, vector<BaseObject*>& Meshes, vector<Material*>& Materials, vector<int>& MaterialIndices, float& OutYOffset)
{
    Assimp::Importer importer;
    
    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices |
        aiProcess_SortByPType |
        aiProcess_JoinIdenticalVertices
        /*aiProcessPreset_TargetRealtime_Quality |
        aiProcessPreset_TargetRealtime_MaxQuality | 
        aiProcess_FlipUVs*/
    );

    if (!pScene)
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return false;
    }

    int meshesCount = pScene->mNumMeshes;
    int materialsCount = pScene->mNumMaterials;

    const size_t last_slash_idx = filePath.rfind('/');
    string directory;
    if (string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    if(!(meshesCount > 0 && materialsCount > 0)) 
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return false;
    }

    for (unsigned int i = 0; i < materialsCount; i++)
    {
        Materials.push_back(new Material());

        for (int tt = 0; tt <= 27; tt++)
        {
            aiString path;
            aiTextureType texType = (aiTextureType)tt;
            pScene->mMaterials[i]->GetTexture(texType, 0, &path, NULL, NULL, NULL, NULL, NULL);

            string p = path.C_Str();

            if (p != "")
            {
                Materials[i]->m_ImagePaths[(TextureType)tt] = directory + "/" + p;
            }
        }

        Materials[i]->Load(commandList);
    }

    float yOffset = 0.0f;

    for (unsigned int i = 0; i < meshesCount; i++)
    {
        Meshes.push_back(new BaseObject());
        MaterialIndices.push_back(0);

        const aiMesh* paiMesh = pScene->mMeshes[i];

        vector<VertexStruct> Vertices;
        vector<WORD> Indices;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
        {
            const aiVector3D* pPos = &(paiMesh->mVertices[i]);
            const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &Zero3D;
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;
            const aiVector3D* pTangent = &Zero3D;
            const aiVector3D* pBitangent = &Zero3D;
            if (paiMesh->HasTangentsAndBitangents())
            {
                pTangent = &(paiMesh->mTangents[i]);
                pBitangent = &(paiMesh->mBitangents[i]);
            }

            VertexStruct v({
                XMFLOAT3(pPos->x, pPos->y, pPos->z),
                XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
                XMFLOAT2(pTexCoord->x, pTexCoord->y),
                XMFLOAT3(pTangent->x, pTangent->y, pTangent->z),
                XMFLOAT3(pBitangent->x, pBitangent->y, pBitangent->z)});

            yOffset = pPos->y < yOffset ? pPos->y : yOffset;

            Vertices.push_back(v);
        }

        MaterialIndices[i] = paiMesh->mMaterialIndex;

        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
        {
            const aiFace& Face = paiMesh->mFaces[i];

            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }

        Meshes[i]->OnLoad<VertexStruct>(commandList, Vertices, Indices);
    }

    OutYOffset = yOffset;
}