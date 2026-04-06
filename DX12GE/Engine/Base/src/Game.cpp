#include "../Game.h"
#include "../Application.h"
#include "../Window.h"
#include "../../../EngineConfig.h"

Game::Game(const std::wstring& name, int width, int height, bool vSync) : m_Name(name), m_Width(width), m_Height(height), m_vSync(vSync)
{

}

Game::~Game()
{
    assert(!m_pWindow && "Use Game::Destroy() before destruction.");
}

bool Game::Initialize()
{
    // Check for DirectX Math library support.
    if (!DirectX::XMVerifyCPUSupport())
    {
        MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    switch (EngineConfig::Output)
    {
    case EngineConfigRuntimeOutput::RUNTIME_OUTPUT_EDITOR:
        m_pWindow = Application::Get().GetWindowByName(m_Name);
        if (!m_pWindow)
        {
            throw std::runtime_error("Viewport window was nullptr");
        }
        break;
    case EngineConfigRuntimeOutput::RUNTIME_OUTPUT_WINDOW:
        m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
        break;
    default:
        throw std::runtime_error("Unknown runtime output type");
    }

    m_pWindow->RegisterCallbacks(shared_from_this());
    m_pWindow->Show();

    return true;
}

void Game::Destroy()
{
    Application::Get().DestroyWindow(m_pWindow);
    m_pWindow.reset();
    m_pWindow = nullptr;
}

void Game::OnUpdate(UpdateEventArgs& e)
{

}

void Game::OnRender(RenderEventArgs& e)
{

}

void Game::OnKeyPressed(KeyEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnKeyReleased(KeyEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseMoved(class MouseMotionEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseWheel(MouseWheelEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnResize(ResizeEventArgs& e)
{
    m_Width = e.Width;
    m_Height = e.Height;
}

void Game::OnWindowDestroy()
{
    // If the Window which we are registered to is 
    // destroyed, then any resources which are associated 
    // to the window must be released.
    UnloadContent();
}

