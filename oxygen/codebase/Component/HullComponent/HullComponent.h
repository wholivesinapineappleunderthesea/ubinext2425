#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct HullComponent final : Component
	{
		OXYGENOBJECT(HullComponent, Component);

		template <typename... TArgs>
		auto AddCollideEvent(TArgs&&... args) -> void
		{
			m_onCollideEvent.AddCallback(std::forward<TArgs>(args)...);
		}
		template <typename... TArgs>
		auto AddBounceEvent(TArgs&&... args) -> void
		{
			m_onBounceEvent.AddCallback(std::forward<TArgs>(args)...);
		}

		auto TraceLine(const oxyVec3& start, const oxyVec3& end,
					   oxyVec3& outPosition,
					   oxyVec3& outNormal) const -> oxyBool;
		auto CollidesWithHull(const oxyVec3& otherHullWorldPosition,
							  CollisionHull otherHull, oxyVec3& outPosition,
							  oxyVec3& outNormal) const -> oxyBool;
		auto IsWithinRadius(const oxyVec3& position,
							oxyF32 radius) const -> oxyBool;

		auto DoesIgnoreEntity(const struct Entity* entity) const -> oxyBool;

		auto GetHull() const -> CollisionHull
		{
			return m_hull;
		}
		auto GetVelocity() const -> const oxyVec3&
		{
			return m_velocity;
		}
		auto GetGravityPerSecond() const -> oxyF32
		{
			return m_gravityPerSecond;
		}
		auto GetDrag() const -> oxyF32
		{
			return m_drag;
		}
		auto GetSolidToOtherHulls() const -> oxyBool
		{
			return m_solid;
		}
		auto GetBounceVelocityMultiplier() const -> oxyF32
		{
			return m_bounceVelocityMultiplier;
		}
		auto GetResponseType() const -> oxyBool
		{
			return m_response;
		}

		auto SetHull(CollisionHull hull) -> void
		{
			m_hull = hull;
		}
		auto SetVelocity(const oxyVec3& velocity) -> void
		{
			m_velocity = velocity;
		}
		auto SetGravityPerSecond(oxyF32 gravityPerSecond) -> void
		{
			m_gravityPerSecond = gravityPerSecond;
		}
		auto SetDrag(oxyF32 drag) -> void
		{
			m_drag = drag;
		}
		auto SetSolidToOtherHulls(oxyBool solid) -> void
		{
			m_solid = solid;
		}
		auto
		SetBounceVelocityMultiplier(oxyF32 bounceVelocityMultiplier) -> void
		{
			m_bounceVelocityMultiplier = bounceVelocityMultiplier;
		}
		auto SetResponse(CollisionResponseType response) -> void
		{
			m_response = response;
		}
		auto
		AddToIgnoreList(const std::shared_ptr<struct Entity>& entity) -> void;

	  protected:
		auto Update(oxyF32 deltaTimeSeconds) -> void override;
		//auto Render() const -> void override;

	  private:
		auto UpdateSlide(oxyF32 deltaTimeSeconds) -> void;
		auto UpdateBounce(oxyF32 deltaTimeSeconds) -> void;

		auto ClipToHullsAndUpdateWorldPosition(
			const oxyVec3& position, const oxyVec3& newPosition,
			const struct World* world, struct Entity* self) -> void;

		// HullComponent* this hull
		// Entity* other hull entity
		// const oxyVec3& position of the collision
		// const oxyVec3& normal of the collision (magnitude equal to
		// penetration distance)
		CallbackList<void, HullComponent*, struct Entity*, const oxyVec3&,
					 const oxyVec3&>
			m_onCollideEvent;
		// HullComponent* this hull
		// Entity* other hull entity
		// const oxyVec3& position of the bounce
		CallbackList<void, HullComponent*, struct Entity*, const oxyVec3&>
			m_onBounceEvent;

		CollisionHull m_hull;
		oxyVec3 m_velocity{};
		oxyF32 m_gravityPerSecond{};
		oxyF32 m_drag{};
		oxyBool m_solid{true};
		oxyF32 m_bounceVelocityMultiplier{1.f};
		CollisionResponseType m_response{
			CollisionResponseType::CollisionResponseType_Bounce};
		std::vector<std::weak_ptr<const struct Entity>> m_ignoreEntities;
	};
}; // namespace oxygen