#include "DirectX12_API.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Memory/Memory.h"
#include "Engine/Platform/Platform.h"

#include <stdarg.h>

// DirectX 12 headers.
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <dxgi1_6.h>
#include <wrl.h>

// ImGui backend
#if defined(RH_IMGUI)
    #include "imgui.h"
    #include "backends/imgui_impl_win32.h"
    #include "backends/imgui_impl_dx12.h"
#endif

struct DX12State {
    // DirectX 12 Objects
    Microsoft::WRL::ComPtr<ID3D12Device5>               Device;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue>          CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>  CmdList;

    Microsoft::WRL::ComPtr<IDXGISwapChain4>             SwapChain;
    Microsoft::WRL::ComPtr<ID3D12Resource>              DepthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         PSO_Standard;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         PSO_Blend;
    uint32                                              backbuffer_width  = 100;
    uint32                                              backbuffer_height = 100;

    // Descriptor Heaps
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        RTV_DescriptorHeap;
    uint32                                              RTV_DescriptorSize;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        DSV_DescriptorHeap;
    uint32                                              DSV_DescriptorSize;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        CBV_SRV_UAV_DescriptorHeap;
    uint32                                              CBV_SRV_UAV_DescriptorSize;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        imgui_heap;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        Sam_DescriptorHeap;
    uint32                                              Sam_DescriptorSize;


    // mesh and data
    Microsoft::WRL::ComPtr<ID3D12Resource>              TriangleVBResource;
    //Render_Geometry                                     Triangle;

    Microsoft::WRL::ComPtr<ID3D12Resource>              ScreenVBResource;
    //Render_Geometry                                     Screen;

    // texture
    Microsoft::WRL::ComPtr<ID3D12Resource>              TextureMetalResource;
    //Renderer_Texture                                    TextureMetal;

    Microsoft::WRL::ComPtr<ID3D12Resource>              TextureChainlinkResource;
    //Renderer_Texture                                    TextureChainLink;

    // Frame Resources
    uint32                                              frame_idx;
    static const uint8                                  num_frames_in_flight = 3;
    Microsoft::WRL::ComPtr<ID3D12Fence>                 Fence;
    uint64                                              CurrentFence = 0;
    struct FrameResources {
        Microsoft::WRL::ComPtr<ID3D12Resource>          BackBuffer;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>  CommandAllocator;
        
        Microsoft::WRL::ComPtr<ID3D12Resource>          upload_cbuffer_PerFrame;
        BYTE*                                           upload_cbuffer_PerFrame_mapped = nullptr;

        Microsoft::WRL::ComPtr<ID3D12Resource>          upload_cbuffer_PerModel;
        BYTE*                                           upload_cbuffer_PerModel_mapped = nullptr;

        uint64                                          FenceValue = 0;
    } frames[DX12State::num_frames_in_flight];
};
global_variable DX12State dx12;

void FlushDirectQueue();

bool32 DirectX12_api::initialize(const char* application_name, platform_state* plat_state) {
    using namespace Microsoft::WRL;

    // Enable D3D12 debug layer
    #if defined(DEBUG) || defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debug_controller;
        if FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))) {
            RH_FATAL("Could not get debug interface!");
            return false;
        }
        debug_controller->EnableDebugLayer();
        RH_INFO("Debug Layer Enabled");
    }
