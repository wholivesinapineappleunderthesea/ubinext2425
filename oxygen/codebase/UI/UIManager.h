#pragma once

#include "Singleton/Singleton.h"

namespace oxygen
{
	struct UIManager : SingletonBase<UIManager>
	{
		auto Render() const -> void;
		auto Update() -> void;

		auto DisplayPopup(std::string message) -> void;

	  private:
		enum SelectableUIElements
		{
			SelectableUIElements_None,
			SelectableUIElements_Back,
			SelectableUIElements_Host,
			SelectableUIElements_Join,
			SelectableUIElements_Options,
			SelectableUIElements_Quit,

			SelectableUIElements_HostListSelection,

			SelectableUIElements_Count,
		};

		oxyS32 m_width{};
		oxyS32 m_height{};

		oxyBool m_mainMenuOpen{true};
		std::vector<std::string> m_popups{};

			int m_hoverHostSelectionIndex{-1};
		SelectableUIElements m_hoverItem{SelectableUIElements_None};
		std::bitset<SelectableUIElements_Count> m_selectedItems{};

		auto MousePosNDC() const -> oxyVec2;

		auto DrawMainMenu() const -> void;

		auto RefreshHostList() -> void;

		auto MainMenuItemSelected() const -> oxyBool
		{
			return m_selectedItems.test(SelectableUIElements_Host) ||
				   m_selectedItems.test(SelectableUIElements_Join) ||
				   m_selectedItems.test(SelectableUIElements_Options) ||
				   m_selectedItems.test(SelectableUIElements_Quit);
		}

		auto HoverTextColour(SelectableUIElements expect) const -> oxyVec3;
	};
} // namespace oxygen