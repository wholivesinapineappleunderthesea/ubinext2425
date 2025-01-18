#include "OxygenPCH.h"
#include "WeaponComponent.h"

#include "Entity/Entity.h"

#include "Component/Pawn/Pawn.h"
#include "Component/HullComponent/HullComponent.h"
#include "Component/CameraComponent/CameraComponent.h"

#include "GameManager/GameManager.h"
#include "Net/NetSystem.h"

namespace oxygen
{
	auto WeaponComponent::Update(oxyF32 deltaTimeSeconds) -> void
	{
		m_timeSinceLastShot += deltaTimeSeconds;
		const auto firing =
			m_rightHanded ? m_fire2InputDown : m_fireInputDown;
		if (m_timeSinceLastShot > 60.f / m_rpm && firing &&
			!m_reloading)
		{
			m_timeSinceLastShot = 0.0f;
			Fire();
		}
		if (m_reloading)
		{
			m_reloadTimer += deltaTimeSeconds;
			if (m_reloadTimer > m_timeToReload)
			{
				ReloadEnded();
			}
		}
		if (m_reloadInputDown && !m_reloading && m_clipAmmo < m_maxClipAmmo)
		{
			BeginReload();
		}
	}
	auto WeaponComponent::OnPickedUp(std::shared_ptr<Pawn> pawn) -> void
	{
		auto ent = GetEntity();
		if (pawn)
		{
			const auto ownerent = pawn->GetEntity();
			ent->SetParent(ownerent);
			ent->SetFlag(EntityFlags_EnableTransformReplication, false);
			ent->SetFlag(EntityFlags_EnableTransformInterpolation, false);
		}
		m_owner = std::move(pawn);

		const auto hull = ent->GetComponent<HullComponent>();
		if (hull)
		{
			hull->SetEnabled(false);
		}

		ResetStateAndTimers();
	}

	auto WeaponComponent::OnDropped() -> void
	{
		m_owner.reset();
		auto ent = GetEntity();
		ent->SetParent(nullptr);
		ent->SetFlag(EntityFlags_EnableTransformReplication, true);
		ent->SetFlag(EntityFlags_EnableTransformInterpolation, true);

		const auto hull = ent->GetComponent<HullComponent>();
		if (hull)
		{
			hull->SetEnabled(true);
		}

		ResetStateAndTimers();
	}

	auto WeaponComponent::SetFireInputDown(oxyBool pressed) -> void
	{
		m_fireInputDown = pressed;
	}

	auto WeaponComponent::SetFire2InputDown(oxyBool pressed) -> void
	{
		m_fire2InputDown = pressed;
	}

	auto WeaponComponent::SetReloadInputDown(oxyBool pressed) -> void
	{
		m_reloadInputDown = pressed;
	}

	auto WeaponComponent::FireInDirectionFromPos(const oxyVec3& euler,
												 const oxyVec3& pos) -> void
	{
		// Decrease clip ammo by bps
		const auto bulletsToFire = m_infiniteClip
									   ? m_bulletsPerShot
									   : std::min(m_clipAmmo, m_bulletsPerShot);
		if (!m_infiniteClip)
			m_clipAmmo -= bulletsToFire;

		auto pawn = m_owner.lock();

		for (oxyU32 i = 0; i < bulletsToFire; ++i)
		{
			const auto rndspr = RandomSpreadAngles();
			const auto rndeuler = euler + oxyVec3{rndspr.y, 0.0f, rndspr.x};
			const auto rndquat = Math::EulerAnglesToQuat(euler);

			if (m_fireType == WeaponFireType_GolfClub)
			{
				auto proj = GameManager::GetInstance().HostSummonEntity(
					EntitySpawnType_Golfclub, pos, rndquat);
				if (proj)
				{
					const auto projhull = proj->GetComponent<HullComponent>();
					if (projhull)
					{
						projhull->SetVelocity(Math::EulerForward(rndeuler) *
											  1250.f);
						if (pawn)
						{
							projhull->AddToIgnoreList(pawn->GetEntity());
						}
					}
				}
			}
			else if (m_fireType == WeaponFireType_GolfBall)
				{
				auto proj = GameManager::GetInstance().HostSummonEntity(
					EntitySpawnType_Golfball, pos, rndquat);
				if (proj)
				{
					const auto projhull = proj->GetComponent<HullComponent>();
					if (projhull)
					{
						projhull->SetVelocity(Math::EulerForward(rndeuler) *
											  1250.f);
						if (pawn)
						{
							projhull->AddToIgnoreList(pawn->GetEntity());
						}
					}
				}
				}
		}

		if (m_destroyOnFire)
		{
			if (pawn)
			{
				pawn->DropWeaponNetWrap();
			}

			auto ent = GetEntity();
			ent->SetFlag(EntityFlags_Disabled, true);

			// send destroy msg
			std::vector<oxyU8> data;
			data.resize(sizeof(oxyObjectID));
			*reinterpret_cast<oxyObjectID*>(data.data()) = ent->GetObjectID();
			NetSystem::GetInstance().HostSendToAll(
				NetProtoMsgType_SrvEntityDestroy, data);

			// destroy
			ent->Destroy();
		}
	}