#endif

    // Create DXGI Factory
    ComPtr<IDXGIFactory4> factory;
    {
        UINT createFactoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

        if FAILED(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory))) {
            RH_FATAL("Failed to create factory...");
            return false;
        }
    }

    // Find adapter with the highest vram
    ComPtr<IDXGIAdapter1> adapter;
    uint32 max_idx = 0;
    uint64 max_vram = 0;
    {
        RH_INFO("------ Display Adapters ------------------------");

        uint32 i = 0;
        while (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            RH_TRACE("Device %d: %ls", i, desc.Description);
            if (desc.DedicatedVideoMemory > max_vram) {
                max_vram = desc.DedicatedVideoMemory;
                max_idx = i;
            }

            ++i;
        }

        // get the best choice now
        if (factory->EnumAdapters1(max_idx, &adapter) == DXGI_ERROR_NOT_FOUND) {
            RH_FATAL("could not find the best Device...\n");
            return false;
        }

        // print out description of selected adapter
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        RH_INFO("Chosen Device: '%ls'"
                "\n         VideoMemory:  %.1llf GB"
                "\n         SystemMemory: %.1llf GB"
                , desc.Description, ((double)desc.DedicatedVideoMemory) / (1024.0*1024.0*1024.0));
    }

    // create Device
    {
        if FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&dx12.Device))) {
            if FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dx12.Device))) {
                if FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dx12.Device))) {
                    if FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&dx12.Device))) {
                        if FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dx12.Device))) {
                            RH_FATAL("No feature levels supported, could not create a DX12 Device!");
                            return false;
                        } else {
                            RH_INFO("Created a DX12 Device with Feature Level: 11_0");
                        }
                    } else {
                        RH_INFO("Created a DX12 Device with Feature Level: 11_1");
                    }
                } else {
                    RH_INFO("Created a DX12 Device with Feature Level: 12_0");
                }
            } else {
                RH_INFO("Created a DX12 Device with Feature Level: 12_1");
            }
        } else {
            RH_INFO("Created a DX12 Device with Feature Level: 12_2");
        }

        static const D3D_FEATURE_LEVEL FEATURE_LEVELS_ARRAY[] =
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
        };
        static const D3D_FEATURE_LEVEL MAX_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_2;
        static const D3D_FEATURE_LEVEL MIN_FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_0;

        D3D12_FEATURE_DATA_FEATURE_LEVELS levels = {
            _countof(FEATURE_LEVELS_ARRAY), FEATURE_LEVELS_ARRAY, MAX_FEATURE_LEVEL
        };
        dx12.Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &levels, sizeof(levels));

        // check for raytracing support
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 opt5 = {};
        dx12.Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &opt5, sizeof(opt5));
        if (opt5.RaytracingTier) {
            RH_INFO("Feature: Raytracing Tier: %.1f", (real32)opt5.RaytracingTier / 10.0f);
        }

        // enable debug messages
