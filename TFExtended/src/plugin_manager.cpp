#include "pch.h"
#include "plugin_manager.h"

PluginManager::PluginManager()
{
}

void PluginManager::AddAllPlugins()
{
	const std::string pluginSearchQuery = pluginsDirectory + "\\*.dll";
	WIN32_FIND_DATAA fileData;
	HANDLE fileHandle = FindFirstFileA(pluginSearchQuery.c_str(), &fileData);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			const std::string pluginPath = pluginsDirectory + "\\" + fileData.cFileName;
			TFE_INFO("Loading \"{}\"", fileData.cFileName);

			if (HMODULE module = LoadLibraryA(pluginPath.c_str()))
			{
				char buffer[64];
				snprintf(buffer, 64, "\tLoaded \"%s\" => 0x%08X", fileData.cFileName, module);
				TFE_INFO(buffer);
			}
			else
			{
				TFE_ERROR("\tFailed to load");
			}

		} while (FindNextFileA(fileHandle, &fileData));

		FindClose(fileHandle);
	}
}

void PluginManager::RemoveAllPlugins()
{
	TFE_INFO("Removing all plugins...");
	for (const auto& plugin : m_plugins)
	{
		DisablePlugin(plugin);
	}

	m_plugins = std::vector<Plugin>();
	TFE_INFO("Done");
}

void PluginManager::EnablePlugin(const std::vector<Plugin>::iterator& it)
{
	if (!it->enabled)
	{
		it->handle = LoadLibraryA(it->path.c_str());
		it->enabled = true;
	}
}

void PluginManager::EnablePlugin(Plugin plugin)
{
	if (!plugin.enabled)
	{
		auto it = std::find(m_plugins.begin(), m_plugins.end(), plugin);

		if (it == m_plugins.end())
			return;

		EnablePlugin(it);
	}
}

void PluginManager::DisablePlugin(Plugin plugin)
{
	std::vector<Plugin>::iterator it = std::find(m_plugins.begin(), m_plugins.end(), plugin);

	if (it == m_plugins.end())
		return;

	DisablePlugin(it);
}

void PluginManager::DisablePlugin(const std::vector<Plugin>::iterator& it)
{
	if (it->enabled)
	{
		FreeLibrary(it->handle);
		it->enabled = false;
	}
}