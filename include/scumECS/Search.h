#pragma once

#include "Types.h"
#include <vector>

namespace scum
{

class Manager;

template<typename... Cs>
class Search;

// an object which allows for quick lookup of all the entities which have
// a certain set of components
template<typename... Cs>
class Search
{
public:
	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = ID;
		using pointer = ID*;
		using reference = ID&;

		Iterator(Search* search, std::vector<ID>::iterator cur,
			std::vector<ID>::iterator end);
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

		Search<Cs...>* search;
		std::vector<ID>::iterator cur;
		std::vector<ID>::iterator end;
	};

	Search(Manager& mgr);	
	auto begin();
	auto end();

private:
	Manager& mgr;
	PoolBase* smallest;
	std::vector<PoolBase*> others;

	void getSmallest();
	template<typename C, typename... OtherC>
	PoolBase* getSmallestHelper();
};

}

#include "Manager.h"

namespace scum
{

template<typename... Cs>
bool Search<Cs...>::Iterator::valid() const
{
	bool valid = true;
	for(auto* pool : search->others)
	{
		valid = valid && pool->contains(*cur);
	}
	return valid;
}

template<typename... Cs>
Search<Cs...>::Iterator::Iterator
	(Search<Cs...>* search, std::vector<ID>::iterator cur,
	 std::vector<ID>::iterator end)
	: search(search), cur(cur), end(end)
{
	while(cur != search->smallest->entityEnd() && !valid())
	{
		cur++;
	}
}

template<typename... Cs>
Search<Cs...>::Iterator::Iterator(const Iterator& other)
	: search(other.search), cur(other.cur), end(other.end)
{}

template<typename... Cs>
typename Search<Cs...>::Iterator& Search<Cs...>::Iterator::operator=
	(const Iterator& other)
{
	search = other.search;
	cur = other.cur;
	end = other.end;
	return *this;
}

template<typename... Cs>
ID& Search<Cs...>::Iterator::operator*() const
{
	return *cur;
}

template<typename... Cs>
auto Search<Cs...>::Iterator::operator++()
{
	cur++;
	while(cur != end && !valid())
	{
		cur++;
	}
	return *this;
}

template<typename... Cs>
auto Search<Cs...>::Iterator::operator++(int)
{
	Iterator it = *this;
	++(*this);
	return it;
}

// gets the smallest pool and uses it as the primary pool for future lookups.
template<typename... Cs>
void Search<Cs...>::getSmallest()
{
	others.clear();
	smallest = getSmallestHelper<Cs...>();
}

template<typename... Cs>
template<typename C, typename... OtherC>
PoolBase* Search<Cs...>::getSmallestHelper()
{
	if constexpr (sizeof...(OtherC) == 0)
	{
		return &(mgr.getPool<C>());
	}
	else
	{
		auto* small = getSmallestHelper<OtherC...>();
		PoolBase* pool = &(mgr.getPool<C>());
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
Search<Cs...>::Search(Manager& mgr) : mgr(mgr)
{
	getSmallest();
}

// returns an iterator to the first entity which meets the requirements.
// the iterator references objects of type ID.
template<typename... Cs>
auto Search<Cs...>::begin()
{
	return Search<Cs...>::Iterator
		(this, smallest->entityBegin(), smallest->entityEnd());
}

// returns an iterator to the end of the list of entities which meets 
// the requirements. the iterator references objects of type ID.
template<typename... Cs>
auto Search<Cs...>::end()
{
	return Search<Cs...>::Iterator
		(this, smallest->entityEnd(), smallest->entityEnd());
}

}
