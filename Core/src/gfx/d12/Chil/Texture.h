#pragma once
#include <Core/src/win/CopperWin.h>
#include <Core/src/spa/Dimensions.h>
#include <initguid.h>
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <wrl/client.h>
#include "AllocatorListPair.h"
#include <string>

namespace CPR::GFX::D12
{
	class ITexture
	{
	public:
		virtual ~ITexture() = default;
		virtual void WriteDescriptor(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE handle) const = 0;
		virtual SPA::DimensionsI GetDimensions() const = 0;
	};

	class Texture : public ITexture
	{
	public:
		Texture(Microsoft::WRL::ComPtr<ID3D12Device2> pDevice, AllocatorListPair cmd, std::wstring path);
		void ClearIntermediate() { pIntermediate_.Reset(); }
		void WriteDescriptor(ID3D12Device* pDevice, D3D12_CPU_DESCRIPTOR_HANDLE handle) const override;
		SPA::DimensionsI GetDimensions() const override;
		~Texture() override;
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> pResource_;
		SPA::DimensionsI dimensions_;
		Microsoft::WRL::ComPtr<ID3D12Resource> pIntermediate_;
	};
}
