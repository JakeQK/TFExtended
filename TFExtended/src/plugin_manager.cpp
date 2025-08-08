#include "pch.h"
#include "plugin_manager.h"

PluginManager::PluginManager(std::string pluginFolder) : m_pluginsDirectory(pluginFolder)
{

}

void PluginManager::LoadAllPlugins()
{
	const std::string pluginSearchQuery = m_pluginsDirectory + "\\*.dll";

	SetDllDirectoryA(m_pluginsDirectory.c_str());

	WIN32_FIND_DATAA fileData;
	HANDLE fileHandle = FindFirstFileA(pluginSearchQuery.c_str(), &fileData);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			const std::string pluginPath = m_pluginsDirectory + "\\" + fileData.cFileName;
			TFE_INFO("Loading \"{}\"", pluginPath.c_str());

			HMODULE module = LoadLibraryA(pluginPath.c_str());
			if(!module)
			{
				DWORD error = GetLastError();
				char errorMsg[256];
				FormatMessageA(
					FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					error,
					0, errorMsg,
					sizeof(errorMsg),
					NULL
				);

				TFE_ERROR("Failed to load \"{}\": Error {}: {}",
					fileData.cFileName, error, errorMsg);
				continue;
			}

			TFE_INFO("\tLoaded \"{}\" => 0x{:08X}", fileData.cFileName, (uintptr_t)module);

		} while (FindNextFileA(fileHandle, &fileData));

		FindClose(fileHandle);
	}

	SetDllDirectoryA(NULL);
}

// BUG: 'unregisterPlugin' crashes Trials Fusion
void PluginManager::RemoveAllPlugins()
{
	if (plugins.size()) 
	{
		for (auto& pair : plugins)
		{
			unregisterPlugin(pair.first);
		}
		plugins.clear();
	}
}

void PluginManager::registerPlugin(HMODULE hModule, std::string name, callbackFunction_t presentCallback)
{
	if (plugins.find(hModule) == plugins.end())
	{
		Plugin plugin;
		plugin.name = name;
		plugin.presentCallback = presentCallback;

		plugins[hModule] = std::make_shared<Plugin>(plugin);
		TFE_INFO("Registered plugin '{}'", name);
	}
	else
	{
		TFE_ERROR("Plugin '{}' is already registered", name);
	}

}

// BUG: Crashes trials fusion
void PluginManager::unregisterPlugin(HMODULE hModule)
{
	auto pluginIt = plugins.find(hModule);
	if (pluginIt != plugins.end())
	{
		TFE_INFO("Unregistering '{}'", pluginIt->second->name);
		plugins.erase(hModule);
		FreeLibrary(hModule);
		// CloseHandle(hModule);
	}
}