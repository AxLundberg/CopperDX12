#include "Singletons.h"

namespace CPR::IOC
{
    Singletons& Sing()
    {
        static Singletons sing;
        return sing;
    }
}
