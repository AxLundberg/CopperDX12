#pragma once  
#include <string>  
#include <sstream>  
#include <optional>  
#include "StackTrace.h"  
#include <format>  
#include "Macro.h"  
#include "Exception.h"
#include "NoReturn.h"

namespace CPR::UTL
{
	ZC_EX_DEF(FailedAssertion);

	class Assertion
	{
	public:
		// types
		enum class Consequence
		{
			Log,
			Terminate,
			Exception,
		};
		// functions
		Assertion(std::wstring expression, const wchar_t* file, const wchar_t* function, int line, Consequence consequence = Consequence::Terminate);
		~Assertion();
		Assertion& Msg(const std::wstring& message);
		template<typename T>
		Assertion& Watch(T&& val, const wchar_t* name)
		{
			mStream << L"   " << name << L" => " << std::forward<T>(val) << L"\n";
			return *this;
		}
		[[noreturn]]
		void Ex();
	private:
		const wchar_t* mFile;
		const wchar_t* mFunction;
		int mLine = -1;
		int mSkipDepth = 0;
		Consequence mConsequence;
		std::wostringstream mStream;
	};
}

#ifndef ZC_CHILASS_ACTIVE  
#ifdef NDEBUG  
#define ZC_CHILASS_ACTIVE false  
#else  
#define ZC_CHILASS_ACTIVE true  
#endif  
#endif  

#define cpr_assert(expr) (!ZC_CHILASS_ACTIVE || bool(expr)) ? void(0) : (void)CPR::UTL::Assertion{ ZC_WSTR(expr), __FILEW__, __FUNCTIONW__, __LINE__ }  

#define cpr_check(expr) bool(expr) ? void(0) : (void)CPR::UTL::Assertion{ ZC_WSTR(expr), __FILEW__, __FUNCTIONW__, __LINE__, ZC_CHILASS_ACTIVE ? CPR::UTL::Assertion::Consequence::Terminate : CPR::UTL::Assertion::Consequence::Log }  

#define cpr_check_fail (void)CPR::UTL::Assertion{ L"[Always Fail]", __FILEW__, __FUNCTIONW__, __LINE__, ZC_CHILASS_ACTIVE ? CPR::UTL::Assertion::Consequence::Terminate : CPR::UTL::Assertion::Consequence::Log }  

#define assert_watch(expr) Watch((expr), ZC_WSTR(expr))
