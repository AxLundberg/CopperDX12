#pragma once
#include "Driver.h"
#include <memory>

namespace CPR::LOG
{
	class MsvcDebugDriver : public ITextDriver
	{
	public:
		MsvcDebugDriver(std::shared_ptr<ITextFormatter> formatter = {});
		void Submit(const Entry&) override;
		void SetFormatter(std::shared_ptr<ITextFormatter> formatter) override;
	private:
		std::shared_ptr<ITextFormatter> mFormatter;
	};
}
