#include "AssimpModelLoader.h"

#include "Application.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "VertexStructures.h"

bool AssimpModelLoader::LoadModelData(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath, vector<BaseObject>* Meshes, vector<Material>* Materials, vector<int>* MaterialIndices, float *OutYOffset)
{
    Assimp::Importer importer;

    const aiScene* pScene = importer.ReadFile(filePath.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_FlipUVs |
        aiProcess_PreTransformVertices
    );

    if (!pScene)
    {
        printf("Error parsing '%s': '%s'\n", filePath.c_str(), importer.GetErrorString());
        return false;
    }

    meshesCount = pScene->mNumMeshes;
    materialsCount = pScene->mNumMaterials;

    const size_t last_slash_idx = filePath.rfind('/');
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
        (*Materials).push_back(Material());
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
                    size_t slashInd = p.rfind('\\');
                    if (string::npos != slashInd)
                    {
                        p = p.substr(slashInd + 1, p.length() - slashInd);
                    }
                }

                {
                    size_t slashInd = p.rfind('/');
                    if (string::npos != slashInd)
                    {
                        p = p.substr(slashInd + 1, p.length() - slashInd);
                    }
                }

                (*Materials)[i].m_ImagePaths[(TextureType)tt] = directory + "/" + p;
            }
        }

        (*Materials)[i].Load(commandList);
    }

    float yOffset = 0.0f;

    for (unsigned int i = 0; i < meshesCount; i++)
    {
        (*Meshes).push_back(BaseObject());
        (*MaterialIndices).push_back(0);

        const aiMesh* paiMesh = pScene->mMeshes[i];

        vector<VertexStruct> Vertices;
        vector<WORD> Indices;

        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++)
        {
            const aiVector3D* pPos = &(paiMesh->mVertices[i]);
            const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &Zero3D;
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

            VertexStruct v({ XMFLOAT3(pPos->x, pPos->y, pPos->z), XMFLOAT3(pNormal->x, pNormal->y, pNormal->z), XMFLOAT2(pTexCoord->x, pTexCoord->y) });

            yOffset = pPos->y < yOffset ? pPos->y : yOffset;

            Vertices.push_back(v);
        }

        (*MaterialIndices)[i] = paiMesh->mMaterialIndex;

        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++)
        {
            const aiFace& Face = paiMesh->mFaces[i];

            Indices.push_back(Face.mIndices[0]);
            Indices.push_back(Face.mIndices[1]);
            Indices.push_back(Face.mIndices[2]);
        }

        (*Meshes)[i].OnLoad<VertexStruct>(commandList, Vertices, Indices);
    }

    *OutYOffset = yOffset;
}
