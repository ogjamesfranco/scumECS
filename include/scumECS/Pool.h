#pragma once

#include "Types.h"
#include <vector>
#include <typeinfo>
#include <utility>

namespace scum
{

// provides a generic interface for component pools
class PoolBase
{
public:
	bool contains(ID id);
	void queueRemove(ID id);
	virtual void processQueues() = 0;
	virtual void remove(ID id) = 0;

	auto entityBegin();
	auto entityEnd();
	const auto entityBegin() const;
	const auto entityEnd() const;
	auto size() const;

protected:
	AssocContainer<ID, size_t> lookupTable;
	std::vector<ID> entities;
	std::vector<ID> removeQueue;
};

// queues an entity's component for removal
inline void PoolBase::queueRemove(ID id)
{
	removeQueue.push_back(id);
}

// checks if the pool contains a component for a given entity
inline bool PoolBase::contains(ID id)
{
	return (lookupTable.find(id) != lookupTable.end());
}

// returns an iterator to the start of the list of IDs in the pool
inline const auto PoolBase::entityBegin() const
{
	return entities.begin();
}

inline auto PoolBase::entityBegin()
{
	return entities.begin();
}

// returns an iterator to the end of the list of IDs in the pool
inline const auto PoolBase::entityEnd() const
{
	return entities.end();
}

inline auto PoolBase::entityEnd()
{
	return entities.end();
}

// returns the number of components in the pool
inline auto PoolBase::size() const
{
	return entities.size();
}

template<typename C>
struct ComponentPair
{
	ComponentPair(ID& id, C& data);

	ID id;
	C& data;
};

template<typename C>
ComponentPair<C>::ComponentPair(ID& id, C& data) : id(id), data(data)
{}

// stores components of a given type and associates them with IDs
template<typename C>
class Pool final : public PoolBase
{
public:
	class Iterator
	{
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = typename std::vector<C>::size_type;
		using value_type = ComponentPair<C>;
		using pointer = ComponentPair<C>*;
		using reference = ComponentPair<C>&;

		Iterator(std::vector<C>& components,
			typename std::vector<C>::iterator componentsIt,
			std::vector<ID>& ids,
			typename std::vector<ID>::iterator idsIt);
		Iterator(const Iterator& other);

		Iterator& operator=(const Iterator& other);
		value_type operator*() const;
		auto operator++();
		auto operator++(int);
		auto operator--();
		auto operator--(int);

		friend bool operator==(const Iterator& l, const Iterator& r)
		{
			return l.componentsIt == r.componentsIt;
		}
		friend bool operator!=(const Iterator& l, const Iterator& r)
		{
			return l.componentsIt != r.componentsIt;
		}

	private:
		std::vector<C>& components;
		std::vector<ID>& ids;
		typename std::vector<C>::iterator componentsIt;
		typename std::vector<ID>::iterator idsIt;
	};

	template<typename... Args>
	C* add(ID, Args... args);
	virtual void remove(ID id) final;

	template<typename... Args>
	C* queueAdd(ID id, Args... args);
	virtual void processQueues() final;

	C* get(ID id);
	C* tryGet(ID id);
	C* operator[](ID id);
	const C* get(ID id) const;
	const C* tryGet(ID id) const;
	const C* operator[](ID id) const;

	auto begin();
	auto end();
	const auto begin() const;
	const auto end() const;

private:
	std::vector<C> components;
	std::vector<std::pair<ID, C>> addQueue;
};

template<typename C>
Pool<C>::Iterator::Iterator(std::vector<C>& components,
	typename std::vector<C>::iterator componentsIt,
	std::vector<ID>& ids,
	typename std::vector<ID>::iterator idsIt)
	: components(components), componentsIt(componentsIt), ids(ids), idsIt(idsIt)
{}

template<typename C>
Pool<C>::Iterator::Iterator(const Iterator& other)
	: components(other.components), componentsIt(other.componentsIt),
	ids(other.ids), idsIt(other.idsIt)
{}

template<typename C>
typename Pool<C>::Iterator& Pool<C>::Iterator::operator=
	(const typename Pool<C>::Iterator& other)
{
	components = other.components;
	componentsIt = other.componentsIt;
	ids = other.ids;
	idsIt = other.idsIt;
}

template<typename C>
ComponentPair<C> Pool<C>::Iterator::operator*() const
{
	return ComponentPair<C>{*idsIt, *componentsIt};
}

template<typename C>
auto Pool<C>::Iterator::operator++()
{
	idsIt++;
	componentsIt++;
	return *this;
}

template<typename C>
auto Pool<C>::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

template<typename C>
auto Pool<C>::Iterator::operator--()
{
	idsIt--;
	componentsIt--;
	return *this;
}

template<typename C>
auto Pool<C>::Iterator::operator--(int)
{
	Iterator it = *this;
	--(*this);
	return it;
}

// add component to a given entity
template<typename C>
template<typename... Args>
C* Pool<C>::add(ID id, Args... args)
{
	entities.push_back(id);
	components.push_back(C{std::forward<Args>(args)...});
	lookupTable.insert(std::pair<ID, size_t>(id, components.size() - 1));
	return &components.back();
}

// queue component for addition to a given entity
template<typename C>
template<typename... Args>
C* Pool<C>::queueAdd(ID id, Args... args)
{
	addQueue.emplace_back(
		std::pair<ID, C>(id, C(std::forward<Args>(args)...)) );
	return &addQueue.back.second();
}

// applies all queued additions and removals for the pool
template<typename C>
void Pool<C>::processQueues()
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
void Pool<C>::remove(ID id)
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
const C* Pool<C>::get(ID id) const
{
	return &components[lookupTable.find(id)->second];
}

template<typename C>
C* Pool<C>::get(ID id)
{
	return const_cast<C*>(const_cast<const Pool<C>&>(*this).get(id));
}

// attempts to get the component for a given entity.
// returns nullptr if the entity doesn't have the component.
template<typename C>
const C* Pool<C>::tryGet(ID id) const
{
	auto it = lookupTable.find(id);
	if(it == lookupTable.end())
	{
		return nullptr;
	}
	return &components[it->second].data;
}

template<typename C>
C* Pool<C>::tryGet(ID id)
{
	return const_cast<C*>(tryGet(id));
}

// alternate syntax for get()
template<typename C>
const C* Pool<C>::operator[](ID id) const
{
	return get(id);
}

template<typename C>
C* Pool<C>::operator[](ID id)
{
	return get(id);
}

// returns iterator to the start of the pool of components.
// iterator references objects of type ComponentPair<C>
template<typename C>
auto Pool<C>::begin()
{
	return Iterator(components, components.begin(),
		entities, entities.begin());
}

// returns iterator to the end of the pool of components.
// iterator references objects of type ComponentPair<C>
template<typename C>
auto Pool<C>::end()
{
	return Iterator(components, components.end(),
		entities, entities.end());
}

template<typename C>
const auto Pool<C>::begin() const
{
	return Iterator(components, components.begin(),
		entities, entities.begin());
}

template<typename C>
const auto Pool<C>::end() const
{
	return Iterator(components, components.end(),
		entities, entities.end());
}

}
