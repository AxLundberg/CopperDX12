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

	struct BufferInfo
	{
		u32 elementSize;
		u32 nrOfElements;
		PerFrameUsage rwPattern;
		u32 bindingFlags;
	};

	class IBufferManager
	{
	public:
		virtual ~IBufferManager() = default;
		virtual ResourceIndex AddBuffer(void* data, const BufferInfo&) = 0;
		virtual void UpdateBuffer(ResourceIndex index, void* data) = 0;
		virtual u32 GetElementSize(ResourceIndex index) = 0;
		virtual u32 GetElementCount(ResourceIndex index) = 0;
	};
}