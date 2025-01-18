#include "OxygenPCH.h"
#include "Pawn.h"
#include "World/World.h"
#include "Entity/Entity.h"
#include "Component/HullComponent/HullComponent.h"
#include "Component/CameraComponent/CameraComponent.h"
#include "Component/AnimatedMeshComponent/AnimatedMeshComponent.h"
#include "Component/WeaponComponent/WeaponComponent.h"
#include "Component/HealthComponent/HealthComponent.h"

#include "Net/NetSystem.h"
#include "Input/InputManager.h"
#include "Gfx/GfxRenderer.h"

#include "Platform/Platform.h"

namespace oxygen
{
	auto Pawn::Update(oxyF32 deltaTimeSeconds) -> void
	{
		auto ent = GetEntity();

		if (!m_hull)
		{
			m_hull = ent->GetComponent<HullComponent>();
			OXYCHECK(m_hull);
			if (NetSystem::GetInstance().IsHost())
			{
				m_hull->AddCollideEvent(
					GetHardRef<Pawn>(),
					[](void* ctx, HullComponent* hull, Entity* other,
					   const oxyVec3& position, const oxyVec3& normal) {
						auto pawn = static_cast<Pawn*>(ctx);
						pawn->HostHullCollideEvent(hull, other, position,
												   normal);
					});
			}
		}
		if (!m_camera)
		{
			m_camera = ent->GetComponent<CameraComponent>();
			OXYCHECK(m_camera);
		}
		if (!m_health)
		{
			m_health = ent->GetComponent<HealthComponent>();
			OXYCHECK(m_health);
			m_health->AddHealthStateChangedEvent(
				GetHardRef<Pawn>(), [](void* ctx, HealthComponent* comp,
									   Entity* ent, HealthState state) {
					auto pawn = static_cast<Pawn*>(ctx);
					pawn->HealthStateChanged(comp, ent, state);
				});
		}

		m_weaponDropHistoryClearTimer -= deltaTimeSeconds;
		if (m_weaponDropHistoryClearTimer < 0.f && m_lastDroppedWeapon)
		{
			m_lastDroppedWeapon.reset();
		}

		m_localControl = ent->GetFlag(EntityFlags_IsLocalPlayer);
		if (m_localControl)
			ParseInput();

		if (m_thirdPersonMesh)
			m_thirdPersonMesh->SetEnabled(!m_localControl);

		// Update the pawn's state
		switch (m_state)
		{
		case PawnState_Ground:
			GroundStateUpdate(deltaTimeSeconds, *ent);
			break;
		case PawnState_Void:
			VoidStateUpdate(deltaTimeSeconds, *ent);
			break;
		}
	}
	auto Pawn::Render() const -> void
	{
		if (m_localControl)
		{
			if (m_health)
			{
				const auto fmt = std::format("{}/{}", m_health->GetHealth(),
											 m_health->GetMaxHealth());
				GfxRenderer::GetInstance().OverlayText(
					fmt, 0.f, .5f, {1.f, 1.f, 1.f}, 0.025f, 0.05f, true);
			}
			if (m_equippedWeapon)
			{
				GfxRenderer::GetInstance().OverlayText(
					"YOU HAVE THE CLUB", 0.f, .45f, {1.f, 1.f, 1.f}, 0.025f, 0.05f, true);
				GfxRenderer::GetInstance().OverlayText("LEFT CLICK TO THROW", 0.f,
													   .40f, {1.f, 1.f, 1.f},
													   0.025f, 0.05f, true);
			}
		}
		
		
	}
	auto Pawn::ParseInput() -> void
	{
		// Controller
		{
			const auto x = InputManager::GetInstance().GetControllerAxis(
				0, ControllerAxis_LeftThumbY);
			const auto y = InputManager::GetInstance().GetControllerAxis(
				0, ControllerAxis_LeftThumbX);
			m_moveVector = {x, y};

			const auto lx = InputManager::GetInstance().GetControllerAxis(
				0, ControllerAxis_RightThumbY);
			const auto ly = InputManager::GetInstance().GetControllerAxis(
				0, ControllerAxis_RightThumbX);
			m_lookVector = {lx, ly};
		}
		// Keyboard
		if (m_moveVector.MagnitudeSquared() < 0.0001f)
		{
			if (InputManager::GetInstance().IsKeyDown(KeyboardButton_W))
				m_moveVector.y = 1.0f;
			if (InputManager::GetInstance().IsKeyDown(KeyboardButton_S))
				m_moveVector.y = -1.0f;
			if (InputManager::GetInstance().IsKeyDown(KeyboardButton_A))
				m_moveVector.x = -1.0f;
			if (InputManager::GetInstance().IsKeyDown(KeyboardButton_D))
				m_moveVector.x = 1.0f;
		}
		if (m_lookVector.MagnitudeSquared() < 0.0001f)
		{
			const auto md = InputManager::GetInstance().GetMouseDelta();
			const auto sens = 0.03f; // HACK: TODO: SETTINGS
			m_lookVector = {md.x * sens, md.y * sens};
		}

		// boolean inputs
		m_dropInputPressed =
			InputManager::GetInstance().IsKeyDown(KeyboardButton_Q) &&
			!InputManager::GetInstance().WasKeyDown(KeyboardButton_Q);
		m_fireInputDown =
			InputManager::GetInstance().IsMouseButtonDown(MouseButton_Left);
		m_fire2InputDown =
			InputManager::GetInstance().IsMouseButtonDown(MouseButton_Right);
		m_reloadInputDown =
			InputManager::GetInstance().IsKeyDown(KeyboardButton_R);
	}
	auto Pawn::GroundStateUpdate(oxyF32 deltaTimeSeconds, Entity& ent) -> void
	{
		if (m_localControl)
		{
			InputManager::GetInstance().SetCursorLock(true);

			const auto stanceVelocity = GetStanceVelocity();
			auto fwd = m_camera->GetCameraForward();
			fwd.z = 0.f;
			fwd = fwd.Normalized();
			auto right = m_camera->GetCameraRight();
			right.z = 0.f;
			right = right.Normalized();
			oxyVec3 velocity{};
			velocity += fwd * m_moveVector.x * stanceVelocity;
			velocity += right * m_moveVector.y * stanceVelocity;
			if (velocity.MagnitudeSquared() > 0.0001f)
				velocity = velocity.Normalized();
			velocity *= stanceVelocity;
			if (velocity.MagnitudeSquared() > 0.000001f)
			{
				velocity.z = m_hull->GetVelocity().z;
				m_hull->SetVelocity(velocity);
				// TODO: acceleration
			}

			const auto lookSensitivity = 0.1f;
			const auto look = m_lookVector * lookSensitivity;
			auto euler = m_camera->GetEuler();
			euler.x += look.y;
			euler.z -= look.x;
			// clamp pitch
			euler.x =
				std::clamp(euler.x, -Math::k_pi / 2.0f + 0.1f, Math::k_pi / 2.0f - 0.1f);
			m_camera->SetEuler(euler);

			if (m_dropInputPressed && m_equippedWeapon)
			{
				DropWeaponNetWrap();
			}
			if (m_equippedWeapon)
			{
				m_equippedWeapon->SetFireInputDown(m_fireInputDown);
				m_equippedWeapon->SetFire2InputDown(m_fire2InputDown);
				m_equippedWeapon->SetReloadInputDown(m_reloadInputDown);
			}
			if (m_rightHandEquippedWeapon)
			{
				m_rightHandEquippedWeapon->SetFireInputDown(m_fireInputDown);
				m_rightHandEquippedWeapon->SetFire2InputDown(m_fire2InputDown);
				m_rightHandEquippedWeapon->SetReloadInputDown(
					m_reloadInputDown);
			}
		}

		const auto pos = ent.GetWorldPosition();
		const auto rot = ent.GetWorldRotation();

		const auto diff = pos - m_lastGroundStateUpdatePosition;
		if (m_thirdPersonMesh->GetCurrenntAnimationHash() !=
			AnimationHash_Throw)
		{
			if (diff.MagnitudeSquared() > 0.5f)
			{
				if (m_thirdPersonMesh->GetCurrenntAnimationHash() !=
					AnimationHash_RunForward)
					m_thirdPersonMesh->BeginAnimation(AnimationHash_RunForward);
			}
			else
			{
				if (m_thirdPersonMesh->GetCurrenntAnimationHash() !=
					AnimationHash_Idle)
					m_thirdPersonMesh->BeginAnimation(AnimationHash_Idle);
			}
		}

		m_lastGroundStateUpdatePosition = pos;
	}
	auto Pawn::VoidStateUpdate(oxyF32 deltaTimeSeconds, Entity& ent) -> void
	{
		m_timeDead += deltaTimeSeconds;
		if (m_timeDead > 4.0f)
		{
			m_timeDead = 0.0f;
			if (NetSystem::GetInstance().IsHost())
			{
				m_health->Heal(100);
			}
		}
	}
	auto Pawn::GetStanceVelocity() const -> oxyF32
	{
		switch (m_stance)
		{
		case PawnStance_Stand:
			return 240.f;
		case PawnStance_Crouch:
			return 120.f;
		case PawnStance_Prone:
			return 60.f;
		}
		return 0.f;
	}
	auto Pawn::HealthStateChanged(struct HealthComponent* comp,
								  struct Entity* ent, HealthState state) -> void
	{
		if (state == HealthState_Dead)
		{
			m_state = PawnState_Void;
			if (m_thirdPersonMesh->GetCurrenntAnimationHash() !=
				AnimationHash_Dying)
			{
				m_thirdPersonMesh->BeginAnimation(AnimationHash_Dying, false);
			}
			DropWeaponNetWrap();
			if (ent->GetFlag(EntityFlags_IsLocalPlayer))
			{
				if (m_equippedWeapon)
				{
					m_equippedWeapon->SetFireInputDown(false);
					m_equippedWeapon->SetFire2InputDown(false);
					m_equippedWeapon->SetReloadInputDown(false);
				}
				if (m_rightHandEquippedWeapon)
				{
					m_rightHandEquippedWeapon->SetFireInputDown(false);
					m_rightHandEquippedWeapon->SetFire2InputDown(false);
					m_rightHandEquippedWeapon->SetReloadInputDown(false);
				}
			}
		}
		else if (state == HealthState_Alive)
		{
			SetState(PawnState_Ground);
			if (ent->GetFlag(EntityFlags_IsLocalPlayer))
			{
				// tp us to new spawnpoint
				const auto world = ent->GetWorld();
				ent->SetWorldPosition(world->RandomPlayerSpawn());
			}
			
		}
	}
	auto
	Pawn::PickupWeapon(std::shared_ptr<struct WeaponComponent> weapon) -> void
	{
		oxyObjectID id = weapon->GetObjectID();
		if (weapon->GetRightHanded())
		{
			m_rightHandEquippedWeapon = std::move(weapon);
			m_rightHandEquippedWeapon->OnPickedUp(GetHardRef<Pawn>());
		}
		else
		{
			m_equippedWeapon = std::move(weapon);
			m_equippedWeapon->OnPickedUp(GetHardRef<Pawn>());
		}

		if (NetSystem::GetInstance().IsHost())
		{
			std::vector<oxyU8> buffer;
			buffer.resize(sizeof(oxyObjectID) + sizeof(oxyObjectID));
			*reinterpret_cast<oxyObjectID*>(buffer.data()) = GetObjectID();
			*reinterpret_cast<oxyObjectID*>(buffer.data() +
											sizeof(oxyObjectID)) = id;
			NetSystem::GetInstance().HostSendToAll(
				NetProtoMsgType_SrvPawnPickupWeapon, buffer);
		}
	}
	auto Pawn::DropWeaponNetWrap() -> void
	{
		if (m_equippedWeapon)
		{
			if (NetSystem::GetInstance().IsHost())
			{
				std::vector<oxyU8> buffer;
				buffer.resize(sizeof(oxyObjectID));
				*reinterpret_cast<oxyObjectID*>(buffer.data()) = GetObjectID();
				NetSystem::GetInstance().HostSendToAll(
					NetProtoMsgType_SrvPawnDropWeapon, buffer);

				m_lastDroppedWeapon = m_equippedWeapon;
				m_weaponDropHistoryClearTimer = 3.f; // HEURISTIC

				DropWeaponImpl();
			}
			else if (NetSystem::GetInstance().IsClient())
			{
				std::vector<oxyU8> buffer;
				buffer.resize(sizeof(oxyObjectID));
				*reinterpret_cast<oxyObjectID*>(buffer.data()) = GetObjectID();
				NetSystem::GetInstance().CliSendToHost(
					NetProtoMsgType_CliPawnDropWeapon, buffer);
			}
		}
	}
	auto Pawn::DropWeaponImpl() -> void
	{
		if (m_equippedWeapon)
		{
			m_equippedWeapon->OnDropped();
		}
		m_equippedWeapon.reset();
	}
	auto Pawn::HostHullCollideEvent(HullComponent* hull, Entity* other,
									const oxyVec3& position,
									const oxyVec3& normal) -> void
	{

		if (!m_equippedWeapon || !m_rightHandEquippedWeapon)
		{
			const auto wpn = other->GetComponent<WeaponComponent>();
			if (wpn)
			{
				if (m_weaponDropHistoryClearTimer > 0.f &&
					wpn == m_lastDroppedWeapon)
					return;
				PickupWeapon(wpn);
			}
		}
	}
}; // namespace oxygen