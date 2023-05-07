#include "pch.h"
#include "menu.h"

Menu::Menu(std::shared_ptr<PluginManager>& pluginManager) : m_pluginManager(pluginManager)
{

}

Menu::~Menu()
{

}

void Menu::Render()
{
	// Set up ImGui window flags and get IO
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
	ImGuiIO& io = ImGui::GetIO();

	// Return if menu is closed
	if (!m_isMenuOpen)
		return;

	// Set window properties
	ImGui::Begin("TFExtended", &m_isMenuOpen, flags);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
	ImGui::PushItemWidth(-140);

	// Create tab bar
	ImGui::BeginTabBar("#tabs");

	// Plugin Manager tab
	if (ImGui::BeginTabItem("Plugin Manager"))
	{
		if (m_pluginManager->plugins.empty())
		{
			// No plugins loaded
			ImGui::Text("No plugins currently loaded");
			if (ImGui::Button("Load Plugins"))
			{
				m_pluginManager->LoadAllPlugins();
			}
		}
		else
		{
			// Start a table to display loaded plugins
			if (ImGui::BeginTable("PluginsTable", 2)) 
			{
				ImGui::TableSetupColumn("Address"); // Set up column 1
				ImGui::TableSetupColumn("Plugin");	// Set up column 2
				ImGui::TableHeadersRow();			// Set up header row

				// Iterate over all loaded plugins
				for (auto& pair : m_pluginManager->plugins)
				{
					ImGui::TableNextRow();							// Move to next row
					ImGui::TableNextColumn();						// Move to column 1
					ImGui::Text("0x%08X", pair.first);				// Display plugin address in hex format
					ImGui::TableNextColumn();						// Move to column 2
					ImGui::Text("%s", pair.second->name.c_str());	// Display plugin name
				}
				ImGui::EndTable();
			}

			// TODO: Fix 'RemoveAllPlugins' bug -> plugin_manager.cpp

			/*
			if (ImGui::Button("Unload all plugins"))
			{
				m_pluginManager->RemoveAllPlugins();
			}
			*/
		}

		// End Plugin Manager tab
		ImGui::EndTabItem();
	}

	// Debug tab
	if (ImGui::BeginTabItem("Debug"))
	{
		// TODO: Add debug information.

		// End Debug tab
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();


	ImGui::PopItemWidth();
	ImGui::PopItemWidth();
	// End window
	ImGui::End();
}
