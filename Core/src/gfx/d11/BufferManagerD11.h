#pragma once
#include <vector>
#include <d3d11_4.h>

#include "cmn/D11Headers.h"

namespace CPR::GFX::D11
{
	enum class PerFrameUsage
	{
		STATIC,
		DYNAMIC
	};

	enum BufferBinding
	{
		STRUCTURED_BUFFER = 1,
		CONSTANT_BUFFER = 2
	};

	class BufferManagerD11
	{
	public:
		BufferManagerD11(ComPtr<ID3D11Device> const& device);
		~BufferManagerD11();
		void Initialise(ID3D11Device* deviceToUse,
			ID3D11DeviceContext* contextToUse);

		ResourceIndex AddBuffer(void* data, unsigned int elementSize,
			unsigned int nrOfElements, PerFrameUsage rwPattern, unsigned int bindingFlags);

		void UpdateBuffer(ResourceIndex index, void* data);
		unsigned int GetElementSize(ResourceIndex index);
		unsigned int GetElementCount(ResourceIndex index);

		ID3D11Buffer* GetBufferInterface(ResourceIndex index);
		ID3D11ShaderResourceView* GetSRV(ResourceIndex index);
	private:
		bool DetermineUsage(PerFrameUsage, D3D11_USAGE& usage);
		UINT TranslateBindFlags(unsigned int bindingFlags);
		bool CreateDescription(unsigned int elementSize,
			unsigned int nrOfElements, PerFrameUsage rwPattern, unsigned int bindingFlags,
			D3D11_BUFFER_DESC& toSet);

		ID3D11ShaderResourceView* CreateSRV(ID3D11Buffer* buffer, unsigned int stride);


		struct StoredBuffer
		{
			ID3D11Buffer* interfacePtr = nullptr;
			unsigned int elementSize = 0;
			unsigned int elementCount = 0;
			ID3D11ShaderResourceView* srv;
		};

		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;
		std::vector<StoredBuffer> buffers;

	};
}
