#include "GameSample.h"

GameSample::GameSample(const wstring& name, int width, int height, bool vSync) : super(name, width, height, vSync)
{

}

GameSample::~GameSample()
{

}

bool GameSample::LoadContent()
{
	if (!super::LoadContent()) return false;

	return true;
}

void GameSample::UnloadContent()
{
	super::UnloadContent();
}

void GameSample::Destroy()
{
	super::Destroy();
}

void GameSample::OnUpdate(UpdateEventArgs& e)
{
	super::OnUpdate(e);
}

void GameSample::OnKeyPressed(KeyEventArgs& e)
{
	super::OnKeyPressed(e);
}

void GameSample::OnKeyReleased(KeyEventArgs& e)
{
	super::OnKeyReleased(e);
}

void GameSample::OnMouseWheel(MouseWheelEventArgs& e)
{
	super::OnMouseWheel(e);
}

void GameSample::OnMouseMoved(MouseMotionEventArgs& e)
{
	super::OnMouseMoved(e);
}

void GameSample::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
	super::OnMouseButtonPressed(e);
}

void GameSample::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
	super::OnMouseButtonReleased(e);
}

void GameSample::OnResize(ResizeEventArgs& e)
{
	super::OnResize(e);
}
