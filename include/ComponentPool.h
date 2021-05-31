#pragma once

#include "Types.h"
#include <vector>
#include <typeinfo>
#include <unordered_map>
#include <utility>

#include <iostream>

namespace scum
{

// provides a generic interface for component pools
class ComponentPoolBase
{
public:
	bool contains(EntID id);
	void queueRemove(EntID id);
	virtual void processQueues() = 0;
	virtual void remove(EntID id) = 0;

	auto entityBegin();
	auto entityEnd();
	const auto entityBegin() const;
	const auto entityEnd() const;
	auto size() const;

protected:
	AssocContainer<EntID, size_t> lookupTable;
	std::vector<EntID> entities;
	std::vector<EntID> removeQueue;
};

// queues an entity's component for removal
inline void ComponentPoolBase::queueRemove(EntID id)
{
	removeQueue.push_back(id);
}

// checks if the pool contains a component for a given entity
inline bool ComponentPoolBase::contains(EntID id)
{
	return (lookupTable.find(id) != lookupTable.end());
}

// returns an iterator to the start of the list of IDs in the pool
inline const auto ComponentPoolBase::entityBegin() const
{
	return entities.begin();
}

inline auto ComponentPoolBase::entityBegin()
{
	return entities.begin();
}

// returns an iterator to the end of the list of IDs in the pool
inline const auto ComponentPoolBase::entityEnd() const
{
	return entities.end();
}

inline auto ComponentPoolBase::entityEnd()
{
	return entities.end();
}

// returns the number of components in the pool
auto ComponentPoolBase::size() const
{
	return entities.size();
}

// stores components of a given type and associates them with IDs
template<typename C>
class ComponentPool final : public ComponentPoolBase
{
public:
	template<typename... Args>
	C* add(EntID, Args... args);
	virtual void remove(EntID id) final;

	template<typename... Args>
	C* queueAdd(EntID id, Args... args);
	virtual void processQueues() final;

	C* get(EntID id);
	C* tryGet(EntID id);
	C* operator[](EntID id);
	const C* get(EntID id) const;
	const C* tryGet(EntID id) const;
	const C* operator[](EntID id) const;

	auto begin();
	auto end();
	const auto begin() const;
	const auto end() const;

private:
	std::vector<C> components;
	std::vector<std::pair<EntID, C>> addQueue;
};

// add component to a given entity
template<typename C>
template<typename... Args>
C* ComponentPool<C>::add(EntID id, Args... args)
{
	entities.push_back(id);
	components.push_back(C{std::forward<Args>(args)...});
	lookupTable.insert(std::pair<EntID, size_t>(id, components.size() - 1));
	return &components.back();
}

// queue component for addition to a given entity
template<typename C>
template<typename... Args>
C* ComponentPool<C>::queueAdd(EntID id, Args... args)
{
	addQueue.emplace_back(
		std::pair<EntID, C>(id, C(std::forward<Args>(args)...)) );
	return &addQueue.back.second();
}

// applies all queued additions and removals for the pool
template<typename C>
void ComponentPool<C>::processQueues()
{
	for(auto& pair : addQueue)
	{
		add(pair.first, std::move(pair.second));
	}
	addQueue.clear();
	for(auto& id : removeQueue)
	{
		remove(id);
	}
	removeQueue.clear();
}

// removes the component for a given ID. behavior is undefined if the entity
// does not have the component.
template<typename C>
void ComponentPool<C>::remove(EntID id)
{
	size_t index = lookupTable[id];
	lookupTable[entities.back()] = index;
	entities[index] = entities.back();
	components[index] = components.back();
	components.pop_back();
	entities.pop_back();
}

// gets the component for a given ID. behavior is undefined if the entity
// does not have the component.
template<typename C>
const C* ComponentPool<C>::get(EntID id) const
{
	return &components[lookupTable.find(id)->second];
}

template<typename C>
C* ComponentPool<C>::get(EntID id)
{
	return const_cast<C*>(const_cast<const ComponentPool<C>&>(*this).get(id));
}

// attempts to get the component for a given entity.
// returns nullptr if the entity doesn't have the component.
template<typename C>
const C* ComponentPool<C>::tryGet(EntID id) const
{
	auto it = lookupTable.find(id);
	if(it == lookupTable.end())
	{
		return nullptr;
	}
	return &components[it->second].data;
}

template<typename C>
C* ComponentPool<C>::tryGet(EntID id)
{
	return const_cast<C*>(tryGet(id));
}

// alternate syntax for get()
template<typename C>
const C* ComponentPool<C>::operator[](EntID id) const
{
	return get(id);
}

template<typename C>
C* ComponentPool<C>::operator[](EntID id)
{
	return get(id);
}

// returns iterator to the start of the pool of components.
// iterator references objects of type std::pair<EntID, C>
template<typename C>
auto ComponentPool<C>::begin()
{
	return components.begin();
}

// returns iterator to the end of the pool of components.
// iterator references objects of type std::pair<EntID, C>
template<typename C>
auto ComponentPool<C>::end()
{
	return components.end();
}

template<typename C>
const auto ComponentPool<C>::begin() const
{
	return components.begin();
}

template<typename C>
const auto ComponentPool<C>::end() const
{
	return components.end();
}

}
