#pragma once
#include <vector>
#include <memory>
#include <d3d11_4.h>

#include "DeviceD11.h"
#include "cmn/D11Headers.h"
#include "../IBufferManager.h"

namespace CPR::GFX::D11
{
	class IBufferManager : public GFX::IBufferManager
	{
	public:
		virtual ID3D11Buffer* GetBufferInterface(ResourceIndex index) = 0;
		virtual ID3D11ShaderResourceView* GetSRV(ResourceIndex index) = 0;
	};

	class BufferManagerD11 : public IBufferManager
	{
	public:
		BufferManagerD11(std::shared_ptr<IDevice> device);
		~BufferManagerD11();

		ResourceIndex AddBuffer(void* data, const BufferInfo& info) override;

		void UpdateBuffer(ResourceIndex index, void* data) override;
		u32 GetElementSize(ResourceIndex index) override;
		u32 GetElementCount(ResourceIndex index) override;

		ID3D11Buffer* GetBufferInterface(ResourceIndex index) override;
		ID3D11ShaderResourceView* GetSRV(ResourceIndex index) override;
	private:
		bool DetermineUsage(PerFrameUsage, D3D11_USAGE& usage);
		UINT TranslateBindFlags(u32 bindingFlags);
		bool CreateDescription(u32 elementSize,
			u32 nrOfElements, PerFrameUsage rwPattern, u32 bindingFlags,
			D3D11_BUFFER_DESC& toSet);

		ID3D11ShaderResourceView* CreateSRV(ID3D11Buffer* buffer, u32 stride);


		struct StoredBuffer
		{
			ID3D11Buffer* interfacePtr = nullptr;
			u32 elementSize = 0;
			u32 elementCount = 0;
			ID3D11ShaderResourceView* srv;
		};

		std::shared_ptr<IDevice> deviceAndContext;
		std::vector<StoredBuffer> buffers;

	};
}
