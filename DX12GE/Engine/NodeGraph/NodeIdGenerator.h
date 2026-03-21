#pragma once

/// \brief Generates unique IDs for nodes in the scene graph.
/// \note Used internally by the engine.
class NodeIdGenerator
{
	uint32_t currentId;
public:
	NodeIdGenerator() : currentId(0) {};

	uint32_t GetNextId()
	{
 		return currentId++;
	};
};