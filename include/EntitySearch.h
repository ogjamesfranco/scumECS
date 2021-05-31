#pragma once

#include "Types.h"
#include <vector>

namespace scum
{

class EntityManager;

template<typename... Cs>
class EntitySearch;

// an object which allows for quick lookup of all the entities which have
// a certain set of components
template<typename... Cs>
class EntitySearch
{
public:
	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = EntID;
		using pointer = EntID*;
		using reference = EntID&;

		Iterator(EntitySearch* search, std::vector<EntID>::iterator cur,
			std::vector<EntID>::iterator end);
		Iterator(const Iterator& other);
		Iterator& operator=(const Iterator& other);
		reference operator*() const;
		auto operator++();
		auto operator++(int);

		friend bool operator==(const Iterator& l, const Iterator& r)
		{
			return l.cur == r.cur;
		}
		friend bool operator!=(const Iterator& l, const Iterator& r)
		{
			return l.cur != r.cur;
		}

	private:
		bool valid() const;

		EntitySearch<Cs...>* search;
		std::vector<EntID>::iterator cur;
		std::vector<EntID>::iterator end;
	};

	EntitySearch(EntityManager& mgr);	
	auto begin();
	auto end();

private:
	EntityManager& mgr;
	ComponentPoolBase* smallest;
	std::vector<ComponentPoolBase*> others;

	void getSmallest();
	template<typename C, typename... OtherC>
	ComponentPoolBase* getSmallestHelper();
};

}

#include "EntityManager.h"

namespace scum
{

template<typename... Cs>
bool EntitySearch<Cs...>::Iterator::valid() const
{
	bool valid = true;
	for(auto* pool : search->others)
	{
		valid = valid && pool->contains(*cur);
	}
	return valid;
}

template<typename... Cs>
EntitySearch<Cs...>::Iterator::Iterator
	(EntitySearch<Cs...>* search, std::vector<EntID>::iterator cur,
	 std::vector<EntID>::iterator end)
	: search(search), cur(cur), end(end)
{
	while(cur != search->smallest->entityEnd() && !valid())
	{
		cur++;
	}
}

template<typename... Cs>
EntitySearch<Cs...>::Iterator::Iterator(const Iterator& other)
	: search(other.search), cur(other.cur), end(other.end)
{}

template<typename... Cs>
typename EntitySearch<Cs...>::Iterator& EntitySearch<Cs...>::Iterator::operator=
	(const Iterator& other)
{
	search = other.search;
	cur = other.cur;
	end = other.end;
	return *this;
}

template<typename... Cs>
EntID& EntitySearch<Cs...>::Iterator::operator*() const
{
	return *cur;
}

template<typename... Cs>
auto EntitySearch<Cs...>::Iterator::operator++()
{
	cur++;
	while(cur != end && !valid())
	{
		cur++;
	}
	return *this;
}

template<typename... Cs>
auto EntitySearch<Cs...>::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

// gets the smallest pool and uses it as the primary pool for future lookups.
template<typename... Cs>
void EntitySearch<Cs...>::getSmallest()
{
	others.clear();
	smallest = getSmallestHelper<Cs...>();
}

template<typename... Cs>
template<typename C, typename... OtherC>
ComponentPoolBase* EntitySearch<Cs...>::getSmallestHelper()
{
	if constexpr (sizeof...(OtherC) == 0)
	{
		return mgr.getPool<C>();
	}
	else
	{
		auto* small = getSmallestHelper<OtherC...>();
		ComponentPoolBase* pool = mgr.getPool<C>();
		if(pool->size() < small->size())
		{
			others.push_back(small);
			return pool;
		}

		others.push_back(pool);
		return small;
	}
}

template<typename... Cs>
EntitySearch<Cs...>::EntitySearch(EntityManager& mgr) : mgr(mgr)
{
	getSmallest();
}

// returns an iterator to the first entity which meets the requirements.
// the iterator references objects of type EntID.
template<typename... Cs>
auto EntitySearch<Cs...>::begin()
{
	return EntitySearch<Cs...>::Iterator
		(this, smallest->entityBegin(), smallest->entityEnd());
}

// returns an iterator to the end of the list of entities which meets 
// the requirements. the iterator references objects of type EntID.
template<typename... Cs>
auto EntitySearch<Cs...>::end()
{
	return EntitySearch<Cs...>::Iterator
		(this, smallest->entityEnd(), smallest->entityEnd());
}

}
