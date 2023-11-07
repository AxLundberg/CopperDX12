#include "RendererD11.h"
#include "../cmn/GraphicsError.h"

#include <Core/thirdParty/ImGUI/imgui.h>
#include "Core/thirdParty/ImGUI/backends/imgui_impl_win32.h"
#include "Core/thirdParty/ImGUI/backends/imgui_impl_dx11.h"
#include <stdexcept>


namespace CPR::GFX::D11
{
	RendererD11::RendererD11(HWND windowHandle)
	{
		CreateBasicInterfaces(windowHandle);
		CreateRenderTargetView();
		CreateDepthStencil();
		CreateViewport();
		bufferManager.Initialise(device, immediateContext);
		textureManager.Initialise(device);
		samplerManager.Initialise(device);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(windowHandle);
		ImGui_ImplDX11_Init(device.Get(), immediateContext.Get());
	}

	RendererD11::~RendererD11()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		delete(currentRenderPass);
		delete(currentCamera);
	}

	/*void RendererD11::DestroyGraphicsRenderPass(GfxRenderPassD11* pass)
	{
		delete pass;
	}*/

	void RendererD11::CreateBasicInterfaces(HWND windowHandle)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = windowHandle;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
			nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION,
			&swapChainDesc, &swapChain, &device, nullptr, &immediateContext) >> hrVerify;
	}

	void RendererD11::CreateRenderTargetView()
	{
		ID3D11Texture2D* backBuffer = nullptr;
		if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(&backBuffer))))
		{
			throw std::runtime_error("Could not fetch backbuffer from swap chain");
		}

		HRESULT hr = device->CreateRenderTargetView(backBuffer, NULL, backBufferRTV.GetAddressOf());
		if (FAILED(hr))
			throw std::runtime_error("Could not create rtv for backbuffer");

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
		ID3D11ShaderResourceView* srv = bufferManager.GetSRV(bufferIndex);

		switch (stage)
		{
		case PipelineShaderStage::VS:
			immediateContext->VSSetShaderResources(slot, 1, &srv);
			break;
		case PipelineShaderStage::PS:
			immediateContext->PSSetShaderResources(slot, 1, &srv);
			break;
		default:
			throw std::runtime_error("Incorrect shader stage for binding structured buffer");
		}
	}

	void RendererD11::BindConstantBuffer(ResourceIndex bufferIndex,
		PipelineShaderStage stage, std::uint8_t slot)
	{
		ID3D11Buffer* buffer = bufferManager.GetBufferInterface(bufferIndex);

		switch (stage)
		{
		case PipelineShaderStage::VS:
			immediateContext->VSSetConstantBuffers(slot, 1, &buffer);
			break;
		case PipelineShaderStage::PS:
			immediateContext->PSSetConstantBuffers(slot, 1, &buffer);
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
		ID3D11ShaderResourceView* srv = textureManager.GetSRV(textureIndex);

		switch (stage)
		{
		case PipelineShaderStage::VS:
			immediateContext->VSSetShaderResources(slot, 1, &srv);
			break;
		case PipelineShaderStage::PS:
			immediateContext->PSSetShaderResources(slot, 1, &srv);
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
		ID3D11SamplerState* sampler = samplerManager.GetSampler(index);
		D3D11_SAMPLER_DESC temp;
		sampler->GetDesc(&temp);
		switch (binding.shaderStage)
		{
		case PipelineShaderStage::VS:
			immediateContext->VSSetSamplers(slot, 1, &sampler);
			break;
		case PipelineShaderStage::PS:
			immediateContext->PSSetSamplers(slot, 1, &sampler);
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
			BindBuffer(currentCamera->GetVP(bufferManager), binding);
			break;
		case PipelineDataType::CAMERA_POS:
			BindBuffer(currentCamera->GetPosition(bufferManager), binding);
			break;
		case PipelineDataType::LIGHT:
			BindBuffer(lightBufferIndex, binding);
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
		return samplerManager.CreateSampler(type, adressMode);
	}

	GfxRenderPassD11* RendererD11::CreateRenderPass(RenderPassInfo& intialisationInfo)
	{
		currentRenderPass = new GfxRenderPassD11(device.Get(), intialisationInfo);
		return currentRenderPass;
	}

	ResourceIndex RendererD11::SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage rwPattern, BufferBinding binding)
	{
		return bufferManager.AddBuffer(data, elemSize, elemCount, rwPattern, binding);
	}

	ResourceIndex RendererD11::SubmitTexture(void* data, TextureInfo& info)
	{
		return textureManager.AddTexture(data, info);
	}

	CameraD11* RendererD11::CreateCamera(f32 minDepth, f32 maxDepth, f32 aspectRatio)
	{
		currentCamera = new CameraD11(bufferManager, minDepth, maxDepth, aspectRatio);
		return currentCamera;
	}

	void RendererD11::UpdateBuffer(ResourceIndex bufferIndex, void* data)
	{
		bufferManager.UpdateBuffer(bufferIndex, data);
	}

	void RendererD11::SetRenderPass(GfxRenderPassD11* toSet)
	{
		currentRenderPass = static_cast<GfxRenderPassD11*>(toSet);
	}

	void RendererD11::SetLightBuffer(ResourceIndex lightBufferIndexToUse)
	{
		lightBufferIndex = lightBufferIndexToUse;
	}

	void RendererD11::PreRender()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Hello, world!");
		ImGui::Text("This is some useful text.");
		static i32 counter = 0;
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		ImGui::End();
		ImGui::Render();
		float clearColour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		immediateContext->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
		immediateContext->ClearRenderTargetView(backBufferRTV.Get(), clearColour);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		immediateContext->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		immediateContext->RSSetViewports(1, &viewport);
	}

	void RendererD11::Render(const std::vector<RenderObject>& objectsToRender)
	{
		
		currentRenderPass->SetShaders(immediateContext.Get());
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
				drawCount = bufferManager.GetElementCount(mesh.indexBuffer);
			else
				drawCount = bufferManager.GetElementCount(mesh.vertexBuffer);

			immediateContext->Draw(drawCount, 0);
		}
	}

	void RendererD11::Present()
	{
		swapChain->Present(0, 0);
	}
}