#if defined(DEBUG) || defined(_DEBUG)
        ComPtr<ID3D12InfoQueue> pInfoQueue;
        if (SUCCEEDED(dx12.Device->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
            //if (SUCCEEDED(dx12.Device.As(&pInfoQueue)))
        {
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

            // Suppress whole categories of messages
            //D3D12_MESSAGE_CATEGORY Categories[] = {};

            // Suppress messages based on their severity level
            D3D12_MESSAGE_SEVERITY Severities[] =
            {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            // Suppress individual messages by their ID
            D3D12_MESSAGE_ID DenyIds[] = {
                //D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
            };

            D3D12_INFO_QUEUE_FILTER NewFilter = {};
            //NewFilter.DenyList.NumCategories = _countof(Categories);
            //NewFilter.DenyList.pCategoryList = Categories;
            NewFilter.DenyList.NumSeverities = _countof(Severities);
            NewFilter.DenyList.pSeverityList = Severities;
            NewFilter.DenyList.NumIDs = _countof(DenyIds);
            NewFilter.DenyList.pIDList = DenyIds;

            if (FAILED(pInfoQueue->PushStorageFilter(&NewFilter))) {
                RH_FATAL("Failed to setup info-queue for debug messages");
                return false;
            }

            // THIS FEATURE REQUIRES WINDOWS BUILD 20236 OR HIGHER!!!
            // :(
            /*
                ComPtr<ID3D12InfoQueue1> pInfoQueue1;
                if (SUCCEEDED(pInfoQueue->QueryInterface(IID_PPV_ARGS(&pInfoQueue1)))) {
                D3D12_MESSAGE_CALLBACK_FLAGS callback_flags = D3D12_MESSAGE_CALLBACK_FLAG_NONE;
                if SUCCEEDED(pInfoQueue1->RegisterMessageCallback(d3d_debug_msg_callback, callback_flags, NULL, &dx12.callback_cookie)) {
                RH_FATAL("Could not register msg callback...");
                return false;
                }
                } else {
                RH_FATAL("Could not create info-queue");
                return false;
                }
                */
        } else {
            RH_FATAL("Could not create info-queue");
            return false;
        }
#endif
    }

    // Check for root signature version 1.2
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE sig;
        sig.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_2;
        if (FAILED(dx12.Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &sig, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE)))) {

            // 1.2 not valid. Check for 1.1
            sig.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            if (FAILED(dx12.Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &sig, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE)))) {

                // 1.1 not valid. Check for 1.0
                sig.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
                if (FAILED(dx12.Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &sig, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE)))) {
            
                    // 1.0 not valid. This shouldn't happen?
                    RH_FATAL("Root Signature 1.0 not supported,");
                    return false;
                } else {
                    RH_INFO("Feature: Root Signature 1.0");
                }
            } else {
                RH_INFO("Feature: Root Signature 1.1");
            }
        } else {
            RH_INFO("Feature: Root Signature 1.2");
        }
    }

    // Create Command Queue
    {
        D3D12_COMMAND_QUEUE_DESC queue_desc = {};

        // Direct Queue (for normal 3D rendering)
        queue_desc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queue_desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.NodeMask = 0;

        if (FAILED(dx12.Device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&dx12.CommandQueue)))) {
            RH_FATAL("Could not create direct queue");
            return false;
        }
    }

    // Create Swap Chain with 3 frames in flight
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width         = dx12.backbuffer_width;
        swapChainDesc.Height        = dx12.backbuffer_height;
        swapChainDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo        = FALSE;
        swapChainDesc.SampleDesc    = { 1, 0 };
        swapChainDesc.BufferUsage   = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount   = dx12.num_frames_in_flight;
        swapChainDesc.Scaling       = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect    = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode     = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags         = 0;
        // It is recommended to always allow tearing if tearing support is available.
        //swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

        ComPtr<IDXGISwapChain1> swapChain1;
        HWND window = (HWND)platform_get_window_handle();
        if FAILED(factory->CreateSwapChainForHwnd(
            dx12.CommandQueue.Get(),
            window,
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1)) {
            RH_FATAL("Could not create swap chain");
            return false;
        }

        // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
        // will be handled manually.
        if FAILED(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER)) {
            RH_FATAL("Could not disable alt-enter");
            return false;
        }

        if FAILED(swapChain1.As(&dx12.SwapChain)) {
            RH_FATAL("Could not turn swapchain1 into swapchain4");
            return false;
        }

        dx12.frame_idx = dx12.SwapChain->GetCurrentBackBufferIndex();
    }

    // Query Descriptor Heap Sizes
    dx12.RTV_DescriptorSize         = dx12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);         // 32 bytes
    dx12.DSV_DescriptorSize         = dx12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);         // 8  bytes
    dx12.CBV_SRV_UAV_DescriptorSize = dx12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // 32 bytes
    dx12.Sam_DescriptorSize         = dx12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);     // 32 bytes

    // Create Descriptor Heap
    { // RTV
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = dx12.num_frames_in_flight;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        if FAILED(dx12.Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dx12.RTV_DescriptorHeap))) {
            RH_FATAL("Failed making descriptor heap");
            return false;
        }
    }
    { // DSV
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask       = 0;

        if FAILED(dx12.Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dx12.DSV_DescriptorHeap))) {
            RH_FATAL("Failed making descriptor heap");
            return false;
        }
    }
    { // CBV/SRV/UAV
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1000;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask       = 0;

        if FAILED(dx12.Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dx12.CBV_SRV_UAV_DescriptorHeap))) {
            RH_FATAL("Failed making descriptor heap");
            return false;
        }
    }
    { // Samplers
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask       = 0;

        if FAILED(dx12.Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dx12.Sam_DescriptorHeap))) {
            RH_FATAL("Failed making descriptor heap");
            return false;
        }
    }

    // setup back buffers
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(dx12.RTV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        for (int i = 0; i < dx12.num_frames_in_flight; ++i)
        {
            ComPtr<ID3D12Resource> backBuffer;
            if FAILED(dx12.SwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer))) {
                RH_FATAL("Failed getting backbuffer %d", i);
                return false;
            }

            dx12.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

            dx12.frames[i].BackBuffer = backBuffer;

            rtvHandle.Offset(dx12.RTV_DescriptorSize);
        }
    }

    // command allocators - one direct per frame, one total for copy
    for (uint8 n = 0; n < dx12.num_frames_in_flight; n++) {
        if (FAILED(dx12.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&dx12.frames[n].CommandAllocator)))) {
            RH_FATAL("Could not create command allocators");
            return false;
        }
    }

    // create command lists
    {
        // One for direct renderering
        if FAILED(dx12.Device->CreateCommandList(0, 
                                                 D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                                 dx12.frames[dx12.frame_idx].CommandAllocator.Get(), 
                                                 nullptr, 
                                                 IID_PPV_ARGS(dx12.CmdList.GetAddressOf()))) {
            RH_FATAL("Could not create command list!");
            return false;
        }
        // start it closed, since each update cycle starts with reset, and it needs to be closed.
        dx12.CmdList->Close();
    }

    // create depth/stencil buffer
    {
        DXGI_FORMAT depth_fmt = DXGI_FORMAT_D24_UNORM_S8_UINT;
        D3D12_RESOURCE_DESC desc;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Alignment = 0;
        desc.Width  = dx12.backbuffer_width;
        desc.Height = dx12.backbuffer_height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = depth_fmt;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clear;
        clear.Format = depth_fmt;
        clear.DepthStencil.Depth = 1.0f;
        clear.DepthStencil.Stencil = 0;

        CD3DX12_HEAP_PROPERTIES p(D3D12_HEAP_TYPE_DEFAULT);
        if (FAILED(dx12.Device->CreateCommittedResource(&p,
                                                        D3D12_HEAP_FLAG_NONE,
                                                        &desc,
                                                        D3D12_RESOURCE_STATE_COMMON,
                                                        &clear,
                                                        IID_PPV_ARGS(dx12.DepthStencilBuffer.GetAddressOf())))) {
            RH_FATAL("Failed to create depth/stencil buffer");
            return false;
        }

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format             = depth_fmt;
        dsvDesc.Texture2D.MipSlice = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dx12.DSV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        dx12.Device->CreateDepthStencilView(dx12.DepthStencilBuffer.Get(), &dsvDesc, dsv);
    }

    // create fence
    if FAILED(dx12.Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12.Fence))) {
        RH_FATAL("Could not create fence!");
        return false;
    }

    // Perform initialization commands. This requires creating a command list 
    // and sending it to the GPU
    // TODO: Figure out if stuff like this should be on a copy command list?
    // TODO: maybe move this!

    {
        auto alloc = dx12.frames[dx12.frame_idx].CommandAllocator;
        auto cmdlist = dx12.CmdList;
        auto queue = dx12.CommandQueue;

        alloc->Reset();
        cmdlist->Reset(alloc.Get(), nullptr);

        {
            // Transition the resource from its initial state to be used as a depth buffer.
            auto t = CD3DX12_RESOURCE_BARRIER::Transition(dx12.DepthStencilBuffer.Get(),
                                                          D3D12_RESOURCE_STATE_COMMON,
                                                          D3D12_RESOURCE_STATE_DEPTH_WRITE);
            cmdlist->ResourceBarrier(1, &t);
        }

        // Execute the initialization commands.
        if FAILED(cmdlist->Close()) {
            RH_FATAL("Failed to close command list");
        }
        ID3D12CommandList* cmdsLists[] = { cmdlist.Get() };
        queue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        FlushDirectQueue();
    }

    return true;
}
void DirectX12_api::shutdown() {
    // flush the current direct command queue and for for it to finish.

    FlushDirectQueue();
}

