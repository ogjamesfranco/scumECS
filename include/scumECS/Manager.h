#pragma once

#include "Types.h"
#include "Pool.h"
#include <vector>

namespace scum
{

template<typename... Cs>
class Search;

// the core class of the entity system. contains a set of pools, each of which
// contains components of a particular type. also manages entity IDs
class Manager
{
public:
	Manager();
	~Manager();
	ID newID();
	template<typename C, typename... Args>
	C* add(ID id, Args... args);
	template<typename C>
	void remove(ID id);
	void destroy(ID id);

	template<typename C, typename... Args>
	C* queueAdd(ID id, Args... args);
	void queueDestroy(ID id);
	void processQueues();

	template<typename C>
	C* get(ID id);
	template<typename C>
	C* tryGet(ID id);
	template<typename C>
	Pool<C>& getPool();

	template<typename... Cs>
	Search<Cs...> search();

private:
	std::vector<PoolBase*> pools;
	AssocContainer<size_t, size_t> lookupTable;
	std::vector<ID> freeIDs;
	ID nextID = 0; // the ID counter starts at 1; 0 is reserved as "Null"

	std::vector<ID> destroyQueue;
};

}

#include "Search.h"

namespace scum
{

inline Manager::Manager()
{
	freeIDs.push_back(nextID + 1); // add "1" as the first free ID
}

inline Manager::~Manager()
{
	for(auto* pool : pools)
	{
		delete(pool);
	}
}

// returns a free ID. the manager is guaranteed to return at least
// 4,096 other IDs before recycling a given previously used ID.
// there is a limit of 1,048,576 simultaneous unique IDs. generating new IDs
// past that point is undefined behavior
inline ID Manager::newID()
{
	if(freeIDs.size() != 0)
	{
		ID id = freeIDs.back();
		freeIDs.pop_back();
		return id;
	}

	nextID += 4096;
	return nextID;
}

// adds a component to an entity
template<typename C, typename... Args>
C* Manager::add(ID id, Args... args)
{
	return getPool<C>().add(id, std::forward<Args>(args)...);
}

// queues a component for addition to an entity
template<typename C, typename... Args>
C* Manager::queueAdd(ID id, Args... args)
{
	return getPool<C>().queueAdd(id, std::forward<Args>(args)...);
}

// removes a component from an entity
template<typename C>
void Manager::remove(ID id)
{
	getPool<C>().remove(id);
}

// removes all components from an entity, then frees the ID
inline void Manager::destroy(ID id)
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
inline void Manager::queueDestroy(ID id)
{
	destroyQueue.push_back(id);
}

// applies all queued additions, removals, and destructions for all pools
inline void Manager::processQueues()
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
Pool<C>& Manager::getPool()
{
	size_t type = typeid(C).hash_code();
	if(lookupTable.find(type) == lookupTable.end())
	{
		pools.push_back(new Pool<C>);
		lookupTable.insert(std::pair<size_t,size_t>(type, pools.size()-1));
	}

	return static_cast<Pool<C>&>
		(*(pools[lookupTable.find(type)->second]));
}

// gets a component for a given entity.
// behavior is undefined if the entity doesn't have the component
template<typename C>
C* Manager::get(ID id)
{
	return getPool<C>().get(id);
}

// attempts to get a component for a given entity.
// returns nullptr if the entity doesn't have the component
template<typename C>
C* Manager::tryGet(ID id)
{
	return getPool<C>().tryGet(id);
}

// returns an entity search for the given components
template<typename... Cs>
Search<Cs...> Manager::search()
{
	return Search<Cs...>(*this);
}

}
