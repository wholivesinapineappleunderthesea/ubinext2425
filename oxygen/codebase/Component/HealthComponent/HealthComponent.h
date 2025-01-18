#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct HealthComponent : Component
	{
		OXYGENOBJECT(HealthComponent, Component);

		template <typename... TArgs>
		auto AddHealthStateChangedEvent(TArgs&&... args) -> void
		{
			m_healthStateChangedEvent.AddCallback(std::forward<TArgs>(args)...);
		}

		template <typename... TArgs>
		auto AddHealedEvent(TArgs&&... args) -> void
		{
			m_healedEvent.AddCallback(std::forward<TArgs>(args)...);
		}

		template <typename... TArgs>
		auto AddDamagedEvent(TArgs&&... args) -> void
		{
			m_damagedEvent.AddCallback(std::forward<TArgs>(args)...);
		}

		auto Heal(oxyS32 amount) -> void;
		auto Damage(oxyS32 amount, DamageType type) -> void;

		auto SetHealth(oxyU32 health) -> void
		{
			m_health = health;
		}
		auto SetMaxHealth(oxyU32 maxHealth) -> void
		{
			m_maxHealth = maxHealth;
		}

		auto GetHealth() const -> oxyU32
		{
			return m_health;
		}
		auto GetMaxHealth() const -> oxyU32
		{
			return m_maxHealth;
		}

		auto GetState() const -> HealthState
		{
			return m_state;
		}

	  private:
		oxyS32 m_health{};
		oxyS32 m_maxHealth{};
		HealthState m_state{};

		// HealthComponent* this
		// Entity* owner entity
		// HealthState new state
		CallbackList<void, HealthComponent*, struct Entity*, HealthState>
			m_healthStateChangedEvent;

		// HealthComponent* this
		// Entity* owner entity
		// oxyU32 amount of healing
		CallbackList<void, HealthComponent*, struct Entity*, oxyS32>
			m_healedEvent;

		// HealthComponent* this
		// Entity* owner entity
		// oxyU32 amount of damage
		CallbackList<void, HealthComponent*, struct Entity*, oxyS32>
			m_damagedEvent;

		auto HostSendHealthStateChange() -> void;
		auto ClientReceiveHealthStateChange(oxyS32 newhealth, oxyS32 newmax,
											HealthState newstate)
			-> void;
		friend struct GameManager; // ^^^^
		
	};
} // namespace oxygen