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
