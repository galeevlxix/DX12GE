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
#include <xaudio2.h>
#include <x3daudio.h>
#include <wrl/client.h>
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
#include <cstdint>
#include <fstream>
#include <stdexcept>

// Helper functions
#include "Helpers.h"

enum GraphicsAdapter : UINT
{
	GraphicAdapterPrimary = 0,
	GraphicAdapterSecond = 1
};

#define PI 3.1415926535f

enum TextureType
{
    NONE = 0,
    DIFFUSE = 1,
    SPECULAR = 2,
    AMBIENT = 3,
    EMISSIVE = 4,
    HEIGHT = 5,
    NORMALS = 6,
    SHININESS = 7,
    OPACITY = 8,
    DISPLACEMENT = 9,
    LIGHTMAP = 10,
    REFLECTION = 11,
    BASE_COLOR = 12,
    NORMAL_CAMERA = 13,
    EMISSION_COLOR = 14,
    METALNESS = 15,
    DIFFUSE_ROUGHNESS = 16,
    AMBIENT_OCCLUSION = 17,
    UNKNOWN = 18,
    SHEEN = 19,
    CLEARCOAT = 20,
    TRANSMISSION = 21,
    MAYA_BASE = 22,
    MAYA_SPECULAR = 23,
    MAYA_SPECULAR_COLOR = 24,
    MAYA_SPECULAR_ROUGHNESS = 25,
    ANISOTROPY = 26,
    GLTF_METALLIC_ROUGHNESS = 27,
};