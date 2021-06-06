#pragma once
#include "Types.h"

namespace scum
{

class Manager;

class Entity
{
public:
	const ID id;
	Entity(Manager& manager, ID id);

	template<typename C>
	bool has();
	template<typename C>
	C* get();
	template<typename C>
	C* tryGet();
	template<typename C, typename... Args>
	C* add(Args... args);
	template<typename C, typename... Args>
	C* queueAdd(Args... args);
	template<typename C>
	void remove();
private:
	Manager& manager;
};

}

#include "Manager.h"

namespace scum
{

inline Entity::Entity(Manager& manager, ID id) : manager(manager), id(id)
{}

template<typename C>
bool Entity::has()
{
	return manager.contains<C>(id);
}

template<typename C>
C* Entity::get()
{
	return manager.get<C>(id);
}

template<typename C>
C* Entity::tryGet()
{
	return manager.tryGet<C>(id);
}

template<typename C, typename... Args>
C* Entity::add(Args... args)
{
	return manager.add<C>(id, std::forward<Args>(args)...);
}

template<typename C, typename... Args>
C* Entity::queueAdd(Args... args)
{
	return manager.queueAdd<C>(id, std::forward<Args>(args)...);
}

template<typename C>
void Entity::remove()
{
	manager.remove<C>(id);
}

}
