#include "GRUBManupulation.h"
#include <stdint.h>
#include <string.h>
#include <MultiBoot.h>
#include <SystemAPI.h>

extern BootParams g_bootParams;

BootModule* FindFileFromMemory(const char* moduleName)
{
	uint32_t mods_count = g_bootParams._moduleCount;
	uint32_t mods_addr = (uint32_t)g_bootParams.Modules;

	for (uint32_t mod = 0; mod < mods_count; mod++)
	{
		BootModule* module = (BootModule*)(mods_addr + (mod * sizeof(BootModule)));

		const char* module_string = (const char*)module->Name;

		if (strcmp(module_string, moduleName) == 0)
		{
			return module;
		}

		/*if (strlen(module_string) > 1)
		{
			if (strcmp(module_string + 1, moduleName) == 0)
			{
				return module;
			}
		}*/
	}

	return nullptr;
}

void PrintMemoryFileList()
{
	uint32_t mods_count = g_bootParams._moduleCount;
	uint32_t mods_addr = (uint32_t)g_bootParams.Modules;

	for (uint32_t mod = 0; mod < mods_count; mod++)
	{
		Module* module = (Module*)(mods_addr + (mod * sizeof(Module)));

		const char* module_string = (const char*)module->Name;

		kDebugPrint(" %s\n", module_string);
	}
}