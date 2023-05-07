#pragma once
#include "plugin_manager.h"
#include "imgui.h"
#include "Utility.h"

class Menu
{
public:
	/// <summary>
	/// Menu constructor
	/// </summary>
	/// <param name="pluginManager"></param>
	Menu(std::shared_ptr<PluginManager>& pluginManager);

	~Menu();

	/// <summary>
	/// Renders all our menu ImGui calls
	/// </summary>
	void Render();

	/// <summary>
	/// This method returns a boolean value indicating whether the menu is currently open
	/// </summary>
	inline bool IsMenuOpen() const { return m_isMenuOpen; };

	/// <summary>
	/// This method sets the menu open state
	/// </summary>
	/// <param name="value">TRUE: Opens Menu
	/// FALSE: Closes Menu</param>
	inline void SetMenuOpen(bool value) { m_isMenuOpen = value; }

	/// <summary>
	/// This method simply toggles the menu open state
	/// </summary>
	inline void ToggleMenuOpen() { m_isMenuOpen = !m_isMenuOpen; }
private:
	std::shared_ptr<PluginManager>& m_pluginManager;
	bool m_isMenuOpen = false;
};