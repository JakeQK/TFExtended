#include "pch.h"
#include "PluginManager.h"

PluginManager::PluginManager() : m_process()
{
}

void PluginManager::LoadPlugin(fs::path path)
{
	Plugin plugin;
	DWORD processID = m_process.GetProcessID(L"trials_fusion.exe");
	plugin.handle = m_process.LoadLibraryRemotely(processID, path.string().c_str());
	plugin.enabled = true;
	plugin.path = path.string();
	plugin.name = path.stem().string();

	m_plugins.push_back(plugin);
	TFE_INFO("{} loaded", plugin.name);
}

void PluginManager::LoadAllPlugins()
{
	fs::create_directories(m_pluginDirectory);

	for (fs::directory_entry entry : fs::recursive_directory_iterator(m_pluginDirectory))
	{
		if (!entry.is_regular_file()) 
		{
			TFE_INFO("{} is not a regular file, skipping...", entry.path().string());
			continue;
		}

		std::string extension = entry.path().extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
		if (extension != ".DLL")
		{
			TFE_INFO("{} is not a *.dll, skipping...", entry.path().string());
			continue;
		}

		LoadPlugin(entry.path().string());

		Sleep(m_loadDelay);
	}

	if (m_plugins.size() == 1) 
	{
		TFE_INFO("1 plugin loaded");
	}
	else 
	{
		TFE_INFO("{} plugins loaded", (int)m_plugins.size())
	}
}

void PluginManager::UnloadAllPlugins()
{
	TFE_INFO("Unloading {} plugins...", (int)m_plugins.size());
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
		DWORD processID = m_process.GetProcessID(L"trials_fusion.exe");
		it->handle = m_process.LoadLibraryRemotely(processID, it->path.c_str());
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
		DWORD processID = m_process.GetProcessID(L"trials_fusion.exe");
		m_process.FreeLibraryRemotely(processID, it->handle);
		it->enabled = false;
	}
}