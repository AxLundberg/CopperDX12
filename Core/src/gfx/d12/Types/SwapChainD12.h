#pragma once
#include <vector>

#include "../../ISwapChain.h"
#include "../cmn/D12Headers.h"

namespace CPR::GFX::D12
{
    class SwapChainD12 : public ISwapChain {
    public:
        SwapChainD12(HWND window, std::optional<u32> nrOfBuffers);
        void SetClearColor(const std::array<float, 4>& clear_color) override;
        void AsD3D12SwapChain(ComPtr<ID3D12CommandQueue> cmdQueue) override;
        HWND GetHWND() override;
    private:
        HWND _window;
        u32 _nrOfBuffers;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapChain;
        std::vector<u32> _buffers;
    };

}
