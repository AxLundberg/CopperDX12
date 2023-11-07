#pragma once
#include <array>
#include <vector>
#include <Core/src/utl/String.h>

#include "cmn/D11Headers.h"

namespace CPR::GFX::D11
{
	enum class PipelineDataType
	{
		NONE,
		TRANSFORM,
		VIEW_PROJECTION,
		CAMERA_POS,
		LIGHT,
		VERTEX,
		INDEX,
		DIFFUSE,
		SPECULAR,
		SAMPLER
	};

	enum class PipelineBindingType
	{
		NONE,
		CONSTANT_BUFFER,
		SHADER_RESOURCE,
		UNORDERED_ACCESS
	};

	enum class PipelineShaderStage
	{
		NONE,
		VS,
		PS
	};

	struct PipelineBinding
	{
		PipelineDataType dataType = PipelineDataType::NONE;
		PipelineBindingType bindingType = PipelineBindingType::NONE;
		PipelineShaderStage shaderStage = PipelineShaderStage::NONE;
		std::uint8_t slotToBindTo = std::uint8_t(-1);
	};

	struct RenderPassInfo
	{
		std::string vsPath = "";
		std::string psPath = "";
		std::vector<PipelineBinding> objectBindings;
		std::vector<PipelineBinding> globalBindings;
	};

	class GfxRenderPassD11
	{
	public:
		GfxRenderPassD11(ComPtr<ID3D11Device> device, const RenderPassInfo& info);
		~GfxRenderPassD11();
		void SetShaders(ID3D11DeviceContext* deviceContext);
		const std::vector<PipelineBinding>& GetObjectBindings();
		const std::vector<PipelineBinding>& GetGlobalBindings();

		void SetGlobalSampler(PipelineShaderStage shader, std::uint8_t slot, ResourceIndex index);
		ResourceIndex GetGlobalSampler(PipelineShaderStage shader, std::uint8_t slot) const;

	private:
		ComPtr<ID3D11Device> device = nullptr;
		std::vector<PipelineBinding> objectBindings;
		std::vector<PipelineBinding> globalBindings;
		ComPtr<ID3D11VertexShader> vertexShader = nullptr;
		ComPtr<ID3D11PixelShader> pixelShader = nullptr;

		std::array<ResourceIndex, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> vsGlobalSamplers;
		std::array<ResourceIndex, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> psGlobalSamplers;

		ComPtr<ID3DBlob> LoadCSO(const std::string& filepath);
	};
}
