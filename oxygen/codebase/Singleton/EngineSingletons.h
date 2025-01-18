#pragma once

#include "Singleton.h"
#include "Object/ObjectManager.h"
#include "Input/InputManager.h"
#include "Gfx/GfxRenderer.h"
#include "Resources/ResourceManager.h"
#include "UI/UIManager.h"
#include "Net/NetSystem.h"
#include "GameManager/GameManager.h"

namespace oxygen
{
	struct InternalEngineSingletonsOrder
	{
		SingletonInstance<ObjectManager> m_objectManagerInstance{};
		SingletonInstance<InputManager> m_inputManagerInstance{};
		SingletonInstance<GfxRenderer> m_gfxRendererInstance{};
		SingletonInstance<ResourceManager> m_resourceManagerInstance{};
		SingletonInstance<UIManager> m_uiManagerInstance{};
		SingletonInstance<NetSystem> m_netSystemInstance{};
		SingletonInstance<GameManager> m_gameManagerInstance{};
	};

	using EngineSingletons = SingletonHolder<InternalEngineSingletonsOrder>;
}; // namespace oxygen