void FlushDirectQueue() {
    // Advance the fence value to mark commands up to this fence point.
    dx12.frames[dx12.frame_idx].FenceValue = ++dx12.CurrentFence;

    // Add an instruction to the command queue to set a new fence point.  Because we 
    // are on the GPU timeline, the new fence point won't be set until the GPU finishes
    // processing all the commands prior to this Signal().
    if FAILED(dx12.CommandQueue->Signal(dx12.Fence.Get(), dx12.frames[dx12.frame_idx].FenceValue)) {
        RH_FATAL("Failed to signal");
    }

    // Wait until the GPU has completed commands up to this fence point.
    if(dx12.Fence->GetCompletedValue() < dx12.frames[dx12.frame_idx].FenceValue)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

        // Fire event when GPU hits current fence.  
        if FAILED(dx12.Fence->SetEventOnCompletion(dx12.frames[dx12.frame_idx].FenceValue, eventHandle)) {
            RH_FATAL("Failed to set event");
        }

        // Wait until the GPU hits current fence event is fired.
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}

void DirectX12_api::resized(uint16 width, uint16 height) {
    // need to wait until ALL backbuffers are done being used first
    for (uint32 n = 0; n < dx12.num_frames_in_flight; n++) {
        dx12.frames[n].FenceValue = ++dx12.CurrentFence;
        dx12.CommandQueue->Signal(dx12.Fence.Get(), dx12.frames[n].FenceValue);
        if (dx12.frames[n].FenceValue != 0 && dx12.Fence->GetCompletedValue() < dx12.frames[n].FenceValue) {
            HANDLE event_handle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
            dx12.Fence->SetEventOnCompletion(dx12.frames[n].FenceValue, event_handle);
            ::WaitForSingleObject(event_handle, INFINITE);
            ::CloseHandle(event_handle);
            RH_TRACE("Done waiting for frame %u", n);
        }
    }

    // Now we can resize the swapchain without worrying if the GPU
    // is currently in use
    // Things to resize:
    //  - Swapchain
    //  - Backbuffer Render Targets
    //  - Default Depth/Stencil Buffer

    // Release the previous resources we will be recreating.
    for (int n = 0; n < dx12.num_frames_in_flight; n++) {
        dx12.frames[n].BackBuffer.Reset();
    }
    dx12.DepthStencilBuffer.Reset();

    dx12.SwapChain->ResizeBuffers(dx12.num_frames_in_flight,
                                  static_cast<UINT>(width), static_cast<UINT>(height),
                                  DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    dx12.frame_idx = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(dx12.RTV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < dx12.num_frames_in_flight; i++) {
        dx12.SwapChain->GetBuffer(i, IID_PPV_ARGS(&dx12.frames[i].BackBuffer));
        dx12.Device->CreateRenderTargetView(dx12.frames[i].BackBuffer.Get(), nullptr, rtv);
        rtv.Offset(1, dx12.RTV_DescriptorSize);
    }

    // Create the depth/stencil buffer and view.
    D3D12_RESOURCE_DESC desc;
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment          = 0;
    desc.Width              = width;
    desc.Height             = height;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clear;
    clear.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
    clear.DepthStencil.Depth   = 1.0f;
    clear.DepthStencil.Stencil = 0;

    auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    if FAILED(dx12.Device->CreateCommittedResource(
        &heap_prop, D3D12_HEAP_FLAG_NONE,
        &desc, D3D12_RESOURCE_STATE_COMMON, &clear,
        IID_PPV_ARGS(dx12.DepthStencilBuffer.GetAddressOf()))) {
        RH_FATAL("Failed to create new depth/stencil buffer");
    }

    // Create descriptor to mip level 0 of entire resource using the format of the resource.
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags              = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dx12.DSV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    
    dx12.Device->CreateDepthStencilView(dx12.DepthStencilBuffer.Get(), &dsvDesc, dsv);

    // Transition the resource from its initial state to be used as a depth buffer.
    auto trn = CD3DX12_RESOURCE_BARRIER::Transition(dx12.DepthStencilBuffer.Get(),
                                                    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    dx12.frames[dx12.frame_idx].CommandAllocator->Reset();
    dx12.CmdList->Reset(dx12.frames[dx12.frame_idx].CommandAllocator.Get(), nullptr);

    dx12.CmdList->ResourceBarrier(1, &trn);

    dx12.CmdList->Close();

    // Execute the resize commands.
    ID3D12CommandList* cmdsLists[] = { dx12.CmdList.Get() };
    dx12.CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until resize is complete.
    FlushDirectQueue();

    // Update the viewport transform to cover the client area.
    dx12.backbuffer_width  = width;
    dx12.backbuffer_height = height;

    RH_TRACE("Renderer resized %u x %u", width, height);
}

bool32 DirectX12_api::begin_frame(real32 delta_time) {
    //
    // Wait for new frame to be done on the GPU
    //
    uint64 completed_value = dx12.Fence->GetCompletedValue();
    if (dx12.frames[dx12.frame_idx].FenceValue != 0 && dx12.Fence->GetCompletedValue() < dx12.frames[dx12.frame_idx].FenceValue) {
        HANDLE event_handle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        dx12.Fence->SetEventOnCompletion(dx12.frames[dx12.frame_idx].FenceValue, event_handle);
        ::WaitForSingleObject(event_handle, INFINITE);
        ::CloseHandle(event_handle);
    }


    //
    // Start recording commands
    //
    auto allocator  = dx12.frames[dx12.frame_idx].CommandAllocator;
    auto backbuffer = dx12.frames[dx12.frame_idx].BackBuffer;

    allocator->Reset();
    dx12.CmdList->Reset(allocator.Get(), dx12.PSO_Standard.Get());

    return true;
}
bool32 DirectX12_api::end_frame(real32 delta_time) {
    auto backbuffer = dx12.frames[dx12.frame_idx].BackBuffer;

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backbuffer.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    dx12.CmdList->ResourceBarrier(1, &barrier);

    if FAILED(dx12.CmdList->Close()) {
        RH_FATAL("Could not close command list.");
        return false;
    }

    ID3D12CommandList* const commandLists[] = {
        dx12.CmdList.Get()
    };
    dx12.CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    return true;
}
bool32 DirectX12_api::present(uint32 sync_interval) {
    const uint32 presentFlags = 0;
    if FAILED(dx12.SwapChain->Present(sync_interval, presentFlags)) {
        RH_FATAL("Error presenting.");
        return false;
    }

    // signal fence value
    dx12.frames[dx12.frame_idx].FenceValue = ++dx12.CurrentFence;
    dx12.CommandQueue->Signal(dx12.Fence.Get(), dx12.frames[dx12.frame_idx].FenceValue);

    dx12.frame_idx = dx12.SwapChain->GetCurrentBackBufferIndex();

    return true;
}

bool32 DirectX12_api::ImGui_Init() {
    #if defined(RH_IMGUI)
        HWND window = GetActiveWindow();
        if (!window) {
            return false;
        }

        // create the cbv/srv/uav heap that ImGui will use
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask       = 0;

        if FAILED(dx12.Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dx12.imgui_heap))) {
            RH_FATAL("Failed making imgui_heap descriptor heap");
            return false;
        }
        
        // setup imgui
        ImGui_ImplWin32_Init(window);
        ImGui_ImplDX12_Init(dx12.Device.Get(), dx12.num_frames_in_flight, DXGI_FORMAT_R8G8B8A8_UNORM, 
                            dx12.imgui_heap.Get(),
                            dx12.imgui_heap->GetCPUDescriptorHandleForHeapStart(),
                            dx12.imgui_heap->GetGPUDescriptorHandleForHeapStart());
    #endif

    return true;
}
bool32 DirectX12_api::ImGui_begin_frame() {
    #if defined(RH_IMGUI)
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
    #endif

    return true;
}
bool32 DirectX12_api::ImGui_end_frame() {
    #if defined(RH_IMGUI)
        // Bind the descriptor heaps being used
        ID3D12DescriptorHeap* descriptorHeaps[] = { dx12.imgui_heap.Get() };
        dx12.CmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dx12.CmdList.Get());
    #endif

    return true;
}
bool32 DirectX12_api::ImGui_Shutdown() {
    #if defined(RH_IMGUI)
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
    #endif

    return true;
}

