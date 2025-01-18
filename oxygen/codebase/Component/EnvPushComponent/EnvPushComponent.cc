#include "OxygenPCH.h"
#include "EnvPushComponent.h"


#include "World/World.h"
#include "Entity/Entity.h"
#include "Component/HullComponent/HullComponent.h"


namespace oxygen
{
	auto EnvPushComponent::Update(oxyF32 deltaTimeSeconds) -> void 
	{
		auto ent = GetEntity();
		auto world = ent->GetWorld();
		auto localplayer = world->GetLocalPlayer().lock();
		if (localplayer)
		{
			if (localplayer->GetFlag(EntityFlags_HasHull))
			{
				const auto hull = localplayer->GetComponent<HullComponent>();
				if (hull)
				{
					const auto worldPos = ent->GetWorldPosition();
					if (hull->IsWithinRadius(worldPos, m_radius))
					{
						if (!m_isPushing)
						{
							hull->SetVelocity(hull->GetVelocity() + m_velocity);
						}
						m_isPushing = true;
					}
					else
					{
						m_isPushing = false;
					}
				}	
			}
		}
	}
} // namespace oxygen
