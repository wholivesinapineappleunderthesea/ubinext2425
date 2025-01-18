#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct ProjectileComponent final : Component
	{
		OXYGENOBJECT(ProjectileComponent, Component);

		auto SetBouncesLeft(oxyS32 bounces) -> void
		{
			m_bouncesRemaining = bounces;
		}
		auto SetDamage(oxyF32 damage) -> void
		{
			m_damage = damage;
		}
		auto SetDamageRadius(oxyF32 radius) -> void
		{
			m_damageRadius = radius;
		}

	  protected:
		auto Update(oxyF32 deltaTimeSeconds) -> void override;

	  private:
		auto SetHull(std::shared_ptr<struct HullComponent> hull) -> void;
		friend struct GameManager; // ^^^^

		auto OnBounce(struct HullComponent* hull, struct Entity* other,
					  const oxyVec3& position) -> void;

		auto Explode() -> void;

		std::shared_ptr<struct HullComponent> m_hull;
		oxyS32 m_bouncesRemaining;
		oxyF32 m_damage;
		oxyF32 m_damageRadius;
		oxyVec3 m_spinEuler{10.f, 0.f, 0.f};
	};
} // namespace oxygen