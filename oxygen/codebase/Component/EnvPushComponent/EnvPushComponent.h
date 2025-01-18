#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct EnvPushComponent final : Component
	{
		OXYGENOBJECT(EnvPushComponent, Component);

		auto GetVelocity() const -> const oxyVec3&
		{
			return m_velocity;
		}
		auto GetRadius() const -> oxyF32
		{
			return m_radius;
		}

		auto SetVelocity(const oxyVec3& velocity) -> void
		{
			m_velocity = velocity;
		}
		auto SetRadius(oxyF32 radius) -> void
		{
			m_radius = radius;
		}

		protected:
		auto Update(oxyF32 deltaTimeSeconds) -> void override;

	  private:
		oxyVec3 m_velocity{};
		oxyF32 m_radius{};
		oxyBool m_isPushing{};
	};
} // namespace oxygen