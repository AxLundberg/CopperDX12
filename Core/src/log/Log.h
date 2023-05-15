#pragma once
#include "Channel.h"
#include "EntryBuilder.h"

namespace CPR::LOG
{
	IChannel* GetDefaultChannel();
	void Boot();
}

#define cprlog LOG::EntryBuilder{ __FILEW__, __FUNCTIONW__, __LINE__, }.Channel(LOG::GetDefaultChannel())
