#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>

#include "Engine/Application.h"
#include "Game/BianGame.h"

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
        std::shared_ptr<BianGame> demo = std::make_shared<BianGame>(L"Bian Game", 1280, 720);
        //std::shared_ptr<BianGame> demo = std::make_shared<BianGame>(L"Bian Game", 1920, 1080, false);
        retCode = Application::Get().Run(demo);
    }

    Application::Destroy();

    atexit(&ReportLiveObjects);

    return retCode;
}