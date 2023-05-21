#include "SimpleFileDriver.h"
#include "TextFormatter.h"

namespace CPR::LOG
{
	SimpleFileDriver::SimpleFileDriver(std::filesystem::path path, std::shared_ptr<ITextFormatter> formatter)
		:
		mFormatter{ std::move(formatter) }
	{
		// create directories
		std::filesystem::create_directories(path.parent_path());
		// open file, append if exists
		mFile.open(path, mFile.out | mFile.app);
	}

	void SimpleFileDriver::Submit(const Entry& e)
	{
		if (mFormatter)
			mFile << mFormatter->Format(e).c_str();
	}

	void SimpleFileDriver::SetFormatter(std::shared_ptr<ITextFormatter> formatter)
	{
		mFormatter = std::move(formatter);
	}
}
