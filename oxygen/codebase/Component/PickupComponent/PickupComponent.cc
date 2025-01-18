#include "OxygenPCH.h"
#include "PickupComponent.h"

#include "Entity/Entity.h"

#include "Component/Pawn/Pawn.h"
#include "Component/HullComponent/HullComponent.h"

#include "Net/NetSystem.h"

namespace oxygen
{
	auto PickupComponent::Pickup(std::shared_ptr<struct Pawn> pawn) -> oxyBool
	{
		OXYCHECK(pawn);
		if (!m_pickedUpBy)
		{
			m_pickedUpBy = pawn;
			const auto ent = GetEntity();
			ent->SetParent(pawn->GetEntity());

			const auto hull = ent->GetComponent<HullComponent>();
			if (hull)
			{
				hull->SetEnabled(false);
			}

			if (NetSystem::GetInstance().IsHost())
			{
				std::vector<oxyU8> buffer;
				buffer.resize(sizeof(oxyObjectID) + sizeof(oxyObjectID));
				*reinterpret_cast<oxyObjectID*>(buffer.data()) = GetObjectID();
				*reinterpret_cast<oxyObjectID*>(
					buffer.data() + sizeof(oxyObjectID)) = pawn->GetObjectID();
				NetSystem::GetInstance().HostSendToAll(
					NetProtoMsgType_SrvPickupComponentPickedUp, buffer);
			}

			return true;
		}
		return false;
	}

	auto PickupComponent::Drop() -> void
	{
		m_pickedUpBy.reset();
		const auto ent = GetEntity();
		ent->SetParent(nullptr);

		const auto hull = ent->GetComponent<HullComponent>();
		if (hull)
		{
			hull->SetEnabled(true);
		}

		if (NetSystem::GetInstance().IsHost())
		{
			std::vector<oxyU8> buffer;
			buffer.resize(sizeof(oxyObjectID));
			*reinterpret_cast<oxyObjectID*>(buffer.data()) = GetObjectID();
			NetSystem::GetInstance().HostSendToAll(
				NetProtoMsgType_SrvPickupComponentDrop, buffer);
		}

	}

} // namespace oxygen
