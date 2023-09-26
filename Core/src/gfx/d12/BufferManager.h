#pragma once

namespace CPR::GFX::D12
{
	enum class PerFrameUsage
	{
		STATIC,
		DYNAMIC
	};

	enum BufferBinding
	{
		STRUCTURED_BUFFER = 1,
		CONSTANT_BUFFER = 2
	};
}