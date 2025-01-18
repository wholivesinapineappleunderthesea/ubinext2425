#include "OxygenPCH.h"
#include "HealthComponent.h"

#include "Net/NetSystem.h"

namespace oxygen
{
	auto HealthComponent::Heal(oxyS32 amount) -> void
	{
		if (amount < 0)
			return;
		m_health += amount;
		if (m_health > m_maxHealth)
			m_health = m_maxHealth;
		if (m_health > 0 && m_state == HealthState_Dead)
		{
			m_state = HealthState::HealthState_Alive;
			m_healthStateChangedEvent.IterateCallbacks(
				[]() {}, this, GetEntity().get(), m_state);
		}
		HostSendHealthStateChange();
	}

	auto HealthComponent::Damage(oxyS32 amount, DamageType type) -> void
	{
		if (m_state == HealthState_Invulnerable || m_state == HealthState_Dead)
			return;
		if (amount < 0)
			return;

		m_health -= amount;
		if (m_health <= 0)
		{
			m_health = 0;
			m_state = HealthState::HealthState_Dead;
			m_healthStateChangedEvent.IterateCallbacks(
				[]() {}, this, GetEntity().get(), m_state);
		}
		HostSendHealthStateChange();
	}

	auto HealthComponent::HostSendHealthStateChange() -> void
	{
		if (NetSystem::GetInstance().IsHost())
		{
			/*
			// oxyObjectID: health component id
		// oxyS32: health
		// oxyS32: max health
		// oxyU8: health state
		NetProtoMsgType_SrvHealhComponentChange,
			*/
			std::vector<oxyU8> buffer;
			buffer.resize(sizeof(oxyObjectID) + sizeof(oxyS32) +
						  sizeof(oxyS32) + sizeof(oxyU8));
			*reinterpret_cast<oxyObjectID*>(buffer.data()) = GetObjectID();
			*reinterpret_cast<oxyS32*>(buffer.data() + sizeof(oxyObjectID)) =
				m_health;
			*reinterpret_cast<oxyS32*>(buffer.data() + sizeof(oxyObjectID) +
									   sizeof(oxyS32)) = m_maxHealth;
			*reinterpret_cast<oxyU8*>(buffer.data() + sizeof(oxyObjectID) +
									  sizeof(oxyS32) + sizeof(oxyS32)) =
				static_cast<oxyU8>(m_state);
			NetSystem::GetInstance().HostSendToAll(
				NetProtoMsgType_SrvHealhComponentChange, buffer);
		}
	}

	auto HealthComponent::ClientReceiveHealthStateChange(oxyS32 newhealth, oxyS32 newmax, HealthState newstate) -> void
	{
		const auto ent = GetEntity();

		if (m_health != newhealth)
		{
			const auto diff = newhealth - m_health;
			if (diff > 0)
			{
				m_healedEvent.IterateCallbacks([]() {}, this, ent.get(), diff);
			}
			else
			{
				m_damagedEvent.IterateCallbacks([]() {}, this, ent.get(),
												-diff);
			}
			m_health = newhealth;
		}

		m_maxHealth = newmax;
		if (m_state != newstate)
		{
			m_healthStateChangedEvent.IterateCallbacks([]() {}, this, ent.get(),
													   newstate);
			m_state = newstate;
		}
		
	}

} // namespace oxygen
