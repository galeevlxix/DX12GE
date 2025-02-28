#include "BaseObject.h"
#include <string>
#include <vector>

using namespace std;

class BianObject
{
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList, const string& filePath);
	void OnUpdate(double deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
private:
	vector<BaseObject> m_Meshes;
};