#include <stdexcept>
#include "EcsManager.h"

namespace CPR::ECS
{
    Entity& EcsManager::createEntity() {
        Entity entity;

        auto [it, inserted] = entities.try_emplace(entity.getID(), std::move(entity));

        return it->second;
    }

    Entity& EcsManager::getEntity(Entity::ID entityID) {
        auto it = entities.find(entityID);
        bool entityExists = it != entities.end();

        if (entityExists) {
            return it->second;
        }

        return nullEntity;
    }

    void EcsManager::removeEntity(Entity& entity) {
        for (auto it = components.begin(); it != components.end(); ++it) {
            it->second.erase(entity.getID());
        }
        entities.erase(entity.getID());
    }

    void EcsManager::removeEntity(Entity::ID entityID) {
        // Check if the entity exists in the map
        auto it = entities.find(entityID);

        bool entityExists = it != entities.end();

        if (entityExists) {
            // remove components
            for (auto& componentTypeMap : components) {
                componentTypeMap.second.erase(entityID);
            }
            // remove entity
            entities.erase(it);
        }
    }

    void EcsManager::addSystem(std::unique_ptr<System> system) {
        systems.push_back(std::move(system));
    }

    void EcsManager::updateSystems() {
        for (auto& system : systems) {
            system->update();
        }
    }
}