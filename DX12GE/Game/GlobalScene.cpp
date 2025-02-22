#include "GlobalScene.h"

GlobalScene::GlobalScene()
{
	objects = map<string, BaseObject>();
}

GlobalScene::~GlobalScene()
{
	objects.clear();
}

//void GlobalScene::AddCube(string name, ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 Color)
//{
//
//}
//
//void GlobalScene::AddSphere(string name, ComPtr<ID3D12GraphicsCommandList2> commandList, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 Color)
//{
//}
//
//void GlobalScene::Remove(string name)
//{
//}
//
//BaseObject& GlobalScene::operator[](string name)
//{
//	// TODO: вставьте здесь оператор return
//}
//
//int GlobalScene::GetSize()
//{
//	return 0;
//}
//
//void GlobalScene::OnUpdate(double deltaTime)
//{
//}
//
//void GlobalScene::OnRender(ComPtr<ID3D12GraphicsCommandList2> commandList, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
//{
//}
