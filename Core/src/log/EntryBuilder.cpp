#include "EntryBuilder.h"
#include "Channel.h"

#pragma warning(push) 
#pragma warning(disable: 26815) 
// there is a bug in the static analysis that causes a false positive
// on some functions returning *this (not consistent or correct) 

namespace CPR::LOG
{
	CPR::LOG::EntryBuilder::EntryBuilder(const wchar_t* srcFile, const wchar_t* srcFunction, int srcLine)
		:
		Entry{
			.srcFile = srcFile,
			.srcFunction = srcFunction,
			.srcLine = srcLine,
			.timeStamp = std::chrono::system_clock::now(),
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

	EntryBuilder& EntryBuilder::Trace(std::wstring note)
	{
		text = std::move(note);
		level = LogLevel::Trace;
		return *this;
	}

	EntryBuilder& EntryBuilder::Debug(std::wstring note)
	{
		text = std::move(note);
		level = LogLevel::Debug;
		return *this;
	}

	EntryBuilder& EntryBuilder::Info(std::wstring note)
	{
		text = std::move(note);
		level = LogLevel::Info;
		return *this;
	}

	EntryBuilder& EntryBuilder::Warn(std::wstring note)
	{
		text = std::move(note);
		level = LogLevel::Warn;
		return *this;
	}

	EntryBuilder& EntryBuilder::Error(std::wstring note)
	{
		text = std::move(note);
		level = LogLevel::Error;
		return *this;
	}

	EntryBuilder& EntryBuilder::Fatal(std::wstring note)
	{
		text = std::move(note);
		level = LogLevel::Fatal;
		return *this;
	}

	EntryBuilder& CPR::LOG::EntryBuilder::Channel(IChannel* ch)
	{
		mDest = ch;
		return *this;
	}
	EntryBuilder& EntryBuilder::Trace_Skip(int depth)
	{
		mTraceSkipDepth = depth;
		return *this;
	}
	EntryBuilder& EntryBuilder::No_Trace()
	{
		captureTrace = false;
		return *this;
	}
	EntryBuilder& EntryBuilder::Trace()
	{
		captureTrace = true;
		return *this;
	}
	EntryBuilder& EntryBuilder::No_Line()
	{
		showSourceLine = false;
		return *this;
	}
	EntryBuilder& EntryBuilder::Line()
	{
		showSourceLine = true;
		return *this;
	}
	EntryBuilder::~EntryBuilder()
	{
		if (mDest)
		{
			if (captureTrace.value_or((int)level <= (int)LogLevel::Error))
				trace.emplace(mTraceSkipDepth);
			mDest->Submit(*this);
		}
	}
}
#pragma warning(pop)
