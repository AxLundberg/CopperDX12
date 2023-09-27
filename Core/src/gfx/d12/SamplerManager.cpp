#include "SamplerManager.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	SamplerManager::SamplerManager(ComPtr<ID3D12Device5> const& device) : _device(device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		heapDesc.NumDescriptors = HEAP_SIZE;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0u;

		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_samplerHeap)) >> hrVerify;
	}

	SamplerManager::~SamplerManager()
	{
		if (_samplerHeap != nullptr)
			_samplerHeap->Release();
	}


	ResourceIndex SamplerManager::CreateSampler(SamplerType samplerType, AddressMode adressMode)
	{
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MinLOD = 0u;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = samplerType == SamplerType::ANISOTROPIC ? 16 : 1;

		if (adressMode == AddressMode::WRAP) samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		else if (adressMode == AddressMode::CLAMP) samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		else if (adressMode == AddressMode::BLACK_BORDER_COLOUR) samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;

		samplerDesc.AddressV = samplerDesc.AddressW = samplerDesc.AddressU;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = _samplerHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<u64>(_currentSize) * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		_device->CreateSampler(&samplerDesc, handle);

		return _currentSize++;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE SamplerManager::GetDescriptorHandle(ResourceIndex idx)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = _samplerHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		return handle;
	}

	void SamplerManager::SetSamplerHeap(ID3D12GraphicsCommandList*& cmdList)
	{
		cmdList->SetDescriptorHeaps(1u, &_samplerHeap);
	}
}