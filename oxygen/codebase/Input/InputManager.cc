#include "OxygenPCH.h"
#include "InputManager.h"

#include "Platform/Platform.h"

namespace oxygen
{
	auto InputManager::Update() -> void
	{
		m_previousKeyStates = m_currentKeyStates;
		m_previousMouseStates = m_currentMouseStates;

		InputAbstraction::GetKeyStates(m_currentKeyStates);
		InputAbstraction::GetMouseStates(m_currentMouseStates);
		oxyF32 mx, my;
		InputAbstraction::GetMousePosition(mx, my);

		if (m_lockCursor)
		{
			m_mouseDeltaX = mx - m_mouseX;
			m_mouseDeltaY = my - m_mouseY;
			InputAbstraction::HideAndLockCursor(m_lockCursor);
			InputAbstraction::GetMousePosition(m_mouseX, m_mouseY);
		}
		else
		{
			InputAbstraction::HideAndLockCursor(m_lockCursor);
			m_mouseDeltaX = mx - m_mouseX;
			m_mouseDeltaY = my - m_mouseY;
			m_mouseX = mx;
			m_mouseY = my;
		}

		

		m_previousControllerStates = m_currentControllerStates;
		m_previousControllerAxisStates = m_controllerAxisStates;
		for (auto i = 0; i < k_maxControllers; ++i)
		{
			m_controllerConnected[i] =
				InputAbstraction::GetControllerConnected(i);
			if (!m_controllerConnected[i])
				continue;

			InputAbstraction::GetControllerStates(i,
												  m_currentControllerStates[i]);
			InputAbstraction::GetControllerAxisStates(
				i, m_controllerAxisStates[i]);
		}
	}
} // namespace oxygen