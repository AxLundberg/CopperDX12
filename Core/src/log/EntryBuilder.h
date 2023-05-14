#pragma once
#include "Entry.h"

namespace CPR::LOG
{
	class IChannel;

	class EntryBuilder : private Entry
	{
	public:
		EntryBuilder(const wchar_t* srcFile, const wchar_t* srcFunction, int srcLine);
		EntryBuilder& Note(std::wstring note);
		EntryBuilder& Level(LogLevel);
		EntryBuilder& Trace(std::wstring note = L"");
		EntryBuilder& Debug(std::wstring note = L"");
		EntryBuilder& Info(std::wstring note = L"");
		EntryBuilder& Warn(std::wstring note = L"");
		EntryBuilder& Error(std::wstring note = L"");
		EntryBuilder& Fatal(std::wstring note = L"");
		EntryBuilder& Channel(IChannel*);
		~EntryBuilder();
	private:
		IChannel* mDest = nullptr;
	};
}