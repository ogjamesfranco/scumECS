#include "scumECS/ECS.h"
#include <string>

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
	scum::Manager manager;
	for(int i = 0; i < 100; i++)
	{
		auto id = manager.newID();
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
	for(auto id : search)
	{
		auto* cmp = manager.get<String>(id);
		if(cmp->text != "fizzbuzz")
		{
			return -1;
		}
	}
}
