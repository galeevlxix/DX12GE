#include "../ImGuiController.h"

#include "../imgui.h"
#include "../imgui_impl_win32.h"
#include "../imgui_impl_dx12.h"
#include <tchar.h>

#include "../../Base/Application.h"
#include "../../Base/LuaManager.h"
#include "../../Base/CommandQueue.h"
#include "../../Base/Singleton.h"
#include "../../Base/SceneJsonSerializer.h"

#include "../../Graphics/DescriptorHeaps.h"
#include "../../Graphics/ResourceStorage.h"

static bool isInitialized = false;
static bool isShowingScriptsToAdd = false;
static bool isShowingScriptsToRemove = false;
static bool DrawUI = false;
static bool Started = false;

/////////////////////////// FOR FILE SYSTEM MANAGER

struct FileBrowserState
{
	fs::path root;
	fs::path current;

	bool showHidden = false;
	bool recursiveTree = true;
};

static bool IsHiddenWin(const fs::path& p)
{
	auto name = p.filename().string();
	return !name.empty() && name[0] == '.';
}

static bool ShouldSkip(const fs::directory_entry& e, bool showHidden)
{
	if (showHidden) return false;
	return IsHiddenWin(e.path());
}

static FileBrowserState fb;

///////////////////////////

static const int APP_NUM_FRAMES_IN_FLIGHT = 2;

std::shared_ptr<Window> MainWindow;

static ComPtr<ID3D12Device2> device;
static std::shared_ptr<CommandQueue> commandQueue;
static DescriptorHeaps* heaps;

static std::string OutputText = "";

///////////////////////////

const ImVec2 wSize = ImVec2(500.0f, 375.0f);

///////////////////////////

enum ImGuiControllerCommandExecute
{
	COMMAND_EXECUTE_RELOAD_SCENE,
	COMMAND_EXECUTE_SAVE_SCENE,
	COMMAND_EXECUTE_DELETE_SELECTED,
	COMMAND_EXECUTE_ADD_NODE,
	COMMAND_EXECUTE_RECREATE_3D_OBJECT,
	COMMAND_EXECUTE_LOAD_TEXTURE
};
static std::vector<ImGuiControllerCommandExecute> Commands;

enum GUIManagerMode
{
	ManagerModeFileSystem,
	ManagerModeOutput
};
static GUIManagerMode managerMode = ManagerModeOutput;

static NodeTypeEnum objectTypeToCreate = NODE_TYPE_NODE3D;

static std::string filePath = "";
static MaterialEntity* materialToEdit = nullptr;
static TextureType textureToEdit = TextureType::NONE;

static Node3D* selectedNode = nullptr;
static char selectedNameEdit[256];

static std::string GetMultilineText(const std::string& text, int lineLength)
{
	if (lineLength <= 0) return text;

	std::string out;
	out.reserve(text.size() + text.size() / lineLength);

	int count = 0;
	for (char c : text)
	{
		out.push_back(c);
		count++;

		if (count >= lineLength && c != '\n')
		{
			out.push_back('\n');
			count = 0;
		}

		if (c == '\n')
			count = 0;
	}

	return out;
}

void ImGuiController::Create(std::shared_ptr<Window> window)
{
	if (isInitialized) return;

	MainWindow = window;

	device = Application::Get().GetPrimaryDevice();
	commandQueue = Application::Get().GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	heaps = DescriptorHeaps::GetHeaps(GraphicAdapterPrimary);
	if (heaps == nullptr || heaps->GetCBVHeap(GraphicAdapterPrimary) == nullptr) throw;

	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window->GetWindowHandle());

	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = device.Get();
	init_info.CommandQueue = commandQueue->GetD3D12CommandQueue().Get();
	init_info.NumFramesInFlight = APP_NUM_FRAMES_IN_FLIGHT;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
	// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
	// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
	ID3D12DescriptorHeap* srvHeap = DescriptorHeaps::GetCBVHeap(GraphicAdapterPrimary).Get();
	init_info.SrvDescriptorHeap = srvHeap;
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return heaps->CbvHeapAllocator.Alloc(out_cpu_handle, out_gpu_handle); };
	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return heaps->CbvHeapAllocator.Free(cpu_handle, gpu_handle); };
	ImGui_ImplDX12_Init(&init_info);

	if (fb.root.empty())
	{
		fb.root = "../../DX12GE/Resources";
		fb.current = fb.root;
	}

	isInitialized = true;
}

