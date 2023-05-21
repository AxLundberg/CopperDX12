#include "Log.h"
#include "TextFormatter.h"
#include "MsvcDebugDriver.h"
#include "SimpleFileDriver.h"
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
            std::vector<std::shared_ptr<LOG::IDriver>> drivers{
                IOC::Get().Resolve<LOG::IMsvcDebugDriver>(),
                IOC::Get().Resolve<LOG::ISimpleFileDriver>()
            };
            auto ch = std::make_shared<LOG::Channel>(std::move(drivers));
            ch->AttachPolicy(IOC::Get().Resolve<LOG::ISeverityLevelPolicy>());
            return ch;
        });
        IOC::Get().Register<LOG::IMsvcDebugDriver>([] {
            return std::make_shared<LOG::MsvcDebugDriver>(IOC::Get().Resolve<LOG::ITextFormatter>());
        });
        IOC::Get().Register<LOG::ISimpleFileDriver>([] {
            return std::make_shared<LOG::SimpleFileDriver>("logs\\log.txt", IOC::Get().Resolve<LOG::ITextFormatter>());
        });
        IOC::Get().Register<LOG::ITextFormatter>([] {
            return std::make_shared<LOG::TextFormatter>();
        });
        IOC::Get().Register<LOG::ISeverityLevelPolicy>([] {
            return std::make_shared<LOG::SeverityLevelPolicy>(LOG::LogLevel::Error);
        });
        // singleton
        IOC::Sing().RegisterPassThrough<LOG::IChannel>();
    }
}
