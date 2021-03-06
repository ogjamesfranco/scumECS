# scumECS

**scumECS** is a very small, simple, header-only Entity Component System library.  
My goal was to write my own ECS with a convenient API and a data-oriented design that would encourage good performance. My intended use case is game development, but the library does not have any game-specific functionality. scumECS is designed for use as a pure "components are data" ECS, with all the functionality being handled by external "systems" which operate on component data.  
  
Earlier versions looked very different, but I reworked my design significantly after being inspired by how [EnTT](https://github.com/skypjack/entt) dealt with certain ECS design problems very effectively.

## Features/Design
- Very small (~850 lines in total, not including Tessil's Robin Map)
- API offers various syntax styles for each operation
- Simple and fast mechanism for searching for entities by component(s)
- Entity IDs are recycled, eliminating risk of overflow
	- Additionally, the ECS will provide a minimum of 4096 other IDs before reusing a given ID
- Each type of component is stored in a contiguous array with no gaps or placeholder data
- Hash tables are used for constant-time lookup of components and pools
	- The underlying hash table class can be changed by editing a single typedef in include/scumECS/Types.h
	- Tessil's fantastic [Robin Map](https://github.com/Tessil/robin-map) is included and used by default, but the library is also tested with std::unordered_map
- Built-in queue system for delayed addition or removal of components

## Limitations
- Removing or adding any components to pools while iterating over them invalidates references to components, including iterators. The provided queueing API can be used to circumvent this.

## Usage
The Manager class represents a collection of pools, each containing components of a certain type.  
You can __add components__ through the manager, or you can get a specific pool and add them that way.  
To __retrieve components__, you can go through the manager, pools, or a search.  
A __search__ lets you iterate over all the entities that have a certain set of components with minimal additional performance cost.  
An __Entity__ wrapper class is also included as an alternate API for accessing components.  
   
Since references/pointers to components are quickly invalidated, the preferred way to store a reference to a component is by storing the entity's ID. To this end, the constant scum::Null is provided, which will never be equal to an entity ID.

### Example
```cpp
#include "scumECS/ECS.h"

struct MyComponent
{
	int myInt;
	bool myValue;
};

struct OtherComponent
{};

int main()
{
	scum::Manager manager;
	for(int i = 0; i < 100; i++)
	{
		scum::ID id = manager.newID(); // make a new entity
		manager.add<MyComponent>(id, 7, true); // add a component to it
		manager.add<OtherComponent>(id);

		// or use the entity wrapper API
		scum::Entity ent = manager.newEntity();
		ent.add<MyComponent>(7, true);
		ent.add<OtherComponent>();
	}

	// iterate over all components in a pool
	for(auto pair : manager.getPool<MyComponent>())
	{
		if(pair.data.myValue) // access component data
		{
			scum::ID id = pair.id; // access associated entity ID
			// do stuff
		}
	}

	// search for all entities with both Component and OtherComponent
	scum::Search search = manager.search<MyComponent, OtherComponent>();
	for(scum::ID id : search)
	{
		manager.queueDestroy(id); // queue entity for destruction
	}
	
	manager.processQueues(); // apply queued destructions
	return 0;
}
```

## License
scumECS is released under the MIT License. It also contains code from Tessil's
Robin Map, which is released under the MIT License. Both licenses are included
in the LICENSE file.