void DirectX12_api::set_draw_mode(render_draw_mode mode) {
}

void DirectX12_api::set_highlight_mode(bool32 enabled) {
}

void DirectX12_api::disable_depth_test() {
}
void DirectX12_api::enable_depth_test() {
}

void DirectX12_api::disable_depth_mask() {
}
void DirectX12_api::enable_depth_mask() {
}

/* Stencil Testing */
void DirectX12_api::enable_stencil_test() {
}
void DirectX12_api::disable_stencil_test() {
}
void DirectX12_api::set_stencil_mask(uint32 mask) {
}
void DirectX12_api::set_stencil_func(render_stencil_func func, uint32 ref, uint32 mask) {
}
void DirectX12_api::set_stencil_op(render_stencil_op sfail, render_stencil_op dpfail, render_stencil_op dppass) {
}

void DirectX12_api::push_debug_group(const char* label) {
}
void DirectX12_api::pop_debug_group() {
}

void DirectX12_api::create_texture_2D(struct render_texture_2D* texture, 
                                   texture_creation_info_2D create_info, 
                                   const void* data, bool32 is_hdr) {
    texture->handle = 0;
}
void DirectX12_api::create_texture_cube(struct render_texture_cube* texture,
                                     texture_creation_info_cube create_info, 
                                     const void*** data, bool32 is_hdr, uint32 mip_levels) {
    texture->handle = 0;
}

