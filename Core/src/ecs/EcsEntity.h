#pragma once
#include <typeindex>

namespace CPR::ECS
{
    class Entity {
    public:
        using ID = int;
        static ID lastID;

        Entity() : id(lastID++) {}
        Entity(int id) : id(id) {}
        ID getID() const { return id; }

    private:
        ID id;
    };

    extern Entity nullEntity;
}