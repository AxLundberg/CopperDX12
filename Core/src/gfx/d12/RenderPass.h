#pragma once
#include <Core/src/utl/String.h>
#include <vector>

namespace CPR::GFX::D12
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
}
