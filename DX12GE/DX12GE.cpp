#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>

#include "Engine/Base/Application.h"
#include "Game/GameSample.h"
#include "Engine/Lua/LuaManager.h"
#include "EngineConfig.h"
#include <dxgidebug.h>

void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
   
    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

int CALLBACK main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    int retCode = 0;
    setlocale(LC_ALL, "Russian");

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argv != NULL)
    {
        for (int i = 1; i < argc; ++i)
        {
            std::wstring arg = argv[i];

            if (arg == L"-debug")
            {
                EngineConfig::IsReleaseMode = true;
                std::cout << "enbled release mode" << std::endl;
            }
        }

        LocalFree(argv);
    }
    EngineConfig::IsReleaseMode = true;

    auto manager = LuaManager::GetInstance();

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if (GetModuleFileNameW(hModule, path, MAX_PATH) > 0)
    {
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }

    Application::Create(hInstance);
    {
        std::shared_ptr<GameSample> demo = std::make_shared<GameSample>(L"Bian Game", 1280, 720, false);
        retCode = Application::Get().Run(demo);
        demo.reset();
    }

    Application::Destroy();

    //atexit(&ReportLiveObjects);
    return retCode;
}