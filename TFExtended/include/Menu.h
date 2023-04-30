#pragma once
#include "PluginManager.h"
#include "imgui.h"

class Menu
{
public:
	Menu(std::unique_ptr<PluginManager>& pluginManager);

	~Menu();

	void Render();

	bool IsMenuOpen() const { return m_isMenuOpen; };

	void SetMenuOpen(bool value) { m_isMenuOpen = value; }

	void ToggleMenuOpen() { m_isMenuOpen = !m_isMenuOpen; }
private:
	std::unique_ptr<PluginManager>& m_pluginManager;
	bool m_isMenuOpen = false;
};