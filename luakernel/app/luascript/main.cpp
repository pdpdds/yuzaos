#include "luascript.h"
#include <windef.h>
#include <stdio.h>
#include "GUIConsoleFramework.h"

int main_impl(int argc, char** argv)
{
	LuaScript* script = new LuaScript("Player.lua");
	float posX = script->get<float>("player.position.x");
	float posY = script->get<float>("player.position.y");
	std::string filename = script->get<std::string>("player.filename");
	int hp = script->get<int>("player.HP");

	std::cout << "Position X = " << posX << ", Y = " << posY << std::endl;
	std::cout << "Filename:" << filename.c_str() << std::endl;
	std::cout << "HP:" << hp << std::endl;

	// getting arrays
	std::vector<int> v = script->getIntVector("array");
	std::cout << "Contents of array:";
	for (std::vector<int>::iterator it = v.begin();
		it != v.end();
		it++) {
		printf("%d\n", *it);
		std::cout << *it << ",";
	}
	std::cout << std::endl;

	// getting table keys:
	std::vector<std::string> keys = script->getTableKeys("player");
	std::cout << "Keys of [player] table:";
	for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); it++) 
	{
		printf("%s\n", *it);
	}
	std::cout << std::endl;

	return 0;
}

int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);
}