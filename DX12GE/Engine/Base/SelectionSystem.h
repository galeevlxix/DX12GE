#pragma once
#include <vector>
#include <map>
#include <string>

#include "../Graphics/TextureBuffer.h"
#include "../NodeGraph/Node3D.h"
#include "Events.h"

class SelectionSystem
{
public:
	SelectionSystem();

	void SetTextureBuffer(std::shared_ptr<TextureBuffer> idTextureBuffer) { m_IdTextureBuffer = idTextureBuffer; }

private:
	std::vector<Node3D*> m_Selected;
	std::shared_ptr<TextureBuffer> m_IdTextureBuffer;

	void GetObjectIdAt(UINT x, UINT y, UINT* outObjectID);

public:
	void DrawDebug();
	void OnMouseButtonPressed(MouseButtonEventArgs& e);

	std::vector<Node3D*>& GetSelected() { return m_Selected; }
	void DeselectAll();

	void Destroy();
};