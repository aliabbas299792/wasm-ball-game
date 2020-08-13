#ifndef ECS_H
#define ECS_H

#include <memory>
#include <bitset>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>

#include "header/helper.h"

#define MAX_COMPONENTS 32
#define MAX_ENTITIES 100000
#define Entity unsigned int
#define Signature std::bitset<MAX_COMPONENTS>

class ECS;

class System {
public:
  std::unordered_set<Entity> entitySet;
  virtual void run(ECS* ecs) {};

  Signature systemSignature;
};

namespace ecs_parts{
  class EntityManager{
    Entity nextEntityID = 0;
    std::vector<Entity> freeEntityIDs;
  public:
    Entity createEntity(){
      if(!freeEntityIDs.empty()){
        Entity id = freeEntityIDs[freeEntityIDs.size()-1];
        freeEntityIDs.pop_back();
        return id;
      }else{
        return nextEntityID++;
      }
    }

    void destroyEntity(Entity id){
      freeEntityIDs.push_back(id);
    }
  };

  class GenericComponentArray{
  public:
    virtual ~GenericComponentArray() = default;
    virtual void remove(Entity id) {};
  };

  template <typename T>
  class ComponentArray : public GenericComponentArray{
    bimap<Entity, unsigned int> entityToIndexMap;
    unsigned int entities = 0;
    std::array<T, MAX_ENTITIES> componentArray;
  public:
    T* get(Entity entity){
      return &componentArray[entityToIndexMap.classicGet(entity)];
    }

    void insert(Entity entity, T component){
      componentArray[entities] = component;
      entityToIndexMap.set(entity, entities); //maps the entity to the index
      entities++;
    }

    void remove(Entity entity) override{
      auto indexToRemove = entityToIndexMap.classicGet(entity);
      entityToIndexMap.classicErase(entity); //erases the mapping
      if(entities > 1) { //only do the swap procedure if this isn't the last element left in the array
        componentArray[indexToRemove] = componentArray[entities-1]; //overwrites the data of the index we want to remove with the data of the last index
        auto lastEntity = entityToIndexMap.reverseGet(entities-1); //gets the entity which was mapped to the last index
        entityToIndexMap.reverseErase(entities-1); //erases the mapping between the above entity and the last index
        entityToIndexMap.set(lastEntity, indexToRemove); //sets the mapping to be between the entity and the index to remove which we've overwritten with that entity's data
      }
      entities--; //decrements the entity count
    }
  };

  class ComponentManager{
    std::unordered_map<const char*, std::shared_ptr<GenericComponentArray>> nameToComponentArray;
    bimap<unsigned int, const char*> idToName;

    unsigned int nextComponentID = 0;

    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray(){
      const char* name = typeid(T).name();
      return std::static_pointer_cast<ComponentArray<T>>(nameToComponentArray[name]);
    }
  public:
    template <typename T>
    void registerComponent(){
      const char* name = typeid(T).name();
      idToName.set(nextComponentID++, name);
      nameToComponentArray[name] = std::make_shared<ComponentArray<T>>();
    }

    template <typename T>
    Signature add(Entity entity, T component, Signature signature){
      signature[idToName.reverseGet(typeid(T).name())] = 1; //updates the signature to indicate this component is being used
      auto array = getComponentArray<T>();
      array->insert(entity, component);
      return signature;
    }

    void remove(Entity entity, std::bitset<MAX_COMPONENTS> components){ //removes the entry for this entity for the component of type T
      for(int i = 0; i < MAX_COMPONENTS; i++){
        if(components[i]){
          auto name = idToName.classicGet(i);
          auto array = nameToComponentArray[name];
          array->remove(entity);
        }
      }
    }

    template <typename T>
    T* get(Entity entity){
      return getComponentArray<T>()->get(entity);
    }

    template <typename T>
    unsigned int getTypeID(){
      return idToName.reverseGet(typeid(T).name()); //returns the ID of this component
    }
  };

  class SystemsManager{
  public:
    template <typename T>
    std::shared_ptr<T> registerSystem(){
      const char* name = typeid(T).name();

      auto system = std::make_shared<T>(); //make a pointer to the system
      systems.insert({ name, system });

      return system;
    }

    template <typename T>
    void runSystem(ECS* ecs){ //runs the system of this type
      systems[typeid(T).name()]->run(ecs);
    }

    void potentiallyAddEntity(Entity entity, Signature signature){
      for(auto &system : systems){
        if((system.second->systemSignature & signature) == system.second->systemSignature){
          system.second->entitySet.insert(entity);
        }
      }
    }

    void potentiallyRemoveEntity(Entity entity, Signature signature){
      for(auto &system : systems){
        system.second->entitySet.erase(entity);
      }
    }

    template <typename System>
    void setSignature(int id){
      systems[typeid(System).name()]->systemSignature[id] = 1; //set id
    }
  private:
    std::unordered_map<const char*, std::shared_ptr<System>> systems;
  };
}

class ECS{
public:
  Entity entityCreate(){
    Entity entity = entityManager.createEntity();
    entityComponentBitsetMap.insert({ entity, Signature() });
    return entity;
  }

  void entityRemove(Entity entity){
    componentManager.remove(entity, entityComponentBitsetMap[entity]);
    systemManager.potentiallyRemoveEntity(entity, entityComponentBitsetMap[entity]);
    entityManager.destroyEntity(entity);
    entityComponentBitsetMap.erase(entity);
  }

  template <typename T>
  void componentAddEntity(Entity entity, T component){
    auto entityData = entityComponentBitsetMap[entity];
    entityComponentBitsetMap[entity] = componentManager.add<T>(entity, component, entityData);
    systemManager.potentiallyAddEntity(entity, entityComponentBitsetMap[entity]);
  }

  template <typename T>
  T* componentGetData(Entity entity){
    return componentManager.get<T>(entity);
  }

  template <typename T>
  void componentRegister(){
    componentManager.registerComponent<T>();
  }

  template <typename T>
  std::shared_ptr<T> systemRegister(){
    return systemManager.registerSystem<T>();
  }

  template <typename System, typename Component>
  void systemAddComponentToSignature(){
    systemManager.setSignature<System>(componentManager.getTypeID<Component>());
    for(auto &entitySignaturePair : entityComponentBitsetMap){ //loops over every entity and potentially adds them to this system
      systemManager.potentiallyAddEntity(entitySignaturePair.first, entitySignaturePair.second);
    }
  }

  template <typename SystemType>
  void systemRun(ECS* ecs){
    systemManager.runSystem<SystemType>(ecs);
  }
private:
  ecs_parts::SystemsManager systemManager;
  ecs_parts::ComponentManager componentManager;
  ecs_parts::EntityManager entityManager;

  std::unordered_map<Entity, Signature> entityComponentBitsetMap;
};

#endif