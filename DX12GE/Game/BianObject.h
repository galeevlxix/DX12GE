#include "BaseObject.h"
#include <string>
#include <vector>

#include <map>

using namespace std;

class Material
{
public:
    enum TextureType
    {
        NONE = 0,
        DIFFUSE = 1,
        SPECULAR = 2,
        AMBIENT = 3,
        EMISSIVE = 4,
        HEIGHT = 5,
        NORMALS = 6,
        SHININESS = 7,
        OPACITY = 8,
        DISPLACEMENT = 9,
        LIGHTMAP = 10,
        REFLECTION = 11,
        BASE_COLOR = 12,
        NORMAL_CAMERA = 13,
        EMISSION_COLOR = 14,
        METALNESS = 15,
        DIFFUSE_ROUGHNESS = 16,
        AMBIENT_OCCLUSION = 17,
        UNKNOWN = 18,
        SHEEN = 19,
        CLEARCOAT = 20,
        TRANSMISSION = 21,
        MAYA_BASE = 22,
        MAYA_SPECULAR = 23,
        MAYA_SPECULAR_COLOR = 24,
        MAYA_SPECULAR_ROUGHNESS = 25,
        ANISOTROPY = 26,
        GLTF_METALLIC_ROUGHNESS = 27,
    };
    map<TextureType, string> m_ImagePaths;

    void Load(ComPtr<ID3D12GraphicsCommandList2> commandList);
    void Render(ComPtr<ID3D12GraphicsCommandList2> commandList);

private:
    ComPtr<ID3D12Resource> m_Texture;
    ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
    ComPtr<ID3D12Resource> uploadBuffer;
};

class BianObject
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath);
	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
private:
	vector<BaseObject> m_Meshes;
    vector<Material> m_Materials;	
};