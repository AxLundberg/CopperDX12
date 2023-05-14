#pragma once
#include "Driver.h"
#include <memory>

namespace CPR::LOG
{
	class MsvcDebugDriver : public ITextDriver
	{
	public:
		MsvcDebugDriver(std::unique_ptr<ITextFormatter> formatter = {});
		void Submit(const Entry&) override;
		void SetFormatter(std::unique_ptr<ITextFormatter> formatter) override;
	private:
		std::unique_ptr<ITextFormatter> mFormatter;
	};
}
