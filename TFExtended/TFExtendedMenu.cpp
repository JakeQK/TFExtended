#include "pch.h"
#include "TFExtendedMenu.h"

TFExtendedMenu::TFExtendedMenu(std::unique_ptr<PluginManager>& pluginManager) : m_pluginManager(pluginManager)
{

}

TFExtendedMenu::~TFExtendedMenu()
{
}

// Renders the Main Menu for TFExtended
void TFExtendedMenu::Render()
{
	// Return if menu is closed
	if (!m_isMenuOpen)
		return;

	// Set window properties
	ImGui::Begin("TFExtended", &m_isMenuOpen);
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);
	ImGui::PushItemWidth(-140);

	// Create tab bar
	ImGui::BeginTabBar("#tabs");

	// Plugin Manager tab
	if (ImGui::BeginTabItem("Plugin Manager"))
	{
		if (m_pluginManager->m_plugins.empty())
		{
			// No plugins loaded
			ImGui::Text("No plugins currently loaded");
			if (ImGui::Button("Load plugins"))
			{
				m_pluginManager->LoadAllPlugins();
			}
		}
		else
		{
			// Display loaded plugins
			for (auto pluginIterator = m_pluginManager->m_plugins.begin(); pluginIterator != m_pluginManager->m_plugins.end(); pluginIterator++)
			{
				ImGui::Text("%s %s", pluginIterator->name.c_str(), pluginIterator->path.c_str());
				ImGui::SameLine();
				if (pluginIterator->enabled)
				{
					// Unload button
					if (ImGui::Button("Unload"))
						m_pluginManager->DisablePlugin(pluginIterator);
				}
				else
				{
					// Load button
					if (ImGui::Button("Load"))
						m_pluginManager->EnablePlugin(pluginIterator);
				}
			}

			// Unload all plugins button
			if (ImGui::Button("Unload all plugins"))
			{
				m_pluginManager->UnloadAllPlugins();
			}
		}
		ImGui::EndTabItem();
	}

	// Debug tab
	if (ImGui::BeginTabItem("Debug"))
	{
		// TODO: Add debug information

		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();

	// End window
	ImGui::End();
}
