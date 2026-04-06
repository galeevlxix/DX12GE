#include <Shlwapi.h>

#include "Engine/Base/Application.h"
#include "Game/GameSample.h"
#include <dxgidebug.h>
#include "EngineConfig.h"

void ReportLiveObjects()
{
    IDXGIDebug1* dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
   
    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
    dxgiDebug->Release();
}

bool ProcessCommandLine(int argc, char* argv[])
{
    if (argv == nullptr) return false;

    for (int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
		if (arg == nullptr) continue;

        printf("%d) %s\n", i, arg);

        if (arg == "--mode" || arg == "-m")
        {
            if (i + 1 < argc)
            {
                const char* mode = argv[i + 1];
                if (mode == nullptr) return false;  

                if (RuntimeModeMap.find(mode) == RuntimeModeMap.end()) return false;
                EngineConfigRuntimeMode _mode = RuntimeModeMap.at(mode);

                switch (_mode)
                {
                case EngineConfigRuntimeMode::RUNTIME_MODE_EDITING:

                    break;

                case EngineConfigRuntimeMode::RUNTIME_MODE_PLAYING:

                    break;

                default:
                    break;
                }
			}
        }
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    ProcessCommandLine(argc, argv);

    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    if (GetModuleFileNameW(hInstance, path, MAX_PATH) > 0)
    {
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }

    EngineConfig::Mode = EngineConfigRuntimeMode::RUNTIME_MODE_PLAYING;
    EngineConfig::Output = EngineConfigRuntimeOutput::RUNTIME_OUTPUT_WINDOW;

    Application::Create(hInstance);
    std::shared_ptr<GameSample> demo = std::make_shared<GameSample>(L"Bian Game", 1920, 1080, false);
    int retCode = Application::Get().Run(demo);
    demo.reset();

    Application::Destroy();

    //atexit(&ReportLiveObjects);
    return retCode;
}