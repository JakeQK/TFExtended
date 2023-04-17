#pragma once
#include "Process.h"

struct Plugin
{
	HMODULE handle;
	std::string name;
	std::string path;
	bool loaded;
};


class PluginManager
{
public:
	// Constructor
	PluginManager();

	// Destructor
	~PluginManager();

	void LoadPlugin(std::string path);

	void UnloadPlugin(HMODULE handle);

	void LoadPlugin(Plugin plugin);

	void UnloadPlugin(Plugin plugin);

	void LoadAllPlugins();

	void UnloadAllPlugins();
public:
	std::vector<Plugin> m_plugins;

private:
	Process process;
	std::string m_pluginDirectory = ".\\plugins";
	const int m_loadDelay = 100;
};