	auto WeaponComponent::Fire() -> void
	{
		if (m_clipAmmo == 0)
		{
			BeginReload();
			return;
		}

		const auto pawn = m_owner.lock();
		if (!pawn)
			return;
		const auto pawnent = pawn->GetEntity();
		if (!pawnent)
			return;
		const auto cam = pawnent->GetComponent<CameraComponent>();
		if (!cam)
			return;
		const auto camEuler = cam->GetEuler();
		const auto camPos =
			pawnent->GetWorldPosition() + cam->GetCameraLocalOffset();

		if (NetSystem::GetInstance().IsHost())
		{
			FireInDirectionFromPos(camEuler, camPos);
		}
		else if (NetSystem::GetInstance().IsClient())
		{
			// Decrease clip ammo by bps
			const auto bulletsToFire =
				m_infiniteClip ? m_bulletsPerShot
							   : std::min(m_clipAmmo, m_bulletsPerShot);
			if (!m_infiniteClip)
				m_clipAmmo -= bulletsToFire;

			// TODO: 
			// why am i using vector here when it takes a span
			// cant this just be on the stack
			std::vector<oxyU8> data;
			data.resize(sizeof(oxyVec3) + sizeof(oxyVec3) + sizeof(oxyU8));
			*reinterpret_cast<oxyVec3*>(data.data()) = camEuler;
			*reinterpret_cast<oxyVec3*>(data.data() + sizeof(oxyVec3)) = camPos;
			*reinterpret_cast<oxyU8*>(data.data() + sizeof(oxyVec3) +
									  sizeof(oxyVec3)) = m_rightHanded;
			NetSystem::GetInstance().CliSendToHost(
				NetProtoMsgType_CliLocalPlayerFireWeapon, data);
		}
	}

	auto WeaponComponent::RandomSpreadAngles() const -> oxyVec2
	{
		const auto x = RandomF32(-m_spreadRadians.x, m_spreadRadians.x);
		const auto y = RandomF32(-m_spreadRadians.y, m_spreadRadians.y);
		return {x, y};
	}

	auto WeaponComponent::BeginReload() -> void
	{
		m_reloading = true;
		m_reloadTimer = 0.0f;
	}

	auto WeaponComponent::ReloadEnded() -> void
	{
		m_reloading = false;
		if (m_infiniteReserve)
		{
			m_clipAmmo = m_maxClipAmmo;
		}
		else
		{
			const auto needed = m_maxClipAmmo - m_clipAmmo;
			if (m_reserveAmmo >= needed)
			{
				m_clipAmmo = m_maxClipAmmo;
				m_reserveAmmo -= needed;
			}
			else
			{
				m_clipAmmo += m_reserveAmmo;
				m_reserveAmmo = 0;
			}
		}
	}

	auto WeaponComponent::ResetStateAndTimers() -> void
	{
		m_fireInputDown = false;
		m_reloadInputDown = false;
	}

} // namespace oxygen