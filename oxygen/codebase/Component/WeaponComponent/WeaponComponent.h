#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct WeaponComponent final : Component
	{
		OXYGENOBJECT(WeaponComponent, Component);

		auto HasInfiniteReserve() const -> oxyBool
		{
			return m_infiniteReserve;
		}
		auto HasInfiniteClip() const -> oxyBool
		{
			return m_infiniteClip;
		}
		auto GetReserveAmmo() const -> oxyU32
		{
			return m_reserveAmmo;
		}
		auto GetClipAmmo() const -> oxyU32
		{
			return m_clipAmmo;
		}
		auto GetMaxClipAmmo() const -> oxyU32
		{
			return m_maxClipAmmo;
		}
		auto GetMaxReserveAmmo() const -> oxyU32
		{
			return m_maxReserveAmmo;
		}
		auto GetBulletsPerShot() const -> oxyU32
		{
			return m_bulletsPerShot;
		}
		auto GetRPM() const -> oxyU32
		{
			return m_rpm;
		}
		auto GetTimeToReload() const -> oxyF32
		{
			return m_timeToReload;
		}
		auto GetSpreadRadians() const -> oxyVec2
		{
			return m_spreadRadians;
		}
		auto GetFireType() const -> WeaponFireType
		{
			return m_fireType;
		}
		auto GetDestroyOnFire() const -> oxyBool
		{
			return m_destroyOnFire;
		}
		auto GetRightHanded() const -> oxyBool
		{
			return m_rightHanded;
		}
		auto GetCanDrop() const -> oxyBool
		{
			return m_canDrop;
		}

		auto SetInfiniteReserve(oxyBool infinite) -> void
		{
			m_infiniteReserve = infinite;
		}
		auto SetInfiniteClip(oxyBool infinite) -> void
		{
			m_infiniteClip = infinite;
		}
		auto SetReserveAmmo(oxyU32 ammo) -> void
		{
			m_reserveAmmo = ammo;
		}
		auto SetClipAmmo(oxyU32 ammo) -> void
		{
			m_clipAmmo = ammo;
		}
		auto SetMaxClipAmmo(oxyU32 ammo) -> void
		{
			m_maxClipAmmo = ammo;
		}
		auto SetMaxReserveAmmo(oxyU32 ammo) -> void
		{
			m_maxReserveAmmo = ammo;
		}
		auto SetBulletsPerShot(oxyU32 bullets) -> void
		{
			m_bulletsPerShot = bullets;
		}
		auto SetRPM(oxyU32 rpm) -> void
		{
			m_rpm = rpm;
		}
		auto SetTimeToReload(oxyF32 time) -> void
		{
			m_timeToReload = time;
		}
		auto SetSpreadRadians(oxyVec2 spread) -> void
		{
			m_spreadRadians = spread;
		}
		auto SetFireType(WeaponFireType type) -> void
		{
			m_fireType = type;
		}
		auto SetDestroyOnFire(oxyBool destroy) -> void
		{
			m_destroyOnFire = destroy;
		}
		auto SetRightHanded(oxyBool right) -> void
		{
			m_rightHanded = right;
		}
		auto SetCanDrop(oxyBool canDrop) -> void
		{
			m_canDrop = canDrop;
		}

	  protected:
		auto Update(oxyF32 deltaTimeSeconds) -> void override;

	  private:
		oxyBool m_infiniteReserve{false};
		oxyBool m_infiniteClip{false};
		oxyU32 m_reserveAmmo{0};
		oxyU32 m_clipAmmo{0};
		oxyU32 m_maxClipAmmo{0};
		oxyU32 m_maxReserveAmmo{0};
		oxyU32 m_bulletsPerShot{1};
		oxyU32 m_rpm{60};
		oxyF32 m_timeToReload{};
		oxyVec2 m_spreadRadians{};
		WeaponFireType m_fireType{WeaponFireType_Count};
		oxyBool m_destroyOnFire{};
		oxyBool m_rightHanded{};
		oxyBool m_canDrop{};

		std::weak_ptr<struct Pawn> m_owner;

		oxyBool m_fireInputDown{};
		oxyBool m_fire2InputDown{};
		oxyBool m_reloadInputDown{};

		oxyF32 m_timeSinceLastShot{};
		oxyF32 m_reloadTimer{};

		oxyBool m_reloading{};

		

		oxyVec3 m_weaponFireDirectionEuler{};

		auto OnPickedUp(std::shared_ptr<struct Pawn> pawn) -> void;
		auto OnDropped() -> void;
		auto SetFireInputDown(oxyBool pressed) -> void;
		auto SetFire2InputDown(oxyBool pressed) -> void;
		auto SetReloadInputDown(oxyBool pressed) -> void;
		friend struct Pawn; // ^^^

		auto FireInDirectionFromPos(const oxyVec3& euler, const oxyVec3& pos) -> void;

		auto Fire() -> void;
		friend struct GameManager; // ^^^
		// ugh there's a lot of friends everywhere!!!!!!!!

		auto RandomSpreadAngles() const -> oxyVec2;

		auto BeginReload() -> void;
		auto ReloadEnded() -> void;

		auto ResetStateAndTimers() -> void;
	};
}; // namespace oxygen