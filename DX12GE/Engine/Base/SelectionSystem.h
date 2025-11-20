#pragma once
#include <vector>
#include <map>
#include <string>

#include "../Graphics/Object3DEntity.h"
#include "../Graphics/DebugRenderSystem.h"
#include "../Graphics/TextureBuffer.h"

#include "Events.h"

class SelectionSystem
{
private:
	std::vector<Object3DEntity*> m_Selected;

	std::map<std::string, Object3DEntity*>& m_Objects;
	std::shared_ptr<DebugRenderSystem> m_DebugSystem;
	shared_ptr<TextureBuffer> m_IdTextureBuffer;

	void GetObjectIdAt(UINT x, UINT y, UINT* outObjectID);

public:
	SelectionSystem(std::map<std::string, Object3DEntity*>& objects, std::shared_ptr<DebugRenderSystem> debugSystem, shared_ptr<TextureBuffer> idTextureBuffer);
	void Update();
	void OnMouseButtonPressed(MouseButtonEventArgs& e);

	std::vector<Object3DEntity*>& GetSelected() { return m_Selected; }
};