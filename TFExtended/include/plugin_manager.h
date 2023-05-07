#pragma once
#include "process.h"
#include "log.h"

typedef void(*callbackFunction_t)();

struct Plugin
{
	std::string name;
	callbackFunction_t presentCallback = nullptr;

	bool operator==(const Plugin& other) const
	{
		if (this->name == other.name
			&& this->presentCallback == other.presentCallback)
			return true;
		return false;
	}
};

typedef std::map<HMODULE, std::shared_ptr<Plugin>> pluginMap;

class PluginManager
{
public:
	/// <summary>
	/// Default class constructor
	/// </summary>
	PluginManager(std::string pluginsFolder = ".\\plugins");

	/// <summary>
	/// Default class destructor
	/// </summary>
	~PluginManager() = default;

	/// <summary>
	/// Loads all plugins in our plugin folder
	/// </summary>
	void LoadAllPlugins();

	/// <summary>
	/// Calls unregisterPlugin on all plugins in this PluginManager instance
	/// </summary>
	void RemoveAllPlugins();

	/// <summary>
	/// Registers plugins in this PluginManager instance.
	/// This method is exported in dllmain for plugins to call on DLL_PROCESS_ATTACH.
	/// </summary>
	/// <param name="hModule">Plugin HMODULE</param>
	/// <param name="name">Plugin Name</param>
	/// <param name="presentCallback">Plugin present callback function for rendering ImGui</param>
	void registerPlugin(HMODULE hModule, std::string name, callbackFunction_t presentCallback);
	
	/// <summary>
	/// Unregisters plugin
	/// </summary>
	/// <param name="hModule">Plugin HMODULE</param>
	void unregisterPlugin(HMODULE hModule);

public:
	pluginMap plugins;
private:
	std::string m_pluginsDirectory;
};
