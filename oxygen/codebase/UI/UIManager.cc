#include "OxygenPCH.h"
#include "UIManager.h"

#include "Gfx/GfxRenderer.h"
#include "Input/InputManager.h"
#include "Net/NetSystem.h"
#include "GameManager/GameManager.h"


namespace oxygen
{
	auto UIManager::Render() const -> void
	{
		if (m_mainMenuOpen)
		{
			DrawMainMenu();
		}
		if (m_popups.size())
		{
			// draw first popup
			const auto& popup = m_popups.front();
			// draw simple rect:
			GfxRenderer::GetInstance().OverlayRect(
				{0.5f, 0.5f, 0.5f}, {-.5f, -.5f}, {0.5f, 0.5f});
			// draw message
			GfxRenderer::GetInstance().OverlayText(
				popup, 0.f, 0.f, {1.f, 0, 0}, 0.04f, 0.04f, true);
		}
	}
	auto UIManager::Update() -> void
	{
		if (m_mainMenuOpen || m_popups.size())
		{
			InputManager::GetInstance().SetCursorLock(false);
		}

		// click
		const auto mousedownnow =
			InputManager::GetInstance().IsMouseButtonDown(MouseButton_Left);
		const auto mousedownlast =
			InputManager::GetInstance().WasMouseButtonDown(MouseButton_Left);

		if (m_popups.size())
		{
			if (mousedownnow && !mousedownlast)
			{
				m_popups.erase(m_popups.begin());
			}
			return;
		}

		// update hover item
		const auto mpndc = MousePosNDC();

		if (InputManager::GetInstance().IsKeyDown(KeyboardButton_Backtick) &&
			!InputManager::GetInstance().WasKeyDown(KeyboardButton_Backtick))
		{
			m_mainMenuOpen = !m_mainMenuOpen;
		}

		if (m_mainMenuOpen)
		{
			const auto mainmenuitemselected = MainMenuItemSelected();
			m_hoverHostSelectionIndex = -1;

			if (mpndc.x > -1.f && mpndc.x < -0.5f)
			{
				if (!mainmenuitemselected)
				{
					if (mpndc.y > 0.3f && mpndc.y < 0.5f)
						m_hoverItem = SelectableUIElements_Host;
					else if (mpndc.y > 0.1f && mpndc.y < 0.3f)
						m_hoverItem = SelectableUIElements_Join;
					else if (mpndc.y > -0.1f && mpndc.y < 0.1f)
						m_hoverItem = SelectableUIElements_Options;
					else if (mpndc.y > -0.3f && mpndc.y < -0.1f)
						m_hoverItem = SelectableUIElements_Quit;
					else
						m_hoverItem = SelectableUIElements_None;
				}
				else
				{
					m_hoverItem = SelectableUIElements_Back;
				}
			}
			else if (m_selectedItems.test(SelectableUIElements_Join))
			{
				// host list selection
				m_hoverItem = SelectableUIElements_HostListSelection;

				// find which index
				const auto list =
					NetSystem::GetInstance().CliGetDiscoveredHosts();
				for (auto i = 0; i < list.size(); ++i)
				{
					const auto y = 0.5f - i * 0.1f;
					if (mpndc.y > y - 0.05f && mpndc.y < y + 0.05f)
					{
						m_hoverHostSelectionIndex = i;
						break;
					}
				}
			}
			else
			{
				m_hoverItem = SelectableUIElements_None;
			}

			if (!mousedownnow && mousedownlast)
			{
				m_selectedItems.set(m_hoverItem);
				switch (m_hoverItem)
				{
				case SelectableUIElements_Back:
					m_selectedItems.reset(SelectableUIElements_Back);
					m_selectedItems.reset(SelectableUIElements_Host);
					m_selectedItems.reset(SelectableUIElements_Join);
					m_selectedItems.reset(SelectableUIElements_Options);
					m_selectedItems.reset(SelectableUIElements_Quit);
					break;
				case SelectableUIElements_Host:
					m_selectedItems.reset();
					GameManager::GetInstance().HostGame("oregon2");
					m_mainMenuOpen = false;
					break;
				case SelectableUIElements_Join:
					m_selectedItems.reset(SelectableUIElements_Host);
					m_selectedItems.reset(SelectableUIElements_Options);
					m_selectedItems.reset(SelectableUIElements_Quit);
					RefreshHostList();
					break;
				case SelectableUIElements_Options:
					m_selectedItems.reset(SelectableUIElements_Host);
					m_selectedItems.reset(SelectableUIElements_Join);
					m_selectedItems.reset(SelectableUIElements_Quit);
					DisplayPopup("Options not implemented");
					break;
				case SelectableUIElements_Quit:
					m_selectedItems.reset(SelectableUIElements_Host);
					m_selectedItems.reset(SelectableUIElements_Join);
					m_selectedItems.reset(SelectableUIElements_Options);
					std::exit(0);
					break;
				case SelectableUIElements_HostListSelection:
					if (m_hoverHostSelectionIndex != -1)
					{
						m_selectedItems.reset();
						const auto address =
							NetSystem::GetInstance().CliGetDiscoveredHosts()
								[m_hoverHostSelectionIndex];
						if (NetSystem::GetInstance().ConnectToHost(address))
						{
							m_mainMenuOpen = false;
						}
						
					}
					break;
				default:
					break;
				}
			}
		}
	}
	auto UIManager::DisplayPopup(std::string message) -> void
	{
		m_mainMenuOpen = true;
		m_popups.push_back(std::move(message));
	}
	auto UIManager::MousePosNDC() const -> oxyVec2
	{
		const auto width = GfxRenderer::GetInstance().GetWidth();
		const auto height = GfxRenderer::GetInstance().GetHeight();
		const auto mousePos = InputManager::GetInstance().GetMousePosition();
		return {(2.f * mousePos.x) / width - 1.f,
				(2.f * mousePos.y) / height - 1.f};
	}
	auto UIManager::DrawMainMenu() const -> void
	{
		// GfxRenderer::GetInstance().OverlayRect({0.5f, 0.5f, 0.5f}, {-.5f,
		// -.5f},
		//									   {.5f, .5f});

		GfxRenderer::GetInstance().OverlayText(
			"oxygen", -1.f, 0.9f, oxyVec3{1, 1, 1}, 0.1f, 0.1f, false);

		const auto mainmenuitemselected = MainMenuItemSelected();

		if ((mainmenuitemselected &&
			 m_selectedItems.test(SelectableUIElements_Host)) ||
			!mainmenuitemselected)
		{
			if (!m_selectedItems.test(SelectableUIElements_Host))
			{
				GfxRenderer::GetInstance().OverlayText(
					"host", -1.f, 0.4f,
					HoverTextColour(SelectableUIElements_Host), 0.08f, 0.08f,
					false);
			}
			else
			{
				GfxRenderer::GetInstance().OverlayText(
					"back", -1.f, 0.4f,
					HoverTextColour(SelectableUIElements_Back), 0.08f, 0.08f,
					false);
			}
		}

		if ((mainmenuitemselected &&
			 m_selectedItems.test(SelectableUIElements_Join)) ||
			!mainmenuitemselected)
		{
			if (!m_selectedItems.test(SelectableUIElements_Join))
			{
				GfxRenderer::GetInstance().OverlayText(
					"join", -1.f, 0.2f,
					HoverTextColour(SelectableUIElements_Join), 0.08f, 0.08f,
					false);
			}
			else
			{
				GfxRenderer::GetInstance().OverlayText(
					"back", -1.f, 0.2f,
					HoverTextColour(SelectableUIElements_Back), 0.08f, 0.08f,
					false);
			}
		}

		if ((mainmenuitemselected &&
			 m_selectedItems.test(SelectableUIElements_Options)) ||
			!mainmenuitemselected)
		{
			if (!m_selectedItems.test(SelectableUIElements_Options))
			{
				GfxRenderer::GetInstance().OverlayText(
					"options", -1.f, 0.f,
					HoverTextColour(SelectableUIElements_Options), 0.08f, 0.08f,
					false);
			}
			else
			{
				GfxRenderer::GetInstance().OverlayText(
					"back", -1.f, 0.f,
					HoverTextColour(SelectableUIElements_Back), 0.08f, 0.08f,
					false);
			}
		}

		if ((mainmenuitemselected &&
			 m_selectedItems.test(SelectableUIElements_Quit)) ||
			!mainmenuitemselected)
			GfxRenderer::GetInstance().OverlayText(
				"quit", -1.f, -.2f, HoverTextColour(SelectableUIElements_Quit),
				0.08f, 0.08f, false);


		if (m_selectedItems.test(SelectableUIElements_Join))
		{
			if (NetSystem::GetInstance().CliIsDiscoveringHosts())
			{
				GfxRenderer::GetInstance().OverlayText(
					"searching...", 0.f, 0.6f, {1, 1, 1}, 0.08f, 0.08f, true);
			}
			

			const auto list = NetSystem::GetInstance().CliGetDiscoveredHosts();
			for (auto i = 0; i < list.size(); ++i)
			{
				const auto& host = list[i];
				const auto col = m_hoverHostSelectionIndex == i
									 ? oxyVec3{1, 1, 0}
									 : oxyVec3{1, 1, 1};
				GfxRenderer::GetInstance().OverlayText(
					std::format("{}. {}", i, host), 0.f, 0.5f - i * 0.1f, col,
					0.05f, 0.05f, true);
			}

		}
	}
	auto UIManager::RefreshHostList() -> void
	{
		NetSystem::GetInstance().CliDiscoverHosts();
	}
	auto
	UIManager::HoverTextColour(SelectableUIElements expect) const -> oxyVec3
	{
		if (m_hoverItem == expect)
			return {1, 1, 0};
		return {1, 1, 1};
	}
} // namespace oxygen