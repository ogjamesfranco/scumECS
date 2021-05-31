#include "scumECS.h"
#include <iostream>

using namespace scum;

struct String
{
	std::string text;
};

struct Fizz
{};

struct Buzz
{};

int main()
{
	EntityManager manager;
	for(int i = 0; i < 100; i++)
	{
		EntID id = manager.newID();
		auto* cmp = manager.add<String>(id);

		if(i % 3 == 0)
		{
			cmp->text += "fizz";
			manager.add<Fizz>(id);
		}
		if(i % 5 == 0)
		{
			cmp->text += "buzz";
			manager.add<Buzz>(id);
		}
	}

	auto search = manager.search<Fizz, Buzz>();
	for(EntID id : search)
	{
		auto* cmp = manager.get<String>(id);
		if(cmp->text != "fizzbuzz")
		{
			return -1;
		}
	}
}
