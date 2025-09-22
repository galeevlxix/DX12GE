#include "GameSample.h"

GameSample::GameSample(const wstring& name, int width, int height, bool vSync) : SingleGpuGame(name, width, height, vSync)
{

}

GameSample::~GameSample()
{

}

bool GameSample::LoadContent()
{
	if (!SingleGpuGame::LoadContent()) return false;

	return true;
}

void GameSample::UnloadContent()
{
	SingleGpuGame::UnloadContent();
}

void GameSample::Destroy()
{
	SingleGpuGame::Destroy();
}

void GameSample::OnUpdate(UpdateEventArgs& e)
{
	SingleGpuGame::OnUpdate(e);
}

void GameSample::OnKeyPressed(KeyEventArgs& e)
{
	SingleGpuGame::OnKeyPressed(e);
}

void GameSample::OnKeyReleased(KeyEventArgs& e)
{
	SingleGpuGame::OnKeyReleased(e);
}

void GameSample::OnMouseWheel(MouseWheelEventArgs& e)
{
	SingleGpuGame::OnMouseWheel(e);
}

void GameSample::OnMouseMoved(MouseMotionEventArgs& e)
{
	SingleGpuGame::OnMouseMoved(e);
}

void GameSample::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
	SingleGpuGame::OnMouseButtonPressed(e);
}

void GameSample::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
	SingleGpuGame::OnMouseButtonReleased(e);
}

void GameSample::OnResize(ResizeEventArgs& e)
{
	SingleGpuGame::OnResize(e);
}
