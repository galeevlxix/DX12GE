#include "../Singleton.h"

static CommandExecutor* Executor = nullptr;
static DebugRenderSystem* DebugRender = nullptr;
static SelectionSystem* Selection = nullptr;
static NodeGraphSystem* NodeGraph = nullptr;
static SceneJsonSerializer* Serializer = nullptr;
static CurrentPass* Pass = nullptr;
static AudioSystem* Audio = nullptr;
static PhysicsManager* PhysMgr = nullptr;
static NodeIdGenerator* NodeIdGen = nullptr;
static shared_ptr<Window> Wind = nullptr;

static bool m_IsInitialized = false;

void Singleton::Initialize()
{
	if (m_IsInitialized) return;

	NodeIdGen = new NodeIdGenerator();
	Executor = new CommandExecutor();
	DebugRender = new DebugRenderSystem();
	Selection = new SelectionSystem();
	NodeGraph = new NodeGraphSystem();
	Serializer = new SceneJsonSerializer();
	Pass = new CurrentPass();
	Audio = new AudioSystem();
	PhysMgr = new PhysicsManager();
	PhysMgr->Initialize();

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
	delete NodeIdGen;

	Executor = nullptr;
	DebugRender = nullptr;
	Selection = nullptr;
	NodeGraph = nullptr;
	Serializer = nullptr;
	Pass = nullptr;
	Audio = nullptr;
	NodeIdGen = nullptr;
	Wind = nullptr;
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

AudioSystem* Singleton::GetAudioSystem()
{
	return Audio;
}

PhysicsManager* Singleton::GetPhysicsManager()
{
	return PhysMgr;
}

NodeIdGenerator* Singleton::GetNodeIdGenerator()
{
	return NodeIdGen;
}

shared_ptr<Window> Singleton::GetWindow()
{
	return Wind;
}

void Singleton::SetWindow(shared_ptr<Window> window)
{
	Wind = window;
}

bool Singleton::IsInitialized()
{
	return m_IsInitialized;
}
