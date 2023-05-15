#include "Log.h"
#include "TextFormatter.h"
#include "MsvcDebugDriver.h"
#include "SeverityLevelPolicy.h"
#include <Core/src/ioc/Container.h>
#include <Core/src/ioc/Singletons.h>

namespace CPR::LOG
{
    IChannel* GetDefaultChannel()
    {
        static std::shared_ptr<IChannel> channelCachePtr = IOC::Sing().Resolve<IChannel>();
        return channelCachePtr.get();
    }

    void Boot()
    {
        // container
        IOC::Get().Register<LOG::IChannel>([] {
            std::vector drivers{ IOC::Get().Resolve<LOG::IDriver>() };
            auto ch = std::make_shared<LOG::Channel>(std::move(drivers));
            ch->AttachPolicy(IOC::Get().Resolve<LOG::SeverityLevelPolicy>());
            return ch;
        });
        IOC::Get().Register<LOG::IDriver>([] {
            return std::make_shared<LOG::MsvcDebugDriver>(IOC::Get().Resolve<LOG::ITextFormatter>());
        });
        IOC::Get().Register<LOG::ITextFormatter>([] {
            return std::make_shared<LOG::TextFormatter>();
        });
        IOC::Get().Register<LOG::SeverityLevelPolicy>([] {
            return std::make_shared<LOG::SeverityLevelPolicy>(LOG::LogLevel::Error);
        });
        // singleton
        IOC::Sing().RegisterPassThrough<LOG::IChannel>();
    }
}