void DirectX12_api::create_texture_3D(struct render_texture_3D* texture, 
                                   texture_creation_info_3D create_info, 
                                   const void* data, bool32 is_hdr) {
    texture->handle = 0;
}

void DirectX12_api::destroy_texture_2D(struct render_texture_2D* texture) {
}
void DirectX12_api::destroy_texture_3D(struct render_texture_3D* texture) {
}
void DirectX12_api::destroy_texture_cube(struct render_texture_cube* texture) {
}


void DirectX12_api::create_mesh(render_geometry* mesh, 
                             uint32 num_verts, const void* vertices,
                             uint32 num_inds, const uint32* indices,
                             const ShaderDataType* attributes) {
    mesh->handle = 0;
    mesh->num_inds = 0;
    mesh->num_verts = 0;
    mesh->flag = 0;
}
void DirectX12_api::destroy_mesh(render_geometry* mesh) {
}

bool32 DirectX12_api::create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) {
    shader_prog->handle = 0;

    return true;
}
void DirectX12_api::destroy_shader(shader* shader_prog) {
}

bool32 DirectX12_api::create_framebuffer(frame_buffer* fbo, int num_attachments, 
                                      const frame_buffer_attachment* attachments,
                                      frame_buffer_create_info info) {
    fbo->handle = 0;
    fbo->width = 0;
    fbo->height = 0;
    fbo->num_attachments = 0;

    return true;
}
bool32 DirectX12_api::recreate_framebuffer(frame_buffer* fbo) {
    return true;
}
bool32 DirectX12_api::create_framebuffer_cube(frame_buffer* fbo, 
                                           int num_attachments, 
                                           const frame_buffer_attachment* attachments,
                                           bool32 generate_mipmaps) {
    fbo->handle = 0;
    fbo->width = 0;
    fbo->height = 0;
    fbo->num_attachments = 0;

    return true;
}
void DirectX12_api::destroy_framebuffer(frame_buffer* fbo) {
}
void DirectX12_api::set_framebuffer_cube_face(frame_buffer* fbuffer, uint32 attach_idx, uint32 slot, uint32 mip_level) {
}
void DirectX12_api::resize_framebuffer_renderbuffer(frame_buffer* fbuffer, uint32 new_width, uint32 new_height) {
}
void DirectX12_api::copy_framebuffer_depthbuffer(frame_buffer * src, frame_buffer * dst) {
}
void DirectX12_api::copy_framebuffer_stencilbuffer(frame_buffer * src, frame_buffer * dst) {
}


