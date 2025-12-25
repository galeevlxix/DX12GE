#include "../AssimpModelLoader.h"
#include "../../Base/Application.h"
#include "../VertexStructures.h"
#include "../ResourceStorage.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

uint32_t AssimpModelLoader::LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath, float& OutYOffset)
{
    if (NotFoundFile(filePath.c_str())) return -1;

    int id = ResourceStorage::AddObject3D(filePath);
    std::shared_ptr<Object3DComponent> object = ResourceStorage::GetObject3D(id);

    if (object->IsInitialized())
        return id;

    printf("Загрузка компонента из файла объекта: %s\n", filePath.c_str());

    Assimp::Importer importer;
    
    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices |
        aiProcess_SortByPType |
        aiProcess_JoinIdenticalVertices
    );

    if (!pScene)
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return -1;
    }

    const size_t last_slash_idx = filePath.rfind('/');
    string directory;
    if (string::npos != last_slash_idx)
    {
        directory = filePath.substr(0, last_slash_idx);
    }

    if(!(pScene->mNumMeshes > 0 && pScene->mNumMaterials > 0))
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return -1;
    }

    std::vector<MaterialEntity*> materials;

    for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
    {
        materials.push_back(new MaterialEntity());
        std::map<TextureType, std::string> imagePaths;

        for (int tt = 0; tt <= 27; tt++)
        {
            aiString path;
            aiTextureType texType = (aiTextureType)tt;
            pScene->mMaterials[i]->GetTexture(texType, 0, &path, NULL, NULL, NULL, NULL, NULL);

            string p = path.C_Str();

            if (p != "")
            {
                imagePaths[(TextureType)tt] = directory + "/" + p;
            }
        }

        materials[i]->Load(commandList, imagePaths);
    }

    std::vector<Mesh3DComponent*> meshes;

    float yOffset = 0.0f;

    for (unsigned int meshIndex = 0; meshIndex < pScene->mNumMeshes; meshIndex++)
    {
        meshes.push_back(new Mesh3DComponent());

        const aiMesh* paiMesh = pScene->mMeshes[meshIndex];

        vector<VertexStruct> Vertices;
        vector<WORD> Indices;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        for (unsigned int vertexIndex = 0; vertexIndex < paiMesh->mNumVertices; vertexIndex++)
        {
            const aiVector3D* pPos = &(paiMesh->mVertices[vertexIndex]);
            const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[vertexIndex]) : &Zero3D;
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][vertexIndex]) : &Zero3D;
            const aiVector3D* pTangent = &Zero3D;
            const aiVector3D* pBitangent = &Zero3D;
            if (paiMesh->HasTangentsAndBitangents())
            {
                pTangent = &(paiMesh->mTangents[vertexIndex]);
                pBitangent = &(paiMesh->mBitangents[vertexIndex]);
            }

            VertexStruct v({
                XMFLOAT3(pPos->x, pPos->y, pPos->z),
                XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
                XMFLOAT2(pTexCoord->x, pTexCoord->y),
                XMFLOAT3(pTangent->x, pTangent->y, pTangent->z),
                XMFLOAT3(pBitangent->x, pBitangent->y, pBitangent->z)});

            object->Box.Add(v.Position);

            yOffset = pPos->y < yOffset ? pPos->y : yOffset;

            Vertices.push_back(v);
        }

        meshes[meshIndex]->m_Material = materials[paiMesh->mMaterialIndex];

        for (unsigned int faceIndex = 0; faceIndex < paiMesh->mNumFaces; faceIndex++)
        {
            const aiFace& Face = paiMesh->mFaces[faceIndex];

            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }

        meshes[meshIndex]->OnLoad<VertexStruct>(commandList, Vertices, Indices);
    }

    OutYOffset = yOffset;

    object->OnLoad(meshes);
    object->ResourcePath = filePath;

    return id;
}