#include "BufferManagerD11.h"

namespace CPR::GFX::D11
{
	bool BufferManagerD11::DetermineUsage(PerFrameUsage rwPattern, D3D11_USAGE& usage)
	{
		usage = rwPattern == PerFrameUsage::STATIC ?
			D3D11_USAGE_IMMUTABLE :
			D3D11_USAGE_DYNAMIC;

		return true;
	}

	UINT BufferManagerD11::TranslateBindFlags(unsigned int bindingFlags)
	{
		UINT toReturn = 0;

		if (bindingFlags & BufferBinding::CONSTANT_BUFFER)
			toReturn |= D3D11_BIND_CONSTANT_BUFFER;
		if (bindingFlags & BufferBinding::STRUCTURED_BUFFER)
			toReturn |= D3D11_BIND_SHADER_RESOURCE;

		return toReturn;
	}

	bool BufferManagerD11::CreateDescription(unsigned int elementSize,
		unsigned int nrOfElements, PerFrameUsage rwPattern, unsigned int bindingFlags,
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

	ID3D11ShaderResourceView* BufferManagerD11::CreateSRV(
		ID3D11Buffer* buffer, unsigned int nrOfElements)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = nrOfElements;

		ID3D11ShaderResourceView* toReturn;
		HRESULT hr = device->CreateShaderResourceView(buffer,
			&desc, &toReturn);

		if (FAILED(hr))
			toReturn = nullptr;

		return toReturn;
	}

	BufferManagerD11::BufferManagerD11()
	{
	}

	BufferManagerD11::~BufferManagerD11()
	{
		for (auto& buffer : buffers)
		{
			buffer.interfacePtr->Release();
			if (buffer.srv != nullptr)
				buffer.srv->Release();
		}
	}

	void BufferManagerD11::Initialise(ID3D11Device* deviceToUse,
		ID3D11DeviceContext* contextToUse)
	{
		device = deviceToUse;
		context = contextToUse;
	}

	ResourceIndex BufferManagerD11::AddBuffer(void* data,
		unsigned int elementSize, unsigned int nrOfElements,
		PerFrameUsage rwPattern, unsigned int bindingFlags)
	{
		D3D11_BUFFER_DESC desc;
		bool result = CreateDescription(elementSize, nrOfElements,
			rwPattern, bindingFlags, desc);

		if (result == false)
			return ResourceIndex(-1);

		HRESULT hr = S_OK;
		ID3D11Buffer* interfacePtr = nullptr;

		if (data != nullptr)
		{
			D3D11_SUBRESOURCE_DATA resourceData;
			resourceData.pSysMem = data;
			resourceData.SysMemPitch = resourceData.SysMemSlicePitch = 0;

			hr = device->CreateBuffer(&desc, &resourceData, &interfacePtr);
		}
		else
		{
			hr = device->CreateBuffer(&desc, nullptr, &interfacePtr);
		}

		if (FAILED(hr))
			return ResourceIndex(-1);

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

		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		context->Map(toUpdate.interfacePtr, 0,
			D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		size_t dataSize = toUpdate.elementSize * toUpdate.elementCount;
		memcpy(mappedBuffer.pData, data, dataSize);
		context->Unmap(toUpdate.interfacePtr, 0);
	}

	unsigned int BufferManagerD11::GetElementSize(ResourceIndex index)
	{
		return buffers[index].elementSize;
	}

	unsigned int BufferManagerD11::GetElementCount(ResourceIndex index)
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
