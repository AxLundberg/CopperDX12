#include "EntryBuilder.h"
#include "Channel.h"

namespace CPR::LOG
{
	CPR::LOG::EntryBuilder::EntryBuilder(const wchar_t* srcFile, const wchar_t* srcFunction, int srcLine)
		:
		Entry{
			.level = LogLevel::Error,
			.srcFile = srcFile,
			.srcFunction = srcFunction,
			.srcLine = srcLine,
		}
	{}

	EntryBuilder& CPR::LOG::EntryBuilder::Note(std::wstring note)
	{
		text = std::move(note);
		return *this;
	}

	EntryBuilder& CPR::LOG::EntryBuilder::Level(LogLevel lvl)
	{
		level = lvl;
		return *this;
	}

	EntryBuilder& CPR::LOG::EntryBuilder::Channel(IChannel* ch)
	{
		mDest = ch;
		return *this;
	}
	EntryBuilder::~EntryBuilder()
	{
		if (mDest)
		{
			mDest->Submit(*this);
		}
	}
}
