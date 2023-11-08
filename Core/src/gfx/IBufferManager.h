#pragma once
#include "cmn/TypeDefs.h"

namespace CPR::GFX
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

	class IBufferManager
	{
	public:
		virtual ~IBufferManager() = default;

	private:

	};
}