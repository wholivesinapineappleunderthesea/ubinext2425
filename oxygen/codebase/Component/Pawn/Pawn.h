#pragma once

#include "Component/Component.h"

namespace oxygen
{
	enum PawnState
	{
		PawnState_Ground,
		PawnState_Void,
	};

	enum PawnStance
	{
		PawnStance_Stand,
		PawnStance_Crouch,
		PawnStance_Prone,
	};

	struct Pawn final : Component
	{
		OXYGENOBJECT(Pawn, Component);

		auto GetState() const -> PawnState
		{
			return m_state;
		}
		auto GetStance() const -> PawnStance
		{
			return m_stance;
		}

		auto GetEquippedWeapon() const
			-> const std::shared_ptr<struct WeaponComponent>&
		{
			return m_equippedWeapon;
		}

		auto GetEquippedRightHandedWeapon() const
			-> const std::shared_ptr<struct WeaponComponent>&
		{
			return m_rightHandEquippedWeapon;
		}

	  protected:
		auto Update(oxyF32 deltaTimeSeconds) -> void override;
		auto Render() const -> void override;

	  private:
		auto SetState(PawnState state) -> void
		{
			m_state = state;
		}
		auto SetStance(PawnStance stance) -> void
		{
			m_stance = stance;
		}

		auto ParseInput() -> void;

		auto GroundStateUpdate(oxyF32 deltaTimeSeconds,
							   struct Entity& ent) -> void;
		auto VoidStateUpdate(oxyF32 deltaTimeSeconds,
							 struct Entity& ent) -> void;

		auto GetStanceVelocity() const -> oxyF32;

		auto HealthStateChanged(struct HealthComponent* comp,
								struct Entity* ent, HealthState state) -> void;

		oxyBool m_localControl{};
		oxyVec2 m_moveVector{};
		oxyVec2 m_lookVector{};
		oxyBool m_dropInputPressed{};
		oxyBool m_fireInputDown{};
		oxyBool m_fire2InputDown{};
		oxyBool m_reloadInputDown{};
		oxyF32 m_timeDead{};

		PawnState m_state{PawnState_Ground};
		PawnStance m_stance{PawnStance_Stand};

		std::shared_ptr<struct WeaponComponent> m_equippedWeapon;
		std::shared_ptr<struct WeaponComponent> m_rightHandEquippedWeapon;
		std::shared_ptr<struct WeaponComponent> m_lastDroppedWeapon;
		oxyF32 m_weaponDropHistoryClearTimer{};
		oxyVec3 m_lastGroundStateUpdatePosition{};

		std::shared_ptr<struct AnimatedMeshComponent> m_thirdPersonMesh;
		std::shared_ptr<struct AnimatedMeshComponent> m_firstPersonMesh;

		std::shared_ptr<struct HullComponent> m_hull;
		std::shared_ptr<struct CameraComponent> m_camera;
		std::shared_ptr<struct HealthComponent> m_health;

		auto
		PickupWeapon(std::shared_ptr<struct WeaponComponent> weapon) -> void;
		auto DropWeaponNetWrap() -> void;
		auto DropWeaponImpl() -> void;

		auto HostHullCollideEvent(HullComponent* hull, Entity* other,
								  const oxyVec3& position,
								  const oxyVec3& normal) -> void;

		friend struct GameManager;
		friend struct WeaponComponent;
	};
}; // namespace oxygen