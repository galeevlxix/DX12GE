#include "../Singleton.h"

static CommandExecutor* Executor = nullptr;
static DebugRenderSystem* DebugRender = nullptr;
static SelectionSystem* Selection = nullptr;
static NodeGraphSystem* NodeGraph = nullptr;
static SceneJsonSerializer* Serializer = nullptr;
static CurrentPass* Pass = nullptr;
<<<<<<< HEAD
static AudioSystem* Audio = nullptr;
=======
static PhysicsManager* PhysMgr = nullptr;
>>>>>>> master

static bool m_IsInitialized = false;

void Singleton::Initialize()
{
	if (m_IsInitialized) return;

	Executor = new CommandExecutor();
	DebugRender = new DebugRenderSystem();
	Selection = new SelectionSystem();
	NodeGraph = new NodeGraphSystem();
	Serializer = new SceneJsonSerializer();
	Pass = new CurrentPass();
<<<<<<< HEAD
	Audio = new AudioSystem();
=======
	PhysMgr = new PhysicsManager();
	PhysMgr->Initialize();
>>>>>>> master

	m_IsInitialized = true;
}

void Singleton::Destroy()
{
	if (!m_IsInitialized) return;

	m_IsInitialized = false;

	Executor->Exit();
	DebugRender->Destroy();
	Selection->Destroy();
	NodeGraph->Destroy();
	Audio->Shutdown();

	//delete Executor;
	delete DebugRender;
	delete Selection;
	delete NodeGraph;
	delete Serializer;
	delete Pass;
	delete Audio;

	Executor = nullptr;
	DebugRender = nullptr;
	Selection = nullptr;
	NodeGraph = nullptr;
	Serializer = nullptr;
	Pass = nullptr;
	Audio = nullptr;
}

CommandExecutor* Singleton::GetExecutor()
{
	return Executor;
}

DebugRenderSystem* Singleton::GetDebugRender()
{
	return DebugRender;
}

SelectionSystem* Singleton::GetSelection()
{
	return Selection;
}

NodeGraphSystem* Singleton::GetNodeGraph()
{
	return NodeGraph;
}

SceneJsonSerializer* Singleton::GetSerializer()
{
	return Serializer;
}

CurrentPass* Singleton::GetCurrentPass()
{
	return Pass;
}

<<<<<<< HEAD
AudioSystem* Singleton::GetAudioSystem()
{
	return Audio;
=======
PhysicsManager* Singleton::GetPhysicsManager()
{
	return PhysMgr;
>>>>>>> master
}

bool Singleton::IsInitialized()
{
	return m_IsInitialized;
}
