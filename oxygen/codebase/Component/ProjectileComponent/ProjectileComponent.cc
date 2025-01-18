#include "OxygenPCH.h"
#include "ProjectileComponent.h"

#include "Entity/Entity.h"
#include "World/World.h"
#include "Component/HullComponent/HullComponent.h"
#include "Component/HealthComponent/HealthComponent.h"

#include "Net/NetSystem.h"

namespace oxygen
{
	auto ProjectileComponent::Update(oxyF32 deltaTimeSeconds) -> void
	{
		// spin entity
		if (NetSystem::GetInstance().IsHost() &&
			oxyVec2{m_hull->GetVelocity()}.MagnitudeSquared() > 0.05f)
		{
			const auto ent = GetEntity();
			const auto rot = ent->GetLocalRotation();
			const auto newRot =
				Math::EulerAnglesToQuat(m_spinEuler * deltaTimeSeconds) * rot;
			ent->SetLocalRotation(newRot);
		}
	}
	auto ProjectileComponent::SetHull(
		std::shared_ptr<struct HullComponent> hull) -> void
	{
		m_hull = std::move(hull);
		if (NetSystem::GetInstance().IsHost())
		{
			// I HATE THIS so much
			m_hull->AddBounceEvent(
				GetHardRef<ProjectileComponent>(),
				[](void* self, struct HullComponent* hull, Entity* other,
				   const oxyVec3& pos) -> void {
					static_cast<ProjectileComponent*>(self)->OnBounce(
						hull, other, pos);
				});
		}
	}

	auto ProjectileComponent::OnBounce(struct HullComponent* hull,
									   Entity* other,
									   const oxyVec3& position) -> void
	{
		if (m_bouncesRemaining >= 0)
		{
			--m_bouncesRemaining;
			if (m_bouncesRemaining < 0 || other)
			{
				Explode();
			}
		}
	}

	auto ProjectileComponent::Explode() -> void
	{
		// only execs on host
		const auto& ent = GetEntity();
		ent->SetFlag(EntityFlags_Disabled, 1);

		const auto worldPos = ent->GetWorldPosition();

		const auto& entList = ent->GetWorld()->GetEntityList();
		for (oxySize i = 0; i < entList.size(); ++i)
		{
			const auto& ent = entList[i];
			if (ent.get() == GetEntity().get())
				continue;
			if (!ent->GetFlag(EntityFlags_HasHull))
				continue;
			const auto hullcomp = ent->GetComponent<HullComponent>();
			if (!hullcomp)
				continue;
			const auto health = ent->GetComponent<HealthComponent>();
			if (!health)
				continue;
			if (hullcomp->IsWithinRadius(worldPos, m_damageRadius))
			{
				health->Damage(static_cast<oxyS32>(m_damage), DamageType_Explosive);
			}
		}

		std::vector<oxyU8> data;
		data.resize(sizeof(oxyObjectID));
		*reinterpret_cast<oxyObjectID*>(data.data()) = GetEntity()->GetObjectID();
		NetSystem::GetInstance().HostSendToAll(
			NetProtoMsgType_SrvEntityDestroy, data);
		
		
		ent->Destroy();
	}

} // namespace oxygen
