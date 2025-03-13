#include "BianObject.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

class KatamariGame
{
private:
	vector<string> m_names;
	map<string, BianObject> m_objects;

	int fieldSize = 12;
	int cellSize = 6;
public:
	void OnLoad(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void Start();

	void OnUpdate(float deltaTime);
	void OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewProjMatrix);
private:
	void CreateField(ComPtr<ID3D12GraphicsCommandList2> commandList);

	void Add(ComPtr<ID3D12GraphicsCommandList2> commandList, string name, string path);
	void Remove(string name);


	
};