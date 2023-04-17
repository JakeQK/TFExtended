#include "pch.h"
#include "PluginManager.h"

PluginManager::PluginManager() : process()
{
}

PluginManager::~PluginManager()
{
}

void PluginManager::LoadPlugin(std::string path)
{
	fs::path fspath = path;
	if (fs::exists(fspath))
	{
		Plugin plugin;
		DWORD processID = process.GetProcessID(L"trials_fusion.exe");
		plugin.handle = process.LoadLibraryRemotely(processID, path.c_str());
		plugin.loaded = true;
		plugin.path = path;
		plugin.name = fspath.stem().string();

		m_plugins.push_back(plugin);
	}
}

void PluginManager::LoadPlugin(Plugin plugin)
{
	if (!plugin.loaded)
	{
		DWORD processID = process.GetProcessID(L"trials_fusion.exe");
		plugin.handle = process.LoadLibraryRemotely(processID, plugin.path.c_str());
		plugin.loaded = true;
	}
}

void PluginManager::LoadAllPlugins()
{
	fs::create_directories(m_pluginDirectory);

	for (fs::directory_entry entry : fs::recursive_directory_iterator(m_pluginDirectory))
	{
		if (!entry.is_regular_file())
			continue;

		fs::path extension = entry.path().stem().string();
		if (extension != ".dll")
			continue;

		LoadPlugin(entry.path().string());

		Sleep(m_loadDelay);
	}
}
