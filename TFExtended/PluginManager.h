#pragma once
#include "Process.h"

struct Plugin
{
	HMODULE handle;
	std::string name;
	std::string path;
	bool enabled;

	bool operator==(const Plugin& other) const
	{
		if (this->handle == other.handle
			&& this->name == other.name
			&& this->path == other.path
			&& this->enabled == other.enabled)
			return true;
		return false;
	}
};


class PluginManager
{
public:
	// Constructor
	PluginManager();

	// Destructor
	~PluginManager() = default;

	// Load plugin by path
	void LoadPlugin(std::string path);

	void LoadAllPlugins();

	void UnloadAllPlugins();

	void EnablePlugin(const std::vector<Plugin>::iterator& it);

	void EnablePlugin(Plugin plugin);


	void DisablePlugin(Plugin plugin);

	void DisablePlugin(const std::vector<Plugin>::iterator& it);

public:
	std::vector<Plugin> m_plugins;

private:
	Process process;
	std::string m_pluginDirectory;
	const int m_loadDelay = 100;
};
