#include "Renderer.h"
#include "utl/Helpers.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	Renderer::Renderer()
	{
		// Initialize factory, adapter and device
		{
			u32 factoryFlags = _DEBUG_ ? DXGI_CREATE_FACTORY_DEBUG : 0;
			CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&_factory)) >> hrVerify;

			_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter)) >> hrVerify;

			D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)) >> hrVerify;
			u32 adapterIndex = 0;
		}
	}
	ResourceIndex Renderer::CreateSampler(SamplerType, AddressMode)
	{
		return ResourceIndex();
	}
	void Renderer::CreateRenderPass(RenderPassInfo&)
	{
	}
	ResourceIndex Renderer::SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding)
	{
		return ResourceIndex();
	}
	ResourceIndex Renderer::SubmitTexture(void* data, TextureInfo&)
	{
		return ResourceIndex();
	}
	void Renderer::PreRender()
	{
	}
	void Renderer::Render()
	{
	}
	void Renderer::Present()
	{
	}
	Renderer::~Renderer()
	{
	}
}
