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
		EntryBuilder& Channel(IChannel*);
		~EntryBuilder();
	private:
		IChannel* mDest = nullptr;
	};
}