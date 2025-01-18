#include "OxygenPCH.h"
#include "Entity.h"
#include "Component/Component.h"

#include "World/World.h"

namespace oxygen
{
	auto Entity::GetWorldPosition() const -> oxyVec3
	{
		if (m_hierarchy.m_parent)
		{
			return m_hierarchy.m_parent->GetWorldPosition() + m_localPosition;
		}
		return m_localPosition;
	}
	auto Entity::GetWorldRotation() const -> oxyQuat
	{
		if (m_hierarchy.m_parent)
		{
			return m_hierarchy.m_parent->GetWorldRotation() * m_localRotation;
		}
		return m_localRotation;
	}
	auto Entity::GetWorldScale() const -> oxyVec3
	{
		if (m_hierarchy.m_parent)
		{
			return m_hierarchy.m_parent->GetWorldScale() * m_localScale;
		}
		return m_localScale;
	}
	auto Entity::GetWorldTransformMatrix() const -> oxyMat4x4
	{
		oxyMat4x4 mtx = oxyMat4x4::Identity();
		mtx = Math::Translate(mtx, GetWorldPosition());
		mtx = Math::Rotate(mtx, GetWorldRotation());
		mtx = Math::Scale(mtx, GetWorldScale());
		return mtx;
	}
	auto Entity::SetWorldPosition(const oxyVec3& position) -> void
	{
		if (m_hierarchy.m_parent)
		{
			m_localPosition =
				position - m_hierarchy.m_parent->GetWorldPosition();
		}
		else
		{
			m_localPosition = position;
		}
	}
	auto Entity::SetWorldRotation(const oxyQuat& rotation) -> void
	{
		if (m_hierarchy.m_parent)
		{
			m_localRotation =
				rotation * m_hierarchy.m_parent->GetWorldRotation();
		}
		else
		{
			m_localRotation = rotation;
		}
	}
	auto Entity::SetWorldScale(const oxyVec3& scale) -> void
	{
		if (m_hierarchy.m_parent)
		{
			m_localScale = scale / m_hierarchy.m_parent->GetWorldScale();
		}
		else
		{
			m_localScale = scale;
		}
	}
	auto Entity::Destroy() -> void
	{
		SetFlag(EntityFlags_Disabled, true);
		const auto world = m_world.lock();
		if (!world)
			return;
		world->RemoveEntity(this);
		// this == nullptr here!!!
	}
	auto Entity::SetParent(std::shared_ptr<Entity> parent) -> void
	{
		auto wpos = GetWorldPosition();
		auto wrot = GetWorldRotation();
		auto wscale = GetWorldScale();
		m_hierarchy.m_parent = parent;
		SetWorldPosition(wpos);
		SetWorldRotation(wrot);
		SetWorldScale(wscale);
		// SetLocalPosition({});
		// SetLocalRotation({0, 0, 0, 1});
		// SetLocalScale({1, 1, 1});
	}
	auto Entity::Update(oxyF32 deltaTimeSeconds) -> void
	{
		// dont destroy this when we are still updating \/
		auto self = GetHardRef<Entity>();
		for (auto& component : m_components)
		{
			if (component->IsEnabled())
				component->Update(deltaTimeSeconds);
		}
	}
	auto Entity::Render() const -> void
	{
		for (const auto& component : m_components)
		{
			if (component->IsEnabled())
				component->Render();
		}
	}
}; // namespace oxygen