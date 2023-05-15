#include "TextFormatter.h"
#include "Entry.h"
#include <format>
#include <sstream>

namespace CPR::LOG
{
    std::wstring CPR::LOG::TextFormatter::Format(const Entry& e) const
    {
        std::wostringstream oss;
        oss << std::format(L"@{} {{{}}} {}\n  >> in function: {}\n     {}({})\n",
            GetLevelName(e.level),
            std::chrono::zoned_time{ std::chrono::current_zone(), e.timeStamp },
            e.text,
            e.srcFunction,
            e.srcFile,
            e.srcLine
        );
        if (e.trace)
        {
            oss << e.trace->Print() << std::endl;
        }
        return oss.str();
    }
}
