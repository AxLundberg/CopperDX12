#include "TextFormatter.h"
#include "Entry.h"
#include <format>

namespace CPR::LOG
{
    std::wstring CPR::LOG::TextFormatter::Format(const Entry& e) const
    {
        return std::format(L"@{} {{{}}} {}\n  >> in function: {}\n     {}({})\n",
            GetLevelName(e.level),
            std::chrono::zoned_time{ std::chrono::current_zone(), e.timeStamp },
            e.text,
            e.srcFunction,
            e.srcFile,
            e.srcLine
       );
    }
}
