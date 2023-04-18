#include "pch.h"
#include "PluginManager.h"

PluginManager::PluginManager() : process(), m_pluginDirectory(".//plugins")
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
		plugin.enabled = true;
		plugin.path = path;
		plugin.name = fspath.stem().string();

		m_plugins.push_back(plugin);
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

void PluginManager::UnloadAllPlugins()
{
	for (const auto& plugin : m_plugins)
	{
		DisablePlugin(plugin);
	}

	m_plugins = std::vector<Plugin>();
}

void PluginManager::EnablePlugin(const std::vector<Plugin>::iterator& it)
{
	if (!it->enabled)
	{
		DWORD processID = process.GetProcessID(L"trials_fusion.exe");
		it->handle = process.LoadLibraryRemotely(processID, it->path.c_str());
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
		DWORD processID = process.GetProcessID(L"trials_fusion.exe");
		process.FreeLibraryRemotely(processID, it->handle);
		it->enabled = false;
	}
}