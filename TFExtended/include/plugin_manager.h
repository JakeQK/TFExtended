#pragma once
#include "process.h"
#include "log.h"

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

	void AddAllPlugins();

	void RemoveAllPlugins();

	void EnablePlugin(const std::vector<Plugin>::iterator& it);

	void EnablePlugin(Plugin plugin);

	void DisablePlugin(Plugin plugin);

	void DisablePlugin(const std::vector<Plugin>::iterator& it);

public:
	std::vector<Plugin> m_plugins;
private:
	std::string pluginsDirectory = ".\\plugins";
};
