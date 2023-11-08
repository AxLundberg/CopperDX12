#include "BufferManagerD11.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D11
{
	BufferManagerD11::BufferManagerD11(std::shared_ptr<IDevice> device)
		:
		deviceAndContext(std::move(device))
	{}

	BufferManagerD11::~BufferManagerD11()
	{
		for (auto& buffer : buffers)
		{
			buffer.interfacePtr->Release();
			if (buffer.srv != nullptr)
				buffer.srv->Release();
		}
	}

	bool BufferManagerD11::DetermineUsage(PerFrameUsage rwPattern, D3D11_USAGE& usage)
	{
		usage = rwPattern == PerFrameUsage::STATIC ?
			D3D11_USAGE_IMMUTABLE :
			D3D11_USAGE_DYNAMIC;

		return true;
	}

	UINT BufferManagerD11::TranslateBindFlags(u32 bindingFlags)
	{
		UINT toReturn = 0;

		if (bindingFlags & BufferBinding::CONSTANT_BUFFER)
			toReturn |= D3D11_BIND_CONSTANT_BUFFER;
		if (bindingFlags & BufferBinding::STRUCTURED_BUFFER)
			toReturn |= D3D11_BIND_SHADER_RESOURCE;

		return toReturn;
	}

	bool BufferManagerD11::CreateDescription(u32 elementSize,
		u32 nrOfElements, PerFrameUsage rwPattern, u32 bindingFlags,
		D3D11_BUFFER_DESC& toSet)
	{
		toSet.ByteWidth = elementSize * nrOfElements;

		if (bindingFlags & BufferBinding::CONSTANT_BUFFER)
			toSet.ByteWidth += 16 - toSet.ByteWidth % 16;

		toSet.BindFlags = TranslateBindFlags(bindingFlags);
		toSet.CPUAccessFlags = rwPattern != PerFrameUsage::STATIC ?
			D3D11_CPU_ACCESS_WRITE : 0;
		toSet.MiscFlags = bindingFlags & BufferBinding::STRUCTURED_BUFFER ?
			D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0;
		toSet.StructureByteStride = elementSize;
		bool result = DetermineUsage(rwPattern, toSet.Usage);

		return result;
	}

	ID3D11ShaderResourceView* BufferManagerD11::CreateSRV(ID3D11Buffer* buffer, u32 nrOfElements)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = nrOfElements;

		auto device = deviceAndContext->GetD3D11Device();
		ID3D11ShaderResourceView* toReturn;
		device->CreateShaderResourceView(buffer, &desc, &toReturn) >> hrVerify;

		return toReturn;
	}

	ResourceIndex BufferManagerD11::AddBuffer(void* data,
		u32 elementSize, u32 nrOfElements,
		PerFrameUsage rwPattern, u32 bindingFlags)
	{
		D3D11_BUFFER_DESC desc;
		bool result = CreateDescription(elementSize, nrOfElements, rwPattern, bindingFlags, desc);

		if (result == false)
			return ResourceIndex(-1);

		ID3D11Buffer* interfacePtr = nullptr;

		auto device = deviceAndContext->GetD3D11Device();
		if (data != nullptr)
		{
			D3D11_SUBRESOURCE_DATA resourceData = {};
			resourceData.pSysMem = data;
			resourceData.SysMemPitch = resourceData.SysMemSlicePitch = 0;

			device->CreateBuffer(&desc, &resourceData, &interfacePtr) >> hrVerify;
		}
		else
		{
			device->CreateBuffer(&desc, nullptr, &interfacePtr) >> hrVerify;
		}


		ID3D11ShaderResourceView* srv = nullptr;
		if (bindingFlags & BufferBinding::STRUCTURED_BUFFER)
		{
			srv = CreateSRV(interfacePtr, nrOfElements);

			if (srv == nullptr)
			{
				interfacePtr->Release();
				return ResourceIndex(-1);
			}
		}

		buffers.push_back({ interfacePtr, elementSize, nrOfElements, srv });
		return ResourceIndex(buffers.size() - 1);
	}

	void BufferManagerD11::UpdateBuffer(ResourceIndex index, void* data)
	{
		StoredBuffer& toUpdate = buffers[index];
		auto context = deviceAndContext->GetD3D11DeviceContext();

		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		context->Map(toUpdate.interfacePtr, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		size_t dataSize = toUpdate.elementSize * toUpdate.elementCount;
		memcpy(mappedBuffer.pData, data, dataSize);
		context->Unmap(toUpdate.interfacePtr, 0);
	}

	u32 BufferManagerD11::GetElementSize(ResourceIndex index)
	{
		return buffers[index].elementSize;
	}

	u32 BufferManagerD11::GetElementCount(ResourceIndex index)
	{
		return buffers[index].elementCount;
	}

	ID3D11Buffer* BufferManagerD11::GetBufferInterface(ResourceIndex index)
	{
		return buffers[index].interfacePtr;
	}

	ID3D11ShaderResourceView* BufferManagerD11::GetSRV(ResourceIndex index)
	{
		return buffers[index].srv;
	}
}
