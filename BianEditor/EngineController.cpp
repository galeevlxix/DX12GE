#include "EngineController.h"

#include <EngineConfig.h>
#include <Engine/Base/Application.h>
#include <Engine/SingleGpuGame.h>
#include <Engine/Base/LuaManager.h>

#include <dxgidebug.h>

static std::shared_ptr<Game> game;

int EngineController::Run(DirectX12Viewport* viewport)
{
    if (!viewport)
    {
        throw std::runtime_error("viewport is not initialized");
    }

    EngineConfig::Mode = EngineConfigRuntimeMode::RUNTIME_MODE_EDITING;
    EngineConfig::Output = EngineConfigRuntimeOutput::RUNTIME_OUTPUT_EDITOR;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    Application::Create(hInstance);

    Application::Get().CreateRenderViewport(viewport->GetHWND(), viewport->GetName(), viewport->GetWidth(), viewport->GetHeight(), false);
    viewport->SetEngineWindow(Application::Get().GetWindowByName(viewport->GetName()));

    game = std::make_shared<SingleGpuGame>(viewport->GetName(), viewport->GetWidth(), viewport->GetHeight(), false);
    
    if (!game->Initialize()) return 1;
    if (!game->LoadContent()) return 2;
    LuaManager::Start();

    return 0;
}

static void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

void EngineController::Quit()
{
    Application::Get().Quit(0);
    Application::Get().Flush();
    if (game)
    {
        game->UnloadContent();
        game->Destroy();
        game.reset();
    }   
    Application::Destroy();

    ReportLiveObjects();
}