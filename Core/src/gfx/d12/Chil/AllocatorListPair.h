#pragma once
#pragma once
#include <Core/src/win/CopperWin.h>
#include <d3d12.h> 
#include <wrl/client.h>

namespace CPR::GFX::D12
{
	struct AllocatorListPair
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	};
}