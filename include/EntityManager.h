#pragma once

#include "Types.h"
#include "ComponentPool.h"
#include <vector>

namespace scum
{

template<typename... Cs>
class EntitySearch;

// the core class of the entity system. contains a set of pools, each of which
// contains components of a particular type. also manages entity IDs
class EntityManager
{
public:
	EntityManager();
	~EntityManager();
	EntID newID();
	template<typename C, typename... Args>
	C* add(EntID id, Args... args);
	template<typename C>
	void remove(EntID id);
	void destroy(EntID id);

	template<typename C, typename... Args>
	C* queueAdd(EntID id, Args... args);
	void queueDestroy(EntID id);
	void processQueues();

	template<typename C>
	C* get(EntID id);
	template<typename C>
	C* tryGet(EntID id);
	template<typename C>
	ComponentPool<C>* getPool();

	template<typename... Cs>
	EntitySearch<Cs...> search();

private:
	std::vector<ComponentPoolBase*> pools;
	AssocContainer<size_t, size_t> lookupTable;
	std::vector<EntID> freeIDs;
	EntID nextID = 0; // the ID counter starts at 1; 0 is reserved as "Null"

	std::vector<EntID> destroyQueue;
};

}

#include "EntitySearch.h"

namespace scum
{

inline EntityManager::EntityManager()
{
	freeIDs.push_back(nextID + 1); // add "1" as the first free EntID
}

inline EntityManager::~EntityManager()
{
	for(auto* pool : pools)
	{
		delete(pool);
	}
}

// returns a free ID. the manager is guaranteed to return at least
// 1,048,576 other IDs before recycling a given previously used ID.
// this is also the maximum number of simulataneous unique IDs - behavior
// is undefined if an ID is requested when 1,048,576 are already in use.
inline EntID EntityManager::newID()
{
	if(freeIDs.size() != 0)
	{
		EntID id = freeIDs.back();
		freeIDs.pop_back();
		return id;
	}

	nextID += 4096;
	return nextID;
}

// adds a component to an entity
template<typename C, typename... Args>
C* EntityManager::add(EntID id, Args... args)
{
	return getPool<C>()->add(id, std::forward<Args>(args)...);
}

// queues a component for addition to an entity
template<typename C, typename... Args>
C* EntityManager::queueAdd(EntID id, Args... args)
{
	return getPool<C>()->queueAdd(id, std::forward<Args>(args)...);
}

// removes a component from an entity
template<typename C>
void EntityManager::remove(EntID id)
{
	getPool<C>()->remove(id);
}

// removes all components from an entity, then frees the ID
inline void EntityManager::destroy(EntID id)
{
	for(auto pool : pools)
	{
		if(pool->contains(id))
		{
			pool->remove(id);
		}
	}

	id++;
	if(id & (4096 - 1) != 0) // equivalent to id % 4096
	{
		freeIDs.push_back(id);
	}
}

// queues a component for destruction
inline void EntityManager::queueDestroy(EntID id)
{
	destroyQueue.push_back(id);
}

// applies all queued additions, removals, and destructions for all pools
inline void EntityManager::processQueues()
{
	for(auto* pool : pools)
	{
		pool->processQueues();
	}
	for(auto& id : destroyQueue)
	{
		destroy(id);
	}
	destroyQueue.clear();
}

// gets and returns the pool for the specified component type.
// will add a pool if one doesn't exist. pointers or references to pools
// are guaranteed to remain valid for the duration of the manager's lifetime.
template<typename C>
ComponentPool<C>* EntityManager::getPool()
{
	size_t type = typeid(C).hash_code();
	if(lookupTable.find(type) == lookupTable.end())
	{
		pools.push_back(new ComponentPool<C>);
		lookupTable.insert(std::pair<size_t,size_t>(type, pools.size()-1));
	}

	return static_cast<ComponentPool<C>*>
		(pools[lookupTable.find(type)->second]);
}

// gets a component for a given entity.
// behavior is undefined if the entity doesn't have the component
template<typename C>
C* EntityManager::get(EntID id)
{
	return getPool<C>()->get(id);
}

// attempts to get a component for a given entity.
// returns nullptr if the entity doesn't have the component
template<typename C>
C* EntityManager::tryGet(EntID id)
{
	return getPool<C>()->tryGet(id);
}

// returns an entity search for the given components
template<typename... Cs>
EntitySearch<Cs...> EntityManager::search()
{
	return EntitySearch<Cs...>(*this);
}

}
