#include "RendererD11.h"
#include "../cmn/GraphicsError.h"

#include <Core/thirdParty/ImGUI/imguiIncludes.h>
#include <stdexcept>


namespace CPR::GFX::D11
{
	RendererD11::RendererD11(HWND windowHandle,
		std::shared_ptr<IDevice> dev,
		std::shared_ptr<IBufferManager> bufferManager,
		std::shared_ptr<ISamplerManager> samplerManager,
		std::shared_ptr<ITextureManager> textureManager)
		:
		deviceSwapchainAndContext(std::move(dev)),
		bufferManager(std::move(bufferManager)),
		samplerManager(std::move(samplerManager)),
		textureManager(std::move(textureManager))
	{
		auto device = deviceSwapchainAndContext->GetD3D11Device();
		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		//CreateBasicInterfaces(windowHandle);
		CreateRenderTargetView();
		CreateDepthStencil();
		CreateViewport();
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(windowHandle);
		ImGui_ImplDX11_Init(device.Get(), context.Get());
	}

	RendererD11::~RendererD11()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		delete(currentRenderPass);
		delete(currentCamera);
	}

	void RendererD11::CreateRenderTargetView()
	{
		ID3D11Texture2D* backBuffer = nullptr;
		auto sc = deviceSwapchainAndContext->GetD3D11SwapChain();
		if (FAILED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(&backBuffer))))
		{
			throw std::runtime_error("Could not fetch backbuffer from swap chain");
		}

		auto device = deviceSwapchainAndContext->GetD3D11Device();
		device->CreateRenderTargetView(backBuffer, NULL, backBufferRTV.GetAddressOf()) >> hrVerify;

		D3D11_TEXTURE2D_DESC desc;
		backBuffer->GetDesc(&desc);
		backBufferWidth = desc.Width;
		backBufferHeight = desc.Height;
		backBuffer->Release();
	}

	void RendererD11::CreateDepthStencil()
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = backBufferWidth;
		desc.Height = backBufferHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		auto device = deviceSwapchainAndContext->GetD3D11Device();
		if (FAILED(device->CreateTexture2D(&desc, nullptr, depthBuffer.GetAddressOf())))
			throw std::runtime_error("Failed to create depth stencil texture!");

		if (FAILED(device->CreateDepthStencilView(depthBuffer.Get(), 0, depthBufferDSV.GetAddressOf())))
			throw std::runtime_error("Failed to create dsv!");
	}

	void RendererD11::CreateViewport()
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.Width = static_cast<float>(backBufferWidth);
		viewport.Height = static_cast<float>(backBufferHeight);
	}

	void RendererD11::BindStructuredBuffer(ResourceIndex bufferIndex,
		PipelineShaderStage stage, std::uint8_t slot)
	{
		ID3D11ShaderResourceView* srv = bufferManager->GetSRV(bufferIndex);

		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		switch (stage)
		{
		case PipelineShaderStage::VS:
			context->VSSetShaderResources(slot, 1, &srv);
			break;
		case PipelineShaderStage::PS:
			context->PSSetShaderResources(slot, 1, &srv);
			break;
		default:
			throw std::runtime_error("Incorrect shader stage for binding structured buffer");
		}
	}

	void RendererD11::BindConstantBuffer(ResourceIndex bufferIndex,
		PipelineShaderStage stage, std::uint8_t slot)
	{
		ID3D11Buffer* buffer = bufferManager->GetBufferInterface(bufferIndex);

		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		switch (stage)
		{
		case PipelineShaderStage::VS:
			context->VSSetConstantBuffers(slot, 1, &buffer);
			break;
		case PipelineShaderStage::PS:
			context->PSSetConstantBuffers(slot, 1, &buffer);
			break;
		default:
			throw std::runtime_error("Incorrect shader stage for binding constant buffer");
		}
	}

	void RendererD11::BindBuffer(ResourceIndex bufferIndex, const PipelineBinding& binding)
	{
		switch (binding.bindingType)
		{
		case PipelineBindingType::SHADER_RESOURCE:
			BindStructuredBuffer(bufferIndex, binding.shaderStage, binding.slotToBindTo);
			break;
		case PipelineBindingType::CONSTANT_BUFFER:
			BindConstantBuffer(bufferIndex, binding.shaderStage, binding.slotToBindTo);
			break;
		default:
			throw std::runtime_error("Incorrect bind type for binding buffer");
		}
	}

	void RendererD11::BindTextureSRV(ResourceIndex textureIndex,
		PipelineShaderStage stage, std::uint8_t slot)
	{
		ID3D11ShaderResourceView* srv = textureManager->GetSRV(textureIndex);

		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		switch (stage)
		{
		case PipelineShaderStage::VS:
			context->VSSetShaderResources(slot, 1, &srv);
			break;
		case PipelineShaderStage::PS:
			context->PSSetShaderResources(slot, 1, &srv);
			break;
		default:
			throw std::runtime_error("Incorrect shader stage for binding texture srv");
		}
	}

	void RendererD11::BindTexture(ResourceIndex textureIndex, const PipelineBinding& binding)
	{
		switch (binding.bindingType)
		{
		case PipelineBindingType::SHADER_RESOURCE:
			BindTextureSRV(textureIndex, binding.shaderStage, binding.slotToBindTo);
			break;
		default:
			throw std::runtime_error("Incorrect bind type for binding texture");
		}
	}

	void RendererD11::BindSampler(ResourceIndex index,
		const PipelineBinding& binding)
	{
		std::uint8_t slot = binding.slotToBindTo;
		ID3D11SamplerState* sampler = samplerManager->GetSampler(index);
		D3D11_SAMPLER_DESC temp;
		sampler->GetDesc(&temp);

		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		switch (binding.shaderStage)
		{
		case PipelineShaderStage::VS:
			context->VSSetSamplers(slot, 1, &sampler);
			break;
		case PipelineShaderStage::PS:
			context->PSSetSamplers(slot, 1, &sampler);
			break;
		default:
			throw std::runtime_error("Incorrect shader stage for binding sampler");
		}
	}

	void RendererD11::HandleBinding(const RenderObject& object, const PipelineBinding& binding)
	{
		switch (binding.dataType)
		{
		case PipelineDataType::TRANSFORM:
			BindBuffer(object.transformBuffer, binding);
			break;
		case PipelineDataType::VERTEX:
			BindBuffer(object.mesh.vertexBuffer, binding);
			break;
		case PipelineDataType::INDEX:
			BindBuffer(object.mesh.indexBuffer, binding);
			break;
		case PipelineDataType::DIFFUSE:
			BindTexture(object.surfaceProperty.diffuseTexture, binding);
			break;
		case PipelineDataType::SPECULAR:
			BindTexture(object.surfaceProperty.specularTexture, binding);
			break;
		case PipelineDataType::SAMPLER:
			BindSampler(object.surfaceProperty.sampler, binding);
			break;
		default:
			throw std::runtime_error("Unknown data type for object binding");
		}
	}

	void RendererD11::HandleBinding(const PipelineBinding& binding)
	{
		switch (binding.dataType)
		{
		case PipelineDataType::VIEW_PROJECTION:
		{
			auto vp = currentCamera->GetVP();
			auto vpBufferIndex = currentCamera->GetVPBufferIndex();
			bufferManager->UpdateBuffer(vpBufferIndex, &vp);
			BindBuffer(vpBufferIndex, binding);
			break;
		}
		case PipelineDataType::CAMERA_POS:
		{
			auto pos = currentCamera->GetPosition();
			auto posBufferIdx = currentCamera->GetPositionBufferIndex();
			bufferManager->UpdateBuffer(posBufferIdx, &pos);
			BindBuffer(posBufferIdx, binding);
			break;
		}
		case PipelineDataType::LIGHT:
			BindBuffer(lightBufferIndex, binding);
			break;
		case PipelineDataType::IMGUI:
			/*auto test = currentCamera->GetPosition();
			bufferManager->UpdateBuffer(imguiBufferIndex, &test);*/
			BindBuffer(imguiBufferIndex, binding);
			break;
		case PipelineDataType::SAMPLER:
			BindSampler(currentRenderPass->GetGlobalSampler(
				binding.shaderStage, binding.slotToBindTo), binding);
			break;
		default:
			throw std::runtime_error("Unknown data type for global binding");
		}
	}

	ResourceIndex RendererD11::CreateSampler(SamplerType type, AddressMode adressMode)
	{
		return samplerManager->CreateSampler(type, adressMode);
	}

	GfxRenderPassD11* RendererD11::CreateRenderPass(RenderPassInfo& intialisationInfo)
	{
		auto device = deviceSwapchainAndContext->GetD3D11Device();
		currentRenderPass = new GfxRenderPassD11(device.Get(), intialisationInfo);
		return currentRenderPass;
	}

	ResourceIndex RendererD11::SubmitBuffer(void* data, const BufferInfo& info)
	{
		return bufferManager->AddBuffer(data, info);
	}

	ResourceIndex RendererD11::SubmitTexture(void* data, TextureInfo& info)
	{
		return textureManager->AddTexture(data, info);
	}

	CameraD11* RendererD11::CreateCamera(f32 width, f32 height)
	{
		currentCamera = new CameraD11(bufferManager, width, height);
		return currentCamera;
	}

	CameraD11* RendererD11::CreateCamera(f32 minDepth, f32 maxDepth, f32 aspectRatio)
	{
		currentCamera = new CameraD11(bufferManager, minDepth, maxDepth, aspectRatio);
		return currentCamera;
	}

	void RendererD11::UpdateBuffer(ResourceIndex bufferIndex, void* data)
	{
		bufferManager->UpdateBuffer(bufferIndex, data);
	}

	void RendererD11::SetRenderPass(GfxRenderPassD11* toSet)
	{
		currentRenderPass = static_cast<GfxRenderPassD11*>(toSet);
	}

	void RendererD11::SetLightBuffer(ResourceIndex lightBufferIndexToUse)
	{
		lightBufferIndex = lightBufferIndexToUse;
	}
	void RendererD11::SetImguiBuffer(ResourceIndex imguiBufferIndexToUse)
	{
		imguiBufferIndex = imguiBufferIndexToUse;
	}
	void RendererD11::PreRender()
	{
		float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
		context->ClearRenderTargetView(backBufferRTV.Get(), clearColour);
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->RSSetViewports(1, &viewport);
	}

	void RendererD11::Render(const std::vector<RenderObject>& objectsToRender)
	{
		
		auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
		currentRenderPass->SetShaders(context.Get());
		const std::vector<PipelineBinding>& objectBindings = currentRenderPass->GetObjectBindings();
		const std::vector<PipelineBinding>& globalBindings = currentRenderPass->GetGlobalBindings();

		for (auto& binding : globalBindings)
			HandleBinding(binding);

		for (auto& object : objectsToRender)
		{
			for (auto& binding : objectBindings)
				HandleBinding(object, binding);

			const Mesh& mesh = object.mesh;
			unsigned int drawCount = 0;
			if (mesh.indexBuffer != ResourceIndex(-1))
				drawCount = bufferManager->GetElementCount(mesh.indexBuffer);
			else
				drawCount = bufferManager->GetElementCount(mesh.vertexBuffer);

			auto context = deviceSwapchainAndContext->GetD3D11DeviceContext();
			context->Draw(drawCount, 0);
		}
	}

	void RendererD11::Present()
	{
		auto swapChain = deviceSwapchainAndContext->GetD3D11SwapChain();
		swapChain->Present(0, 0);
	}
}