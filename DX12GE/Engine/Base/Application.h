/**
* The application class is used to create windows for our application.
*/
#pragma once

#include "DX12LibPCH.h"
#include <string>

class Window;
class Game;
class CommandQueue;

class Application
{
public:

    /**  
    Create the application singleton with the application instance handle.
    */
    static void Create(HINSTANCE hInst);

    /**
    Destroy the application instance and all windows created by this application instance.
    */
    static void Destroy();
    /**
    Get the application singleton.
    */
    static Application& Get();

    /**
    Check to see if VSync-off is supported.
    */
    bool IsTearingSupported() const;

    /**
    * Create a new DirectX11 render window instance.
    * @param windowName The name of the window. This name will appear in the title bar of the window. This name should be unique.
    * @param clientWidth The width (in pixels) of the window's client area.
    * @param clientHeight The height (in pixels) of the window's client area.
    * @param vSync Should the rendering be synchronized with the vertical refresh rate of the screen.
    * @param windowed If true, the window will be created in windowed mode. If false, the window will be created full-screen.
    * @returns The created window instance. If an error occurred while creating the window an invalid
    * window instance is returned. If a window with the given name already exists, that window will be
    * returned.
    */
    std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true);

    /**
    * Destroy a window given the window name.
    */
    void DestroyWindow(const std::wstring& windowName);
    /**
    * Destroy a window given the window reference.
    */
    void DestroyWindow(std::shared_ptr<Window> window);

    /**
    * Find a window by the window name.
    */
    std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

    /**
    * Run the application loop and message pump.
    * @return The error code if an error occurred.
    */
    int Run(std::shared_ptr<Game> pGame);

    /**
    * Request to quit the application and close all windows.
    * @param exitCode The error code to return to the invoking process.
    */
    void Quit(int exitCode = 0);

    /**
     * Get the Direct3D 12 device
     */
    ComPtr<ID3D12Device2> GetPrimaryDevice() const;

    /**
     * Get the additional device
     */
    //ComPtr<ID3D12Device2> GetSecondDevice() const;

    /**
     * Get a command queue. Valid types are:
     * - D3D12_COMMAND_LIST_TYPE_DIRECT : Can be used for draw, dispatch, or copy commands.
     * - D3D12_COMMAND_LIST_TYPE_COMPUTE: Can be used for dispatch or copy commands.
     * - D3D12_COMMAND_LIST_TYPE_COPY   : Can be used for copy commands.
     */
    std::shared_ptr<CommandQueue> GetPrimaryCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;
    //std::shared_ptr<CommandQueue> GetSecondCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

    // Flush all command queues.
    void Flush();

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
    UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

protected:

    // Create an application instance.
    Application(HINSTANCE hInst);
    // Destroy the application instance and all windows associated with this application.
    virtual ~Application();

    std::vector<ComPtr<IDXGIAdapter4>> GetAdapters();

    ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);
    bool CheckTearingSupport();

private:
    Application(const Application& copy) = delete;
    Application& operator=(const Application& other) = delete;

    // The application instance handle that this application was created with.
    HINSTANCE m_hInstance;

    ComPtr<IDXGIAdapter4> PrimaryAdapter;
    ComPtr<ID3D12Device2> PrimaryDevice;
    std::shared_ptr<CommandQueue> PrimaryDirectCommandQueue;
    std::shared_ptr<CommandQueue> PrimaryComputeCommandQueue;
    std::shared_ptr<CommandQueue> PrimaryCopyCommandQueue;

    //ComPtr<IDXGIAdapter4> SecondAdapter;
    //ComPtr<ID3D12Device2> SecondDevice;
    //std::shared_ptr<CommandQueue> SecondDirectCommandQueue;
    //std::shared_ptr<CommandQueue> SecondComputeCommandQueue;
    //std::shared_ptr<CommandQueue> SecondCopyCommandQueue;

    bool m_TearingSupported;
};