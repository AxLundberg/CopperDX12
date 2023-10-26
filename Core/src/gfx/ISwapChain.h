#pragma once
#include <array>
#include <optional>
#include <Core/src/win/CopperWin.h>

#include "d12/cmn/D12Headers.h"
#include "cmn/GraphicsError.h"

namespace CPR::GFX
{

    class ISwapChain {
        // The common interface for all SwapChains
    public:
        struct IocParams
        {
            HWND window;
            std::optional<u32> nrOfBuffers;
        };
    public:
        virtual ~ISwapChain() = default;
        virtual void AsD3D12SwapChain(ComPtr<ID3D12CommandQueue> cmdQueue) { return; }
        virtual void SetClearColor(const std::array<f32, 4>& clear_color) = 0;
        virtual HWND GetHWND() = 0;
    };
}