void DirectX12_api::use_shader(shader* shader_prog) {
}
void DirectX12_api::use_framebuffer(frame_buffer* fbuffer) {
}

void DirectX12_api::draw_geometry(render_geometry* geom) {
}
void DirectX12_api::draw_geometry(render_geometry* geom, uint32 start_idx, uint32 num_inds) {
}
void DirectX12_api::draw_geometry(render_geometry* geom, render_material* mat) {
}
void DirectX12_api::draw_geometry_lines(render_geometry* geom) {
}
void DirectX12_api::draw_geometry_points(render_geometry* geom) {
}

void DirectX12_api::bind_texture_2D(render_texture_2D texture, uint32 slot) {
}
void DirectX12_api::bind_texture_3D(render_texture_3D texture, uint32 slot) {
}
void DirectX12_api::bind_texture_cube(render_texture_cube texture, uint32 slot) {
}


void DirectX12_api::bind_texture_2D(frame_buffer_attachment attachment, uint32 slot) {
}
void DirectX12_api::bind_texture_3D(frame_buffer_attachment attachment, uint32 slot) {
}
void DirectX12_api::bind_texture_cube(frame_buffer_attachment attachment, uint32 slot) {
}



void DirectX12_api::set_viewport(uint32 x, uint32 y, uint32 width, uint32 height) {
    // note: CmdList needs to be ready to record commands

    // set viewport and scissor rectangles
    // todo: separate function to set scissor

    D3D12_VIEWPORT viewport;
    viewport.TopLeftX = static_cast<float>(x);
    viewport.TopLeftY = static_cast<float>(y);
    viewport.Width    = static_cast<float>(width);
    viewport.Height   = static_cast<float>(height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    dx12.CmdList->RSSetViewports(1, &viewport);

    D3D12_RECT scissor;
    scissor.left   = static_cast<int>(x);
    scissor.top    = static_cast<int>(y);
    scissor.right  = static_cast<int>(width);
    scissor.bottom = static_cast<int>(height);

    dx12.CmdList->RSSetScissorRects(1, &scissor);
}
void DirectX12_api::clear_viewport(real32 r, real32 g, real32 b, real32 a) {
    auto backbuffer = dx12.frames[dx12.frame_idx].BackBuffer;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(dx12.RTV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                      dx12.frame_idx, dx12.RTV_DescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(dx12.DSV_DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // clear render target
    {
        // transition the backbuffer into render target
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backbuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        dx12.CmdList->ResourceBarrier(1, &barrier);

        // clear the color buffer
        FLOAT color[] = { 0.4f, 0.6f, 0.9f, 1.0f }; // cornflower blue
        dx12.CmdList->ClearRenderTargetView(rtv, color, 0, nullptr);

        // clear the depth/stencil buffer
        dx12.CmdList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        // specify render target to use
        dx12.CmdList->OMSetRenderTargets(1, &rtv, true, &dsv);
    }
}
void DirectX12_api::clear_viewport_only_color(real32 r, real32 g, real32 b, real32 a) {
}
void DirectX12_api::clear_framebuffer_attachment(frame_buffer_attachment *attach, real32 r, real32 b, real32 g, real32 a) {
}

void DirectX12_api::get_texture_data(render_texture_2D texture, void* data, int num_channels, bool is_hdr, uint32 mip) {
}

void DirectX12_api::get_cubemap_data(render_texture_cube texture, void* data, int num_channels, bool is_hdr, uint32 face, uint32 mip) {
}

void DirectX12_api::upload_uniform_float(   ShaderUniform_float uniform, real32  value) {
}
void DirectX12_api::upload_uniform_float2(  ShaderUniform_vec2 uniform, const laml::Vec2& values) {
}
void DirectX12_api::upload_uniform_float3(  ShaderUniform_vec3 uniform, const laml::Vec3& values) {
}
void DirectX12_api::upload_uniform_float4(  ShaderUniform_vec4 uniform, const laml::Vec4& values) {
}
void DirectX12_api::upload_uniform_float4x4(ShaderUniform_mat4 uniform, const laml::Mat4& values) {
}
void DirectX12_api::upload_uniform_int(     ShaderUniform_int  uniform, int32  value) {
}
void DirectX12_api::upload_uniform_int2(    ShaderUniform_ivec2 uniform, const laml::Vector<int32,2>& values) {
}
void DirectX12_api::upload_uniform_int3(    ShaderUniform_ivec3 uniform, const laml::Vector<int32,3>& values) {
}
void DirectX12_api::upload_uniform_int4(    ShaderUniform_ivec4 uniform, const laml::Vector<int32,4>& values) {
}