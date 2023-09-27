#pragma once
#include "cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		u32 size;
	};
}
