#pragma once

#include "../Engine/SingleGpuGame.h"

class GameSample : public SingleGpuGame
{
public:
    using super = SingleGpuGame;

	GameSample(const wstring& name, int width, int height, bool vSync = false);
	~GameSample();

	virtual bool LoadContent() override;
	virtual void UnloadContent() override;
	virtual void Destroy() override;

protected:
    virtual void OnUpdate(UpdateEventArgs& e) override;

    virtual void OnKeyPressed(KeyEventArgs& e) override;
    virtual void OnKeyReleased(KeyEventArgs& e) override;
    virtual void OnMouseWheel(MouseWheelEventArgs& e) override;
    virtual void OnMouseMoved(MouseMotionEventArgs& e) override;
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e) override;
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e) override;
    virtual void OnResize(ResizeEventArgs& e) override;
};