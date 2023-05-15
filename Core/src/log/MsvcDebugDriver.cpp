#include "MsvcDebugDriver.h"
#include "TextFormatter.h"
#include <windows.h>


namespace CPR::LOG
{
	MsvcDebugDriver::MsvcDebugDriver(std::shared_ptr<ITextFormatter> formatter)
		:
		mFormatter{ std::move(formatter) }
	{}

	void MsvcDebugDriver::Submit(const Entry& e)
	{
		if (mFormatter)
		{
			OutputDebugStringW(mFormatter->Format(e).c_str());
		}
		// How to log from LogSystem?
	}

	void MsvcDebugDriver::SetFormatter(std::shared_ptr<ITextFormatter> formatter)
	{
		mFormatter = std::move(formatter);
	}
}
