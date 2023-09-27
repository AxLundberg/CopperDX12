#pragma once
#include "cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	enum class SamplerType
	{
		POINT,
		BILINEAR,
		ANISOTROPIC
	};

	enum class AddressMode
	{
		WRAP,
		CLAMP,
		BLACK_BORDER_COLOUR
	};

	class SamplerManager
	{
	private:
		static constexpr u32 HEAP_SIZE = 10;

	public:
		SamplerManager(ComPtr<ID3D12Device5> const& device);
		~SamplerManager();

		ResourceIndex CreateSampler(SamplerType type, AddressMode adressMode);
		D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle(ResourceIndex idx);
		void SetSamplerHeap(ID3D12GraphicsCommandList*& cmdList);
	private:
		ComPtr<ID3D12Device5> _device;
		ID3D12DescriptorHeap* _samplerHeap;

		u32 _currentSize = 0u;
	};
}