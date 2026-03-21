#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h> // For CommandLineToArgvW

// The min/max macros conflict with like-named member functions.
// Only use std::min and std::max defined in <algorithm>.
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h> // For ComPtr
using namespace Microsoft::WRL;

// DirectX 12 specific headers.
#include <d3d12.h> // For ID3D12CommandQueue, ID3D12Device2, and ID3D12Fence
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include "d3dx12.h"

// STL Headers
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <unordered_map>
#include <thread>
#include <vector>

// Helper functions
#include "Helpers.h"

enum GraphicsAdapter : UINT
{
	GraphicAdapterPrimary = 0,
	GraphicAdapterSecond = 1
};

#define PI 3.1415926535f
