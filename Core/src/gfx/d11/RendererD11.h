#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>

#include "../RenderObject.h"
#include "RenderPassD11.h"
#include "BufferManagerD11.h"
#include "TextureManagerD11.h"
#include "SamplerManagerD11.h"
#include "CameraD11.h"
#include "DeviceD11.h"

namespace CPR::GFX::D11
{
	class IRendererD11
	{
	public:
		struct IocParams
		{
			HWND hWnd;
		};
	public:
		virtual ~IRendererD11() = default;

		virtual ResourceIndex CreateSampler(SamplerType, AddressMode) = 0;
		virtual GfxRenderPassD11* CreateRenderPass(RenderPassInfo&) = 0;
		virtual ResourceIndex SubmitBuffer(void* data, const BufferInfo&) = 0;
		virtual ResourceIndex SubmitTexture(void* data, TextureInfo&) = 0;
		virtual CameraD11* CreateCamera(f32 minDepth, f32 maxDepth, f32 aspectRatio) = 0;
		virtual void UpdateBuffer(ResourceIndex bufferIndex, void* data) = 0;
		virtual void SetLightBuffer(ResourceIndex lightBufferIndexToUse) = 0;
		virtual void SetImguiBuffer(ResourceIndex imguiBufferIndexToUse) = 0;

		virtual void SetRenderPass(GfxRenderPassD11* toSet) = 0;
		virtual void PreRender() = 0;
		virtual void Render(const std::vector<RenderObject>& objectsToRender) = 0;
		virtual void Present() = 0;
	};

	class RendererD11 : public IRendererD11
	{
	public:
		RendererD11(HWND windowHandle,
			std::shared_ptr<IDevice> device,
			std::shared_ptr<IBufferManager> bufferManager,
			std::shared_ptr<ISamplerManager> samplerManager,
			std::shared_ptr<ITextureManager> textureManager
		);
		~RendererD11();

		ResourceIndex CreateSampler(SamplerType, AddressMode) override;
		GfxRenderPassD11* CreateRenderPass(RenderPassInfo&) override;
		ResourceIndex SubmitBuffer(void* data, const BufferInfo&) override;
		ResourceIndex SubmitTexture(void* data, TextureInfo&) override;
		CameraD11* CreateCamera(f32 minDepth, f32 maxDepth, f32 aspectRatio) override;
		void UpdateBuffer(ResourceIndex bufferIndex, void* data) override;
		void SetLightBuffer(ResourceIndex lightBufferIndexToUse) override;
		void SetImguiBuffer(ResourceIndex imguitBufferIndexToUse) override;
		//void DestroyGraphicsRenderPass(GfxRenderPassD11* pass) override;
		void SetRenderPass(GfxRenderPassD11* toSet) override;
		void PreRender() override;
		void Render(const std::vector<RenderObject>& objectsToRender) override;
		void Present() override;

	private:
		void CreateRenderTargetView();
		void CreateDepthStencil();
		void CreateViewport();

		void BindStructuredBuffer(ResourceIndex bufferIndex,
			PipelineShaderStage stage, std::uint8_t slot);
		void BindConstantBuffer(ResourceIndex bufferIndex,
			PipelineShaderStage stage, std::uint8_t slot);
		void BindBuffer(ResourceIndex bufferIndex, const PipelineBinding& binding);

		void BindTextureSRV(ResourceIndex textureIndex,
			PipelineShaderStage stage, std::uint8_t slot);
		void BindTexture(ResourceIndex textureIndex, const PipelineBinding& binding);

		void BindSampler(ResourceIndex index, const PipelineBinding& binding);

		void HandleBinding(const RenderObject& object,
			const PipelineBinding& binding);

		void HandleBinding(const PipelineBinding& binding);

	private:
		unsigned int backBufferWidth = 0;
		unsigned int backBufferHeight = 0;

		std::shared_ptr<IDevice> deviceSwapchainAndContext;
		
		D3D11_VIEWPORT viewport;
		ComPtr<ID3D11Texture2D> depthBuffer = nullptr;
		ComPtr<ID3D11RenderTargetView> backBufferRTV = nullptr;
		ComPtr<ID3D11DepthStencilView> depthBufferDSV = nullptr;

		std::shared_ptr<IBufferManager> bufferManager;
		std::shared_ptr<ISamplerManager> samplerManager;
		std::shared_ptr<ITextureManager> textureManager;

		GfxRenderPassD11* currentRenderPass = nullptr;
		CameraD11* currentCamera = nullptr;
		ResourceIndex lightBufferIndex = ResourceIndex(-1);
		ResourceIndex imguiBufferIndex = ResourceIndex(-1);
	};
}
