#pragma once

namespace CPR::ECS
{
    class System {
    public:
        virtual ~System() = default;
        virtual void update() = 0;
    };
}