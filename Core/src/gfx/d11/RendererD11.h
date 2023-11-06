//#pragma once
//
//#include <d3d11_4.h>
//#include <dxgi1_6.h>
//
//#include "RenderPassD11.h"
//#include "BufferManagerD11.h"
//#include "TextureManagerD11.h"
//#include "SamplerManagerD11.h"
//#include "CameraD11.h"
//
//namespace CPR::GFX::D11
//{
//	class RendererD11 : public IRendererD11
//	{
//	private:
//		unsigned int backBufferWidth = 0;
//		unsigned int backBufferHeight = 0;
//
//		ID3D11Device* device = nullptr;
//		ID3D11DeviceContext* immediateContext = nullptr;
//		IDXGISwapChain* swapChain = nullptr;
//		ID3D11RenderTargetView* backBufferRTV = nullptr;
//		ID3D11Texture2D* depthBuffer = nullptr;
//		ID3D11DepthStencilView* depthBufferDSV = nullptr;
//		D3D11_VIEWPORT viewport;
//
//		BufferManagerD11 bufferManager;
//		TextureManagerD11 textureManager;
//		SamplerManagerD11 samplerManager;
//
//		GraphicsRenderPassD11* currentRenderPass = nullptr;
//		CameraD11* currentCamera = nullptr;
//		ResourceIndex lightBufferIndex = ResourceIndex(-1);
//
//		void CreateBasicInterfaces(HWND windowHandle);
//		void CreateRenderTargetView();
//		void CreateDepthStencil();
//		void CreateViewport();
//
//		void BindStructuredBuffer(ResourceIndex bufferIndex,
//			PipelineShaderStage stage, std::uint8_t slot);
//		void BindConstantBuffer(ResourceIndex bufferIndex,
//			PipelineShaderStage stage, std::uint8_t slot);
//		void BindBuffer(ResourceIndex bufferIndex, const PipelineBinding& binding);
//
//		void BindTextureSRV(ResourceIndex textureIndex,
//			PipelineShaderStage stage, std::uint8_t slot);
//		void BindTexture(ResourceIndex textureIndex, const PipelineBinding& binding);
//
//		void BindSampler(ResourceIndex index, const PipelineBinding& binding);
//
//		void HandleBinding(const RenderObject& object,
//			const PipelineBinding& binding);
//
//		void HandleBinding(const PipelineBinding& binding);
//
//	public:
//		RendererD11(HWND windowHandle);
//		virtual ~RendererD11();
//
//		GraphicsRenderPass* CreateGraphicsRenderPass(const GraphicsRenderPassInfo& intialisationInfo) override;
//		void DestroyGraphicsRenderPass(GraphicsRenderPass* pass) override;
//
//		Camera* CreateCamera(float minDepth, float maxDepth, float aspectRatio) override;
//		void DestroyCamera(Camera* camera) override;
//
//		void SetRenderPass(GraphicsRenderPass* toSet) override;
//		void SetCamera(Camera* toSet) override;
//		void SetLightBuffer(ResourceIndex lightBufferIndexToUse) override;
//
//		void PreRender() override;
//		void Render(const std::vector<RenderObject>& objectsToRender) override;
//		void Present() override;
//	};
//}
