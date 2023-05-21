#pragma once
#include "Driver.h"
#include <memory>
#include <fstream>
#include <filesystem>

namespace CPR::LOG
{
	class ISimpleFileDriver : public ITextDriver {};

	class SimpleFileDriver : public ISimpleFileDriver
	{
	public:
		SimpleFileDriver(std::filesystem::path path, std::shared_ptr<ITextFormatter> formatter = {});
		void Submit(const Entry&) override;
		void SetFormatter(std::shared_ptr<ITextFormatter> formatter) override;
	private:
		std::wofstream mFile;
		std::shared_ptr<ITextFormatter> mFormatter;
	};
}