#pragma once
#include "../IDevice.h"
#include "cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	class IDevice : public GFX::IDevice
	{
	public:
		virtual Microsoft::WRL::ComPtr<ID3D12Device5> AsD3D12Device() = 0;
	};

	class Device : public IDevice
	{
	public:
		Device();
		~Device();
		virtual Microsoft::WRL::ComPtr<ID3D12Device5> AsD3D12Device() override;
	private:
		Microsoft::WRL::ComPtr<ID3D12Device5> device_;
	};
}
