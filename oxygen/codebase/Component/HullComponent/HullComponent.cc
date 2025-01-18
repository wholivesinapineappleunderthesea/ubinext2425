#include "OxygenPCH.h"
#include "HullComponent.h"
#include "Entity/Entity.h"
#include "World/World.h"

//#include "Gfx/GfxRenderer.h"

namespace oxygen
{
	auto HullComponent::TraceLine(const oxyVec3& start, const oxyVec3& end,
								  oxyVec3& outPosition,
								  oxyVec3& outNormal) const -> oxyBool
	{
		if (m_hull == CollisionHull::CollisionHull_None)
			return false;
		const auto ent = GetEntity();
		if (ent->GetFlag(EntityFlags_Disabled))
			return false;
		const auto worldPos = ent->GetWorldPosition();
		const auto worldMin =
			worldPos + k_collisionHullMins[static_cast<int>(m_hull)];
		const auto worldMax =
			worldPos + k_collisionHullMaxs[static_cast<int>(m_hull)];

		// Ray direction
		oxyVec3 direction = end - start;
		oxyVec3 invDir = {1.0f / direction.x, 1.0f / direction.y,
						  1.0f / direction.z};

		// Calculate intersection times
		float tMin = (worldMin.x - start.x) * invDir.x;
		float tMax = (worldMax.x - start.x) * invDir.x;

		if (tMin > tMax)
			std::swap(tMin, tMax);

		float tyMin = (worldMin.y - start.y) * invDir.y;
		float tyMax = (worldMax.y - start.y) * invDir.y;

		if (tyMin > tyMax)
			std::swap(tyMin, tyMax);

		if ((tMin > tyMax) || (tyMin > tMax))
			return false;

		if (tyMin > tMin)
			tMin = tyMin;
		if (tyMax < tMax)
			tMax = tyMax;

		float tzMin = (worldMin.z - start.z) * invDir.z;
		float tzMax = (worldMax.z - start.z) * invDir.z;

		if (tzMin > tzMax)
			std::swap(tzMin, tzMax);

		if ((tMin > tzMax) || (tzMin > tMax))
			return false;

		if (tzMin > tMin)
			tMin = tzMin;
		if (tzMax < tMax)
			tMax = tzMax;

		// If the intersection is valid, calculate the hit position and normal
		if (tMin < 0.0f || tMax < 0.0f)
			return false; // No intersection in the forward direction

		float tHit = tMin; // Use the nearest intersection
		outPosition = start + direction * tHit;

		// Determine the hit normal
		if (fabs(outPosition.x - worldMin.x) < 1e-4f)
			outNormal = {-1.0f, 0.0f, 0.0f};
		else if (fabs(outPosition.x - worldMax.x) < 1e-4f)
			outNormal = {1.0f, 0.0f, 0.0f};
		else if (fabs(outPosition.y - worldMin.y) < 1e-4f)
			outNormal = {0.0f, -1.0f, 0.0f};
		else if (fabs(outPosition.y - worldMax.y) < 1e-4f)
			outNormal = {0.0f, 1.0f, 0.0f};
		else if (fabs(outPosition.z - worldMin.z) < 1e-4f)
			outNormal = {0.0f, 0.0f, -1.0f};
		else if (fabs(outPosition.z - worldMax.z) < 1e-4f)
			outNormal = {0.0f, 0.0f, 1.0f};
		else
			outNormal = {0.0f, 0.0f, 0.0f}; // Default case (unlikely)

		return true;
	}
	auto HullComponent::CollidesWithHull(const oxyVec3& otherHullWorldPosition,
										 CollisionHull otherHull,
										 oxyVec3& outPosition,
										 oxyVec3& outNormal) const -> oxyBool
	{
		if (m_hull == CollisionHull_None || otherHull == CollisionHull_None)
			return false;

		const auto ent = GetEntity();
		if (ent->GetFlag(EntityFlags_Disabled))
			return false;
		const auto worldPos = ent->GetWorldPosition();

		// Get the bounds of this hull
		const auto thisMin =
			worldPos + k_collisionHullMins[static_cast<int>(m_hull)];
		const auto thisMax =
			worldPos + k_collisionHullMaxs[static_cast<int>(m_hull)];

		// Get the bounds of the other hull
		const auto otherMin = otherHullWorldPosition +
							  k_collisionHullMins[static_cast<int>(otherHull)];
		const auto otherMax = otherHullWorldPosition +
							  k_collisionHullMaxs[static_cast<int>(otherHull)];

		// Check for overlap (AABB intersection test)
		if (thisMax.x < otherMin.x || thisMin.x > otherMax.x ||
			thisMax.y < otherMin.y || thisMin.y > otherMax.y ||
			thisMax.z < otherMin.z || thisMin.z > otherMax.z)
		{
			return false; // No collision
		}

		// Compute the penetration depth on each axis
		float overlapX =
			std::min(thisMax.x, otherMax.x) - std::max(thisMin.x, otherMin.x);
		float overlapY =
			std::min(thisMax.y, otherMax.y) - std::max(thisMin.y, otherMin.y);
		float overlapZ =
			std::min(thisMax.z, otherMax.z) - std::max(thisMin.z, otherMin.z);

		// Determine the axis of minimum penetration depth
		if (overlapX < overlapY && overlapX < overlapZ)
		{
			// Collision normal is along X axis
			outNormal = (worldPos.x < otherHullWorldPosition.x)
							? oxyVec3{-overlapX, 0.0f, 0.0f}
							: oxyVec3{overlapX, 0.0f, 0.0f};
			outPosition = oxyVec3(
				(worldPos.x + otherHullWorldPosition.x) * 0.5f, 0.0f, 0.0f);
		}
		else if (overlapY < overlapZ)
		{
			// Collision normal is along Y axis
			outNormal = (worldPos.y < otherHullWorldPosition.y)
							? oxyVec3{0.0f, -overlapY, 0.0f}
							: oxyVec3{0.0f, overlapY, 0.0f};
			outPosition = oxyVec3(
				0.0f, (worldPos.y + otherHullWorldPosition.y) * 0.5f, 0.0f);
		}
		else
		{
			// Collision normal is along Z axis
			outNormal = (worldPos.z < otherHullWorldPosition.z)
							? oxyVec3{0.0f, 0.0f, -overlapZ}
							: oxyVec3{0.0f, 0.0f, overlapZ};
			outPosition = oxyVec3(
				0.0f, 0.0f, (worldPos.z + otherHullWorldPosition.z) * 0.5f);
		}

		return true; // Collision detected
	}
	auto HullComponent::IsWithinRadius(const oxyVec3& position,
									   oxyF32 radius) const -> oxyBool
	{
		if (m_hull == CollisionHull_None)
			return false;
		const auto ent = GetEntity();
		if (ent->GetFlag(EntityFlags_Disabled))
			return false;
		const auto worldPos = ent->GetWorldPosition();
		const auto worldMin =
			worldPos + k_collisionHullMins[static_cast<int>(m_hull)];
		const auto worldMax =
			worldPos + k_collisionHullMaxs[static_cast<int>(m_hull)];
		// Compute the closest point on the AABB to the sphere center
		oxyVec3 closestPoint = position;
		if (position.x < worldMin.x)
			closestPoint.x = worldMin.x;
		else if (position.x > worldMax.x)
			closestPoint.x = worldMax.x;
		if (position.y < worldMin.y)
			closestPoint.y = worldMin.y;
		else if (position.y > worldMax.y)
			closestPoint.y = worldMax.y;
		if (position.z < worldMin.z)
			closestPoint.z = worldMin.z;
		else if (position.z > worldMax.z)
			closestPoint.z = worldMax.z;
		// Check if the closest point is within the sphere radius
		return (position - closestPoint).Magnitude() <= radius;
	}
	auto HullComponent::DoesIgnoreEntity(const Entity* entity) const -> oxyBool
	{
		if (std::find_if(m_ignoreEntities.begin(), m_ignoreEntities.end(),
						 [&entity](const auto& weakEntity) {
							 return weakEntity.lock().get() == entity;
						 }) != m_ignoreEntities.end())
			return true;
		const auto otherHull = entity->GetComponent<HullComponent>();
		if (!otherHull)
			return false;
		const auto self = GetEntity();
		return std::find_if(otherHull->m_ignoreEntities.begin(),
							otherHull->m_ignoreEntities.end(),
							[&self](const auto& weakEntity) {
								return weakEntity.lock().get() == self.get();
							}) != otherHull->m_ignoreEntities.end();
	}
	auto HullComponent::AddToIgnoreList(
		const std::shared_ptr<struct Entity>& entity) -> void
	{
		m_ignoreEntities.push_back(entity);
	}
	auto HullComponent::Update(oxyF32 deltaTimeSeconds) -> void
	{

		if (m_response == CollisionResponseType_Slide)
		{
			UpdateSlide(deltaTimeSeconds);
		}
		else if (m_response == CollisionResponseType_Bounce)
		{
			UpdateBounce(deltaTimeSeconds);
		}
	}
	//auto HullComponent::Render() const -> void
	//{
	//	const auto ent = GetEntity();
	//	if (ent->GetFlag(EntityFlags_IsLocalPlayer))
	//		return;
	//	const auto world = ent->GetWorld();
	//	const auto worldPos = ent->GetWorldPosition();
	//	const auto worldMin =
	//		worldPos + k_collisionHullMins[static_cast<int>(m_hull)];
	//	const auto worldMax =
	//		worldPos + k_collisionHullMaxs[static_cast<int>(m_hull)];
	//	
	//	{
	//		oxyVec4 p = oxyVec4{worldPos, 1.0f} * GfxRenderer::GetInstance()
	//				.GetViewProjectionMatrix();
	//		p /= p.w;
	//		p.x *= -1.f;
	//		GfxRenderer::GetInstance().OverlayText("P", p.x, p.y, {1, 0, 1},
	//											   0.02f, 0.02f, true);
	//	}
	//	{
	//		oxyVec4 p = oxyVec4{worldMax, 1.0f} *
	//					GfxRenderer::GetInstance().GetViewProjectionMatrix();
	//		p /= p.w;
	//		p.x *= -1.f;
	//		GfxRenderer::GetInstance().OverlayText("X", p.x, p.y, {1, 0, 1},
	//											   0.02f, 0.02f, true);
	//	}
	//	{
	//		oxyVec4 p = oxyVec4{worldMin, 1.0f} *
	//					GfxRenderer::GetInstance().GetViewProjectionMatrix();
	//		p /= p.w;
	//		p.x *= -1.f;
	//		GfxRenderer::GetInstance().OverlayText("N", p.x, p.y, {1, 0, 1},
	//											   0.02f, 0.02f, true);
	//	}
	//}
	auto HullComponent::UpdateSlide(oxyF32 deltaTimeSeconds) -> void
	{
		const auto ent = GetEntity();
		const auto world = ent->GetWorld();
		const auto worldPosition = ent->GetWorldPosition();
		const auto stepdist = oxyVec3{0, 0, deltaTimeSeconds * 48.f};
		const auto pos = world->CalculateHullSlideMovement(
			m_hull, worldPosition + stepdist, m_velocity * deltaTimeSeconds);
		if (m_gravityPerSecond != 0.0f)
		{
			const auto gravityVec =
				oxyVec3{0, 0, -m_gravityPerSecond * deltaTimeSeconds};
			World::LineTraceResult result{};
			const auto end = pos + gravityVec;
			if (world->HullTrace(m_hull, pos, end, result))
			{
				m_velocity.z = 0.0f;
				ClipToHullsAndUpdateWorldPosition(
					worldPosition, result.m_endPos, world.get(), ent.get());
			}
			else
			{
				m_velocity.z -= m_gravityPerSecond * deltaTimeSeconds;
				ClipToHullsAndUpdateWorldPosition(worldPosition, pos,
												  world.get(), ent.get());
			}
		}
		else
		{
			ClipToHullsAndUpdateWorldPosition(worldPosition, pos, world.get(),
											  ent.get());
		}

		// apply drag
		m_velocity.x *= 1.0f - m_drag * deltaTimeSeconds;
		m_velocity.y *= 1.0f - m_drag * deltaTimeSeconds;
	}

#if 0
	auto World::RecursiveBounceHull(oxyS32 rootClipNode,
									const oxyVec3& position,
									const oxyVec3& offset, oxyVec3& velocity,
									int depth) -> oxyVec3
	{
		if (depth > 4)
			return {};
		LineTraceResult hitResult;
		if (RecursiveClipNodeLineTrace(rootClipNode, position,
									   position + offset, hitResult))
		{
			// Redirect velocity based on the plane normal
			auto invNormal =
				-oxyVec3{hitResult.m_planeNormal.x, hitResult.m_planeNormal.y,
						 hitResult.m_planeNormal.z};
			auto dot = velocity.DotProduct(invNormal);
			velocity -= invNormal * dot * 2.f;
			// Redirect remaining offset as well
			const auto hitDist = (hitResult.m_endPos - position).Magnitude();
			const auto offsetTaken = offset.Normalized() * hitDist;
			const auto newOffset = offset - offsetTaken;
			const auto newPos = hitResult.m_endPos;
			return RecursiveBounceHull(rootClipNode, newPos, newOffset,
									   velocity, depth + 1);
		}
		return offset;
	}
#endif
	auto HullComponent::UpdateBounce(oxyF32 deltaTimeSeconds) -> void
	{
		const auto ent = GetEntity();
		const auto world = ent->GetWorld();

		const auto worldPos = ent->GetWorldPosition();

		World::LineTraceResult result{};
		if (world->HullTrace(m_hull, worldPos,
							 worldPos + m_velocity * deltaTimeSeconds, result))
		{
			// Redirect velocity based on the plane normal
			auto invNormal = -result.m_planeNormal;
			auto dot = m_velocity.DotProduct(invNormal);
			m_velocity = (m_velocity - invNormal * dot * 2.f) *
						 m_bounceVelocityMultiplier;
			// Redirect remaining offset as well
			const auto hitDist = (result.m_endPos - worldPos).Magnitude();
			const auto offsetTaken = m_velocity.Normalized() * hitDist;
			const auto newOffset = m_velocity - offsetTaken;
			const auto newPos = result.m_endPos;
			m_onBounceEvent.IterateCallbacks([]() {}, this, nullptr, newPos);
			ClipToHullsAndUpdateWorldPosition(worldPos, newPos, world.get(),
											  ent.get());
		}
		else
		{
			ClipToHullsAndUpdateWorldPosition(
				worldPos, worldPos + m_velocity * deltaTimeSeconds, world.get(),
				ent.get());
		}
		m_velocity.z -= m_gravityPerSecond * deltaTimeSeconds;

		// apply drag
		m_velocity.x *= 1.0f - m_drag * deltaTimeSeconds;
		m_velocity.y *= 1.0f - m_drag * deltaTimeSeconds;

	}
	auto HullComponent::ClipToHullsAndUpdateWorldPosition(
		const oxyVec3& position, const oxyVec3& newPosition, const World* world,
		Entity* self) -> void
	{
		// HACK: TODO:
		// this was commented because the host does not have velocity of a client,
		// thus it will never have any collision events ugh this should
		// be continuous collision detection
		if ((newPosition - position).MagnitudeSquared() < 0.0001f)
			return; // we didnt really move


		const auto& entityList = world->GetEntityList();
		auto finalPos = newPosition;
		for (oxySize i = 0; i < entityList.size(); ++i)
		{
			const auto& ent = entityList[i];
			if (ent.get() == self)
				continue;
			if (ent->GetFlag(EntityFlags_Disabled))
				continue;
			if (!ent->GetFlag(EntityFlags_HasHull))
				continue;
			const auto hullcomp = ent->GetComponent<HullComponent>();
			if (!hullcomp)
				continue;
			if (!hullcomp->IsEnabled())
				continue;
			if (hullcomp->DoesIgnoreEntity(self))
				continue;
			oxyVec3 hullCollidePos;
			oxyVec3 hullCollideNormal;
			if (hullcomp->CollidesWithHull(finalPos, m_hull, hullCollidePos,
										   hullCollideNormal))
			{
				m_onCollideEvent.IterateCallbacks([]() {}, this, ent.get(),
												  hullCollidePos,
												  hullCollideNormal);
				if (m_response == CollisionResponseType_Bounce)
					m_onBounceEvent.IterateCallbacks([]() {}, this, ent.get(),
													 hullCollidePos);
				if (hullcomp->m_solid && m_solid)
				{
					// push away from the hull
					// the collide normal is scaled by the penetration depth
					finalPos -= hullCollideNormal;

					if (m_response == CollisionResponseType_Bounce)
					{
						// Redirect velocity based on the plane normal
						const auto normalized = hullCollideNormal.Normalized();
						const auto invNormal = -normalized;
						auto dot = m_velocity.DotProduct(invNormal);
						m_velocity = (m_velocity - invNormal * dot * 2.f) *
									 m_bounceVelocityMultiplier;
					}
				}
			}
		}
		self->SetWorldPosition(finalPos);
	}
}; // namespace oxygen