void ImGuiController::OnRenderStart()
{
	if (!isInitialized) return;

	if (!DrawUI || Started) return;

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	for (auto command : Commands)
	{
		switch (command)
		{
		case COMMAND_EXECUTE_RELOAD_SCENE:
		{
			auto commandList = commandQueue->GetCommandList();

			Singleton::GetNodeGraph()->Reset(true);
			Singleton::GetSerializer()->Load(commandList);

			uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
			commandQueue->WaitForFenceValue(fenceValue);

			OutputText += "Scene has loaded from file\n";
		}
		break;
		case COMMAND_EXECUTE_SAVE_SCENE:
		{
			Singleton::GetSerializer()->Save();
			OutputText += "Scene has saved to file\n";
		}
		break;
		case COMMAND_EXECUTE_DELETE_SELECTED:
		{
			auto objects = Singleton::GetSelection()->GetSelected();
			Singleton::GetSelection()->DeselectAll();
			for (int i = 0; i < objects.size(); i++)
			{
				if (objects[i] && objects[i] != Singleton::GetNodeGraph()->GetRoot())
				{
					auto path = objects[i]->GetNodePath();
					if (Singleton::GetNodeGraph()->RemoveNodeFromScene(path, true))
					{
						OutputText += "Node " + path + " has removed\n";
					}
					else
					{
						OutputText += "Error: Node has not removed\n";
					}
				}					
			}
		}
		break;
		case COMMAND_EXECUTE_ADD_NODE:
		{
			Node3D* parent = nullptr;

			size_t selectedCount = Singleton::GetSelection()->SelectedCount();

			if (selectedCount == 0)
			{
				parent = Singleton::GetNodeGraph()->GetRoot();
			}
			else if (selectedCount == 1)
			{
				auto objects = Singleton::GetSelection()->GetSelected();
				parent = objects[0];
			}
			else break;

			Singleton::GetSelection()->DeselectAll();
			
			Node3D* newNode = Singleton::GetNodeGraph()->CreateNewNodeInScene(parent->GetNodePath() + "/" + "newNode", objectTypeToCreate);

			if (newNode)
			{
				OutputText += "Node " + newNode->GetNodePath() + " with type " + GetTypeName(objectTypeToCreate) + " has created\n";
			}
			else
			{
				OutputText += "Error: Node has not created\n";
			}

			objectTypeToCreate = NODE_TYPE_NODE3D;
		}
		break;

		case COMMAND_EXECUTE_RECREATE_3D_OBJECT:
		{

			if (Object3DNode* obj = dynamic_cast<Object3DNode*>(selectedNode))
			{
				auto commandList = commandQueue->GetCommandList();

				if (obj->Create(commandList, filePath))
				{
					OutputText += "3D object data has loaded\n";
				}
				else
				{
					OutputText += "Error: 3D object data has not loaded\n";
				}
				uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
				commandQueue->WaitForFenceValue(fenceValue);
			}
			else if (AudioEmitterNode* emitter = dynamic_cast<AudioEmitterNode*>(selectedNode))
			{
				emitter->DestroyPlayingSound();

				try
				{
					emitter->LoadWav(filePath);
					emitter->SpawnPlayingSound(false);
					OutputText += "Audio data has loaded\n";
				}
				catch(const std::runtime_error& e)
				{
					OutputText += "Error: Audio data has not loaded (";
					OutputText += e.what();
					OutputText += ")\n";
				}				
			}

			filePath = "";
		}
		break;

		case COMMAND_EXECUTE_LOAD_TEXTURE:
		{
			Object3DNode* obj = dynamic_cast<Object3DNode*>(selectedNode);
			if (!obj) break;

			uint32_t newID = -1;

			if (filePath != "")
			{
				newID = ResourceStorage::AddTexture(filePath);
				auto textureComponent = ResourceStorage::GetTexture(newID);

				if (!textureComponent->IsInitialized())
				{
					auto commandList = commandQueue->GetCommandList();
					textureComponent->OnLoad(commandList, filePath);
					uint64_t fenceValue = commandQueue->ExecuteCommandList(commandList);
					commandQueue->WaitForFenceValue(fenceValue);
				}

				if (textureComponent->IsInitialized())
				{
					OutputText += "Texture successfully replaced\n";
				}
				else
				{
					OutputText += "Error: Texture has not replaced! \n";
					textureToEdit = TextureType::NONE;
					filePath = "";
					break;
				}
			}

			if (materialToEdit->GetType() == MaterialEntity::ORIGINAL)
			{
				materialToEdit = materialToEdit->Duplicate();
				obj->MaterialsOverride[obj->SelectedMaterial] = materialToEdit;
			}

			switch (textureToEdit)
			{
			case TextureType::DIFFUSE:
				materialToEdit->m_DiffuseTextureId = newID;
				materialToEdit->m_HasDiffuseNormalEmissive.x = (newID != -1) ? 1.0f : 0.0f;
				break;
			case TextureType::NORMALS:
				materialToEdit->m_NormalTextureId = newID;
				materialToEdit->m_HasDiffuseNormalEmissive.y = (newID != -1) ? 1.0f : 0.0f;
				break;
			case TextureType::EMISSIVE:
				materialToEdit->m_EmissiveTextureId = newID;
				materialToEdit->m_HasDiffuseNormalEmissive.z = (newID != -1) ? 1.0f : 0.0f;
				break;
			case TextureType::METALNESS:
				materialToEdit->m_MetallicTextureId = newID;
				materialToEdit->m_HasOcclusionRoughnessMetallicCombined.z = (newID != -1) ? 1.0f : 0.0f;
				break;
			case TextureType::DIFFUSE_ROUGHNESS:
				materialToEdit->m_RoughnessTextureId = newID;
				materialToEdit->m_HasOcclusionRoughnessMetallicCombined.y = (newID != -1) ? 1.0f : 0.0f;
				break;
			case TextureType::GLTF_METALLIC_ROUGHNESS:
				materialToEdit->m_GltfMetallicRoughnessTextureId = newID;
				materialToEdit->m_HasOcclusionRoughnessMetallicCombined.w = (newID != -1) ? 1.0f : 0.0f;
				break;
			case TextureType::AMBIENT_OCCLUSION:
				materialToEdit->m_AOTextureId = newID;
				materialToEdit->m_HasOcclusionRoughnessMetallicCombined.x = (newID != -1) ? 1.0f : 0.0f;
				break;
			}

			textureToEdit = TextureType::NONE;
			filePath = "";
		}
		break;

		default:
			OutputText += "Unknown command\n";
		break;
		}
	}
	Commands.clear();

	Started = true;
}

void ImGuiController::OnRenderEnd(double deltaTime, ComPtr<ID3D12GraphicsCommandList2> commandList)
{
	if (!isInitialized) return;

	if (!Started) return;
	Started = false;

	bool show_demo_window = false;

	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImVec2 windowSize = vp->WorkSize;

	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(wSize.x, windowSize.y), ImGuiCond_Always);

		ImGui::Begin("Scene Tree", (bool*)0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					Commands.push_back(COMMAND_EXECUTE_RELOAD_SCENE);
				}

				if (ImGui::MenuItem("Save")) 
				{
					Commands.push_back(COMMAND_EXECUTE_SAVE_SCENE);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Node"))
			{
				if (ImGui::BeginMenu("Add"))
				{
					for (int i = 0; i <= NODE_TYPE_AUDIO_EMITTER; i++)
					{
						NodeTypeEnum type = NodeTypeEnum(i);
						if (ImGui::MenuItem(GetTypeName(type)))
						{
							objectTypeToCreate = type;
							Commands.push_back(COMMAND_EXECUTE_ADD_NODE);
						}
					}

					ImGui::EndMenu();
				}


				if (ImGui::MenuItem("Delete", "", false, Singleton::GetSelection()->SelectedCount() > 0))
				{
					Commands.push_back(COMMAND_EXECUTE_DELETE_SELECTED);
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		UpdateSceneTree(Singleton::GetNodeGraph()->GetRoot());
		ImGui::End();
	}

	{
		ImGui::SetNextWindowPos(ImVec2(windowSize.x - wSize.x, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(wSize.x, windowSize.y / 2.0f), ImGuiCond_Always);

		ImGui::Begin("Inspector", (bool*)0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		auto selectedArray = Singleton::GetSelection()->GetSelected(); 

		if (selectedArray.size() == 1)
		{
			if (!selectedNode || selectedArray[0] != selectedNode)
			{
				selectedNode = selectedArray[0];

				memset(selectedNameEdit, 0, sizeof(selectedNameEdit));
				size_t nameLength = selectedNode->GetName().length();
				strcpy_s(selectedNameEdit, nameLength + 1, selectedNode->GetName().c_str());
			}

			ImGui::Text(GetTypeName(selectedNode->GetType()));
			ImGui::Text("Id: ");
			ImGui::SameLine();
			ImGui::Text((to_string(static_cast<int>(selectedNode->GetNodeId()))).c_str());

			ImGui::InputText("Name", selectedNameEdit, IM_COUNTOF(selectedNameEdit));
			ImGui::SameLine();
			if (ImGui::Button("Confirm"))
			{
				selectedNode->Rename(selectedNameEdit);
			}

			UpdateInspector(selectedNode);
		}
		else
		{
			selectedNode = nullptr;
		}

		ImGui::End();
	}

	{
		ImGui::SetNextWindowPos(ImVec2(windowSize.x - wSize.x, windowSize.y / 2.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(wSize.x, windowSize.y / 2.0f), ImGuiCond_Always);

		ImGui::Begin("Material Editor", (bool*)0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		UpdateMaterialInspector(selectedNode);

		ImGui::End();
	}

	{
		const char* title = "";

		switch (managerMode)
		{
		case(ManagerModeFileSystem):
			title = "File System";
			break;
		case(ManagerModeOutput):
			title = "Output";
			break;
		}

		ImGui::SetNextWindowPos(ImVec2(wSize.x, windowSize.y - wSize.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(windowSize.x - 2 * wSize.x, wSize.y), ImGuiCond_Always);

		ImGui::Begin(title, (bool*)0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Manager Mode"))
			{
				if (ImGui::MenuItem("File System")) { managerMode = ManagerModeFileSystem; }
				if (ImGui::MenuItem("Output")) { managerMode = ManagerModeOutput; }
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		
		switch (managerMode)
		{
		case(ManagerModeFileSystem):
			if (ImGui::Button("Up"))
			{
				std::error_code ec;
				if (fb.current.has_parent_path())
					fb.current = fb.current.parent_path();
			}
			ImGui::SameLine();
			ImGui::Checkbox("Show hidden", &fb.showHidden);

			ImGui::Separator();
			DrawDirContents();
			break;
		case(ManagerModeOutput):
			ImGui::Text(OutputText.c_str());
			break;
		}

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}


bool ImGuiController::CursorOnWindow()
{
	if (!isInitialized) return false;
	return ImGui::GetIO().WantCaptureMouse;
}

void ImGuiController::ShutDown()
{
	if (!isInitialized) return;

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	isInitialized = false;
}

void ImGuiController::ChangeVisiblity()
{
	DrawUI = !DrawUI;
}

bool ImGuiController::AddVector3Edit(const char* label, Vector3& value)
{
	float vec[3] = { value.x, value.y, value.z };

	if (ImGui::InputFloat3(label, vec))
	{
		value.x = vec[0];
		value.y = vec[1];
		value.z = vec[2];
		return true;
	}

	return false;
}

bool ImGuiController::AddColor3Edit(const char* label, Vector3& value)
{
	float vec[3] = { value.x, value.y, value.z };

	if (ImGui::ColorEdit3(label, vec))
	{
		value.x = vec[0];
		value.y = vec[1];
		value.z = vec[2];
		return true;
	}
	return false;
}

void ImGuiController::UpdateSceneTree(Node3D* node)
{
	if (!node) return;

	auto children = node->GetChildren();

	const bool hasChildren = children.size() > 0;

	ImGuiTreeNodeFlags flags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_DefaultOpen;

	if (!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	if (Singleton::GetSelection()->IsSelected(node))
		flags |= ImGuiTreeNodeFlags_Selected;

	// ВАЖНО: давать стабильный ID (лучше pointer/handle), иначе при одинаковых именах будет конфликт
	ImGui::PushID(node);

	const bool open = ImGui::TreeNodeEx(node->GetName().c_str(), flags);

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		Node3D* payloadNode = node;
		ImGui::SetDragDropPayload("kSceneNodePayload", &payloadNode, sizeof(payloadNode));
		ImGui::Text("%s", node->GetName().c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("kSceneNodePayload"))
		{
			Node3D* dragged = *(Node3D**)payload->Data;
			if (dragged && dragged != node)
			{
				if (ImGui::GetIO().KeyAlt)
				{
					auto clone = Singleton::GetNodeGraph()->CloneNode(dragged->GetNodePath(), node->GetNodePath());
					if (clone)
					{
						OutputText += "Node has cloned\n";
					}
					else
					{
						OutputText += "Error: Node has not cloned\n";
					}
				}
				else
				{
					if (Singleton::GetNodeGraph()->MoveNode(dragged->GetNodePath(), node->GetNodePath()))
					{
						OutputText += "Node has moved\n";
					}
					else
					{
						OutputText += "Error: Node has not moved\n";
					}
				}

				dragged->Transform.Move(0, 0, 0);
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Клик по строке выбирает узел
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		Singleton::GetSelection()->DeselectAll();
		Singleton::GetSelection()->SelectNode(node);
	}

	if (hasChildren && open)
	{
		for (Node3D* child : children)
			UpdateSceneTree(child);

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void ImGuiController::UpdateInspector(Node3D* node)
{
	if (!node || node == Singleton::GetNodeGraph()->GetRoot()) return;

	ImGui::SeparatorText("Transform");

	Vector3 pos = node->Transform.GetPosition();
	if (AddVector3Edit("Position", pos))
	{
		node->Transform.SetPosition(pos);
	}

	Vector3 rot = node->Transform.GetRotationDegrees();
	if (AddVector3Edit("Rotation", rot))
	{
		node->Transform.SetRotationDegrees(rot);
	}

	Vector3 scl = node->Transform.GetScale();
	if (AddVector3Edit("Scale", scl))
	{
		node->Transform.SetScale(scl);
	}

	// ZAKHAR - SCRIPTS
	std::vector<std::string> scripts{ node->GetNodeScripts() };
	ImGui::SeparatorText("Lua Scripts");
	ImGui::Text("Current scripts list:");
	int counter = 1;
	for (const auto& item : scripts)
	{
		std::string format = std::format("{}. {}", counter, item);
		ImGui::Text(format.c_str());
		counter++;
	}

	if (ImGui::Button("Add script to node"))
	{
		isShowingScriptsToAdd = !isShowingScriptsToAdd;
	}

	if (isShowingScriptsToAdd)
	{
		ImGui::SeparatorText("Founded Scripts");
		for (std::string& item : LuaManager::GetAllFoundScriptClasses())
		{
			bool clear = true;
			for (const std::string& sc : scripts)
			{
				if (sc + ".lua" == item)
				{
					clear = false;
					break;
				}
			}
			if (clear)
			{
				if (ImGui::Button(item.c_str()))
				{
					size_t lastDot = item.find_last_of('.');
					if (lastDot != std::string::npos && lastDot > 0) 
					{
						node->AddScript(item.substr(0, lastDot));
					} 
					else
					{
						node->AddScript(item);
					}
				}
			}
		}
	}

	if (scripts.size() > 0)
	{
		if (ImGui::Button("Remove script from node"))
		{
			isShowingScriptsToRemove = !isShowingScriptsToRemove;
		}

		if (isShowingScriptsToRemove)
		{
			for (auto& item : scripts)
			{
				if (ImGui::Button(item.c_str()))
				{
					node->RemoveScript(item);
				}
			}
		}
	}

	if (ImGui::Button("Reload all scripts"))
	{

	}

	if (AudioEmitterNode* emitter = dynamic_cast<AudioEmitterNode*>(node))
	{
		ImGui::SeparatorText("AudioEmitter");

		ImGui::Text("Resource File: ");
		ImGui::SameLine();
		ImGui::Text(GetMultilineText(emitter->GetWavFilePath(), 25).c_str());
		ImGui::Button("Drop new Wav file here");

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
			{
				const char* pathCStr = (const char*)payload->Data;
				std::string droppedPath(pathCStr);
				std::replace(droppedPath.begin(), droppedPath.end(), '\\', '/');

				filePath = droppedPath;
				Commands.push_back(COMMAND_EXECUTE_RECREATE_3D_OBJECT);
			}
			ImGui::EndDragDropTarget();
		}

		if (emitter->IsPlaying())
		{
			if (ImGui::Button("Press to Stop Playing"))
			{
				emitter->DestroyPlayingSound();
			}
		}
		else
		{
			if (ImGui::Button("Press to Start Playing"))
			{
				emitter->SpawnPlayingSound(true);
			}
		}

		ImGui::Checkbox("Loop", &emitter->Loop);
		ImGui::Checkbox("DopplerEffect", &emitter->DopplerEffect);
		ImGui::Checkbox("Ubiquitous", &emitter->Ubiquitous);

		float volume = emitter->GetVolume();
		if (ImGui::SliderFloat("Volume", &volume, 0.0f, 32.0f))
		{
			emitter->SetVolume(volume);
		}

		float pitch = emitter->GetPitch();
		if (ImGui::SliderFloat("Pitch", &pitch, 0.0f, 32.0f))
		{
			emitter->SetPitch(pitch);
		}
	}
	else if (AudioListenerNode* listener = dynamic_cast<AudioListenerNode*>(node))
	{
		ImGui::SeparatorText("AudioListener");

		if (listener->IsCurrent())
		{
			ImGui::Text("AudioListener is only active one now");
		}
		else
		{
			if (ImGui::Button("Set Current"))
			{
				listener->SetCurrent();
			}
		}
	}
	else if (CameraNode* camera = dynamic_cast<CameraNode*>(node))
	{
		ImGui::SeparatorText("Camera");

		if (camera->IsCurrent())
		{
			ImGui::Text("Camera is only active one now");
		}
		else
		{
			if (ImGui::Button("Set Current"))
			{
				camera->SetCurrent();
			}
		}

		ImGui::SliderFloat("FOV", &camera->Fov, 1.0f, 180.0f);

		float znear = camera->ZNear;
		if (ImGui::InputFloat("ZNear", &znear))
		{
			if (znear > 0.0f && znear <= camera->ZFar)
			{
				camera->ZNear = znear;
			}
		}

		float zfar = camera->ZFar;
		if (ImGui::InputFloat("ZFar", &zfar))
		{
			if (zfar >= camera->ZNear)
			{
				camera->ZFar = zfar;
			}
		}
	}
	else if (DirectionalLightNode* dLight = dynamic_cast<DirectionalLightNode*>(node))
	{
		ImGui::SeparatorText("DirectionalLight");

		if (dLight->IsCurrent())
		{
			ImGui::Text("DirLight is only active one now");
		}
		else
		{
			if (ImGui::Button("Set Current"))
			{
				dLight->SetCurrent();
			}
		}

		Vector3 color = dLight->LightData.BaseLightProperties.Color;
		if (AddColor3Edit("Color", color))
		{
			dLight->LightData.BaseLightProperties.Color = color;
		}

		float intensity = dLight->LightData.BaseLightProperties.Intensity;
		if (ImGui::InputFloat("Intensity", &intensity))
		{
			if (intensity >= 0.0f)
				dLight->LightData.BaseLightProperties.Intensity = intensity;
		}
	}
	else if (EnvironmentNode* env = dynamic_cast<EnvironmentNode*>(node))
	{
		ImGui::SeparatorText("Environment");

		if (env->IsCurrent())
		{
			ImGui::Text("Environment is only active one now");
		}
		else
		{
			if (ImGui::Button("Set Current"))
			{
				env->SetCurrent();
			}
		}

		Vector3 color = env->AmbientLightData.Color;
		if (AddColor3Edit("Ambient Light Color", color))
		{
			env->AmbientLightData.Color = color;
		}

		float intensity = env->AmbientLightData.Intensity;
		if (ImGui::InputFloat("Ambient Light Intensity", &intensity))
		{
			if (intensity >= 0.0f)
				env->AmbientLightData.Intensity = intensity;
		}

		ImGui::Checkbox("Fog Enabled", &env->FogEnabled);
		if (env->FogEnabled)
		{
			Vector3 fogColor = env->FogColor;
			if (AddColor3Edit("Fog Color", fogColor))
			{
				env->FogColor = fogColor;
			}

			float fogStart = env->FogStart;
			if (ImGui::InputFloat("Fog Start", &fogStart))
			{
				if (fogStart >= 0.0f)
					env->FogStart = fogStart;
			}

			float fogDist = env->FogDistance;
			if (ImGui::InputFloat("Fog Distance", &fogDist))
			{
				if (fogDist > 0.0f)
					env->FogDistance = fogDist;
			}
		}

		float ssrDist = env->SSRMaxDistance;
		if (ImGui::InputFloat("SSR Max Distance", &ssrDist))
		{
			if (ssrDist >= 0.0f)
				env->SSRMaxDistance = ssrDist;
		}

		float ssrStep = env->SSRStepLength;
		if (ImGui::InputFloat("SSR Step Length", &ssrStep))
		{
			if (ssrStep > 0.0f)
				env->SSRStepLength = ssrStep;
		}

		float ssrTh = env->SSRThickness;
		if (ImGui::InputFloat("SSR Thickness", &ssrTh))
		{
			if (ssrTh > 0.0f)
				env->SSRThickness = ssrTh;
		}
	}
	else if (PointLightNode* pLight = dynamic_cast<PointLightNode*>(node))
	{
		ImGui::SeparatorText("PointLight");

		Vector3 color = pLight->LightData.BaseLightProperties.Color;
		if (AddColor3Edit("Color", color))
		{
			pLight->LightData.BaseLightProperties.Color = color;
		}

		float intensity = pLight->LightData.BaseLightProperties.Intensity;
		if (ImGui::InputFloat("Intensity", &intensity))
		{
			if (intensity >= 0.0f)
				pLight->LightData.BaseLightProperties.Intensity = intensity;
		}

		float attenConst = pLight->LightData.AttenuationProperties.Constant;
		if (ImGui::InputFloat("Attenuation Const", &attenConst))
		{
			if (attenConst >= 0.0f)
				pLight->LightData.AttenuationProperties.Constant = attenConst;
		}

		float attenLin = pLight->LightData.AttenuationProperties.Linear;
		if (ImGui::InputFloat("Attenuation Linear", &attenLin))
		{
			if (attenLin >= 0.0f)
				pLight->LightData.AttenuationProperties.Linear = attenLin;
		}

		float attenExp = pLight->LightData.AttenuationProperties.Exp;
		if (ImGui::InputFloat("Attenuation Exp", &attenExp))
		{
			if (attenExp >= 0.0f)
				pLight->LightData.AttenuationProperties.Exp = attenExp;
		}
	}
	else if (SpotLightNode* sLight = dynamic_cast<SpotLightNode*>(node))
	{
		ImGui::SeparatorText("SpotLight");

		Vector3 color = sLight->LightData.PointLightProperties.BaseLightProperties.Color;
		if (AddColor3Edit("Color", color))
		{
			sLight->LightData.PointLightProperties.BaseLightProperties.Color = color;
		}

		float intensity = sLight->LightData.PointLightProperties.BaseLightProperties.Intensity;
		if (ImGui::InputFloat("Intensity", &intensity))
		{
			if (intensity >= 0.0f)
				sLight->LightData.PointLightProperties.BaseLightProperties.Intensity = intensity;
		}

		float attenConst = sLight->LightData.PointLightProperties.AttenuationProperties.Constant;
		if (ImGui::InputFloat("Attenuation Const", &attenConst))
		{
			if (attenConst >= 0.0f)
				pLight->LightData.AttenuationProperties.Constant = attenConst;
		}

		float attenLin = sLight->LightData.PointLightProperties.AttenuationProperties.Linear;
		if (ImGui::InputFloat("Attenuation Linear", &attenLin))
		{
			if (attenLin >= 0.0f)
				sLight->LightData.PointLightProperties.AttenuationProperties.Linear = attenLin;
		}

		float attenExp = sLight->LightData.PointLightProperties.AttenuationProperties.Exp;
		if (ImGui::InputFloat("Attenuation Exp", &attenExp))
		{
			if (attenExp >= 0.0f)
				sLight->LightData.PointLightProperties.AttenuationProperties.Exp = attenExp;
		}

		float cutoff = sLight->LightData.Cutoff;
		if (ImGui::InputFloat("Attenuation Cutoff", &cutoff))
		{
			if (cutoff >= 0.0f)
				sLight->LightData.Cutoff = cutoff;
		}
	}
	else if (Object3DNode* obj = dynamic_cast<Object3DNode*>(node))
	{
		if (SkyBoxNode* sky = dynamic_cast<SkyBoxNode*>(node))
		{
			ImGui::SeparatorText("SkyBox");
		}
		else
		{
			ImGui::SeparatorText("Object3D");
		}

		ImGui::Checkbox("IsVisible", &obj->IsVisible);

		ImGui::Text("Resource File: ");
		ImGui::SameLine();
		ImGui::Text(GetMultilineText(obj->GetObjectFilePath(), 25).c_str());
		ImGui::Button("Drop new Resource file here");

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
			{
				const char* pathCStr = (const char*)payload->Data;
				std::string droppedPath(pathCStr);
				std::replace(droppedPath.begin(), droppedPath.end(), '\\', '/');

				filePath = droppedPath;
				Commands.push_back(COMMAND_EXECUTE_RECREATE_3D_OBJECT);
			}
			ImGui::EndDragDropTarget();
		}

		if (PhysicalObjectNode* phys = dynamic_cast<PhysicalObjectNode*>(node))
		{
			ImGui::SeparatorText("PhysicalObject");
			// RAUF - PHYSICS

			if (FirstPersonPlayerNode* fPlayer = dynamic_cast<FirstPersonPlayerNode*>(node))
			{
				ImGui::SeparatorText("FirstPersonPlayer");

				if (fPlayer->IsCurrent())
				{
					ImGui::Text("Player is only active one now");
				}
				else
				{
					if (ImGui::Button("Set Current"))
					{
						fPlayer->SetCurrent();
					}
				}

				float sens = fPlayer->MouseSensitivity;
				if (ImGui::InputFloat("Mouse Sensitivity", &sens))
				{
					if (sens >= 0.0f)
						fPlayer->MouseSensitivity = sens;
				}

				float wsens = fPlayer->WheelSensitivity;
				if (ImGui::InputFloat("Wheel Sensitivity", &wsens))
				{
					if (wsens >= 0.0f)
						fPlayer->WheelSensitivity = wsens;
				}

				float minsp = fPlayer->MinMovementSpeed;
				if (ImGui::InputFloat("Min Speed", &minsp))
				{
					if (minsp >= 0.0f)
						fPlayer->MinMovementSpeed = minsp;
				}

				float normsp = fPlayer->NormalMovementSpeed;
				if (ImGui::InputFloat("Normal Speed", &normsp))
				{
					if (normsp >= 0.0f)
						fPlayer->NormalMovementSpeed = normsp;
				}

				float maxsp = fPlayer->MaxMovementSpeed;
				if (ImGui::InputFloat("Max Speed", &maxsp))
				{
					if (maxsp >= 0.0f)
						fPlayer->MaxMovementSpeed = maxsp;
				}

				float jumpImp = fPlayer->JumpImpulse;
				if (ImGui::InputFloat("Jump Impulse", &jumpImp))
				{
					if (jumpImp >= 0.0f)
						fPlayer->JumpImpulse = jumpImp;
				}

				if (ThirdPersonPlayerNode* tPlayer = dynamic_cast<ThirdPersonPlayerNode*>(node))
				{
					ImGui::SeparatorText("ThirdPersonPlayer");

					float MinFlyRadius = tPlayer->MinFlyRadius;
					if (ImGui::InputFloat("Min Fly Radius", &MinFlyRadius))
					{
						if (MinFlyRadius >= 0.0f)
							tPlayer->MinFlyRadius = MinFlyRadius;
					}

					float MaxFlyRadius = tPlayer->MaxFlyRadius;
					if (ImGui::InputFloat("Max Fly Radius", &MaxFlyRadius))
					{
						if (MaxFlyRadius >= 0.0f)
							tPlayer->MaxFlyRadius = MaxFlyRadius;
					}

					Vector3 CameraAnchor = tPlayer->CameraAnchor;
					if (AddVector3Edit("Camera Anchor", CameraAnchor))
					{
						tPlayer->CameraAnchor = CameraAnchor;
					}
				}
			}
		}
	}
	// GLEB - AI
	//else if (AINode* aiNode = dynamic_cast<AINode*>(node))
	//{
	//	
	//}
}

static void AppendStr(char*& dst, const char* add)
{
	if (!add) return;

	size_t oldLen = dst ? strlen(dst) : 0;
	size_t addLen = strlen(add);

	char* newBuf = new char[oldLen + addLen + 1];

	if (dst)
		memcpy(newBuf, dst, oldLen);

	memcpy(newBuf + oldLen, add, addLen);
	newBuf[oldLen + addLen] = '\0';

	delete[] dst;
	dst = newBuf;
}

void ImGuiController::UpdateTexture(const std::string& label, uint32_t id, TextureType type)
{
	std::shared_ptr<TextureComponent> texture = nullptr;

	ImGui::SeparatorText(label.c_str());

	ImGui::Text("Path: ");
	ImGui::SameLine();

	if (id != -1)
	{
		texture = ResourceStorage::GetTexture(id);

		if (!texture)
		{
			ImGui::Text("-");
		}
		else
		{
			ImGui::Text(GetMultilineText(texture->GetResourcePath(), 35).c_str());
		}		
	}
	else
	{
		ImGui::Text("-");
	}

	if (ImGui::Button(("Remove " + label).c_str()))
	{
		textureToEdit = type;
		filePath = "";
		Commands.push_back(COMMAND_EXECUTE_LOAD_TEXTURE);
	}

	ImGui::Button(("Drop new " + label + " file here").c_str());

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
		{
			const char* pathCStr = (const char*)payload->Data;
			std::string droppedPath(pathCStr);
			std::replace(droppedPath.begin(), droppedPath.end(), '\\', '/');

			textureToEdit = type;
			filePath = droppedPath;
			Commands.push_back(COMMAND_EXECUTE_LOAD_TEXTURE);
		}
		ImGui::EndDragDropTarget();
	}
}

void ImGuiController::UpdateMaterialInspector(Node3D* node)
{
	if (!node) return;

	Object3DNode* obj = dynamic_cast<Object3DNode*>(node);
	if (!obj) return;

	if (node->GetType() == NODE_TYPE_SKYBOX) return;

	std::shared_ptr<Object3DComponent> component = ResourceStorage::GetObject3D(obj->GetComponentId());
	if (!component) return;

	size_t meshCount = component->GetMeshCount();
	if (meshCount == 0) return;

	std::vector<const char*> citems;
	citems.reserve(meshCount);

	for (size_t i = 0; i < meshCount; i++)
	{
		citems.push_back(component->GetMesh(i)->Material->Name.c_str());
	}

	ImGui::Combo("Current Material", &obj->SelectedMaterial, citems.data(), (int)citems.size());

	MaterialEntity* overrideMaterial = obj->MaterialsOverride[obj->SelectedMaterial];
	materialToEdit = overrideMaterial ? overrideMaterial : component->GetMesh(obj->SelectedMaterial)->Material;

	ImGui::Text("Material Type: ");
	ImGui::SameLine();
	ImGui::Text(materialToEdit->GetType() == MaterialEntity::ORIGINAL ? "Original" : "Copy");
	
	UpdateTexture("Diffuse", materialToEdit->m_DiffuseTextureId, TextureType::DIFFUSE);

	Vector3 color = Vector3(materialToEdit->m_AlbedoColor);
	if (AddColor3Edit("AlbedoColor", color))
	{
		if (materialToEdit->GetType() == MaterialEntity::ORIGINAL)
		{
			materialToEdit = materialToEdit->Duplicate();
			obj->MaterialsOverride[obj->SelectedMaterial] = materialToEdit;
		}

		materialToEdit->m_AlbedoColor = Vector4(color.x, color.y, color.z, 1.0f);
	}

	UpdateTexture("Normal", materialToEdit->m_NormalTextureId, TextureType::NORMALS);
	UpdateTexture("Emissive", materialToEdit->m_EmissiveTextureId, TextureType::EMISSIVE);
	UpdateTexture("Metallic", materialToEdit->m_MetallicTextureId, TextureType::METALNESS);
	UpdateTexture("Roughness", materialToEdit->m_RoughnessTextureId, TextureType::DIFFUSE_ROUGHNESS);
	UpdateTexture("GltfMetallicRoughness", materialToEdit->m_GltfMetallicRoughnessTextureId, TextureType::GLTF_METALLIC_ROUGHNESS);
	UpdateTexture("AmbientOcclusion", materialToEdit->m_AOTextureId, TextureType::AMBIENT_OCCLUSION);
}

void ImGuiController::DrawDirContents()
{
	std::error_code ec;
	if (!fs::exists(fb.current, ec) || !fs::is_directory(fb.current, ec))
	{
		ImGui::TextDisabled("Invalid directory");
		return;
	}

	std::vector<fs::directory_entry> entries;
	for (auto it = fs::directory_iterator(fb.current, fs::directory_options::skip_permission_denied, ec);
		!ec && it != fs::directory_iterator(); ++it)
	{
		if (!ShouldSkip(*it, fb.showHidden))
			entries.push_back(*it);
	}

	std::sort(entries.begin(), entries.end(),
		[](const fs::directory_entry& a, const fs::directory_entry& b)
		{
			const bool ad = a.is_directory();
			const bool bd = b.is_directory();
			if (ad != bd) return ad > bd;
			return a.path().filename().string() < b.path().filename().string();
		});

	ImGui::Text("Path: %s", fb.current.string().c_str());
	ImGui::Separator();

	if (ImGui::BeginTable("files", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY, ImVec2(0, 0)))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableHeadersRow();

		for (auto& e : entries)
		{
			const fs::path p = e.path();
			const std::string ext = p.extension().string();
			const bool isDir = e.is_directory(ec);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			std::string name = p.filename().string();
			if (name.empty()) name = p.string();

			ImGui::PushID(p.string().c_str());

			if (ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns))
			{
				
			}

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (isDir)
				{
					fb.current = p;
				}
				else if (ext == ".json")
				{
					Singleton::GetSerializer()->path = p.string();
					Commands.push_back(COMMAND_EXECUTE_RELOAD_SCENE);
				}
			}

			if (ImGui::BeginDragDropSource())
			{
				std::string s = p.string();
				ImGui::SetDragDropPayload("FILE_PATH", s.c_str(), s.size() + 1);
				ImGui::Text("%s", name.c_str());
				ImGui::EndDragDropSource();
			}

			ImGui::PopID();

			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(isDir ? "DIR" : "FILE");

			ImGui::TableSetColumnIndex(2);
			if (!isDir)
			{
				auto sz = e.file_size(ec);
				if (!ec) ImGui::Text("%llu", (unsigned long long)sz);
				else ImGui::TextUnformatted("-");
			}
			else
			{
				ImGui::TextUnformatted("-");
			}
		}

		ImGui::EndTable();
	}
}