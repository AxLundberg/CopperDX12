#include "TextFormatter.h"
#include "Entry.h"
#include "Core/src/win/Util.h"
#include <format>
#include <sstream>

namespace CPR::LOG
{
    std::wstring CPR::LOG::TextFormatter::Format(const Entry& e) const
    {
        std::wostringstream oss;
        oss << std::format(L"@{} {{{}}} {}",
            GetLevelName(e.level),
            std::chrono::zoned_time{ std::chrono::current_zone(), e.timeStamp },
            e.text,
            e.srcFunction,
            e.srcFile,
            e.srcLine
        );
        if (e.hResult)
        {
            oss << std::format(L"  !HRESULT [{:#010x}]: {}\n", *e.hResult,
                WIN::GetErrorDescription(*e.hResult));
        }
        if (e.showSourceLine.value_or(true))
        {
            oss << std::format(L"\n  >> at {}\n     {}({})\n",
                e.srcFunction,
                e.srcFile,
                e.srcLine
            );
        }
        else
        {
            oss << L"\n";
        }
        if (e.trace)
        {
            oss << e.trace->Print() << std::endl;
        }
        return oss.str();
    }
}
