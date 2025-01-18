#pragma once

#include "Singleton/Singleton.h"

namespace oxygen
{
	struct InputManager : SingletonBase<InputManager>
	{
		auto GetMousePosition() const -> const oxyVec2
		{
			return {m_mouseX, m_mouseY};
		}
		auto GetMouseDelta() const -> const oxyVec2
		{
			return {m_mouseDeltaX, m_mouseDeltaY};
		}
		auto IsKeyDown(KeyboardButton key) const -> oxyBool
		{
			return m_currentKeyStates[static_cast<size_t>(key)];
		}
		auto WasKeyDown(KeyboardButton key) const -> oxyBool
		{
			return m_previousKeyStates[static_cast<size_t>(key)];
		}
		auto IsMouseButtonDown(MouseButton button) const -> oxyBool
		{
			return m_currentMouseStates[static_cast<size_t>(button)];
		}
		auto WasMouseButtonDown(MouseButton button) const -> oxyBool
		{
			return m_previousMouseStates[static_cast<size_t>(button)];
		}

		auto IsControllerConnected(oxyU8 controller) const -> oxyBool
		{
			OXYCHECK(controller < k_maxControllers);
			return m_controllerConnected[controller];
		}

		auto IsControllerButtonDown(oxyU8 controller,
									ControllerButton button) const -> oxyBool
		{
			OXYCHECK(controller < k_maxControllers);
			return m_currentControllerStates[controller]
											[static_cast<size_t>(button)];
		}
		auto WasControllerButtonDown(oxyU8 controller,
									 ControllerButton button) const -> oxyBool
		{
			OXYCHECK(controller < k_maxControllers);
			return m_previousControllerStates[controller]
											 [static_cast<size_t>(button)];
		}
		auto GetControllerAxis(oxyU8 controller, ControllerAxis axis) const
			-> oxyF32
		{
			OXYCHECK(controller < k_maxControllers);
			return m_controllerAxisStates[controller]
										 [static_cast<size_t>(axis)];
		}
		auto GetPreviousControllerAxis(oxyU8 controller,
									   ControllerAxis axis) const -> oxyF32
		{
			OXYCHECK(controller < k_maxControllers);
			return m_previousControllerAxisStates[controller]
												 [static_cast<size_t>(axis)];
		}

		auto SetCursorLock(oxyBool lock) -> void
		{
			m_lockCursor = lock;
		}

		auto Update() -> void;

	  private:
		std::bitset<KeyboardButton_Count> m_currentKeyStates;
		std::bitset<KeyboardButton_Count> m_previousKeyStates;
		std::bitset<MouseButton_Count> m_currentMouseStates;
		std::bitset<MouseButton_Count> m_previousMouseStates;
		oxyF32 m_mouseX{};
		oxyF32 m_mouseY{};
		oxyF32 m_mouseDeltaX{};
		oxyF32 m_mouseDeltaY{};

		bool m_lockCursor{false};

		static constexpr auto k_maxControllers{4};
		std::array<std::bitset<ControllerButton_Count>, k_maxControllers>
			m_currentControllerStates;
		std::array<std::bitset<ControllerButton_Count>, k_maxControllers>
			m_previousControllerStates;
		std::array<std::array<oxyF32, ControllerAxis_Count>, k_maxControllers>
			m_controllerAxisStates;
		std::array<std::array<oxyF32, ControllerAxis_Count>, k_maxControllers>
			m_previousControllerAxisStates;
		std::array<oxyBool, k_maxControllers> m_controllerConnected;
	};
}; // namespace oxygen