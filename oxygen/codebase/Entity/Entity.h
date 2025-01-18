#pragma once

namespace oxygen
{
	enum EntityFlags : oxyU32
	{
		EntityFlags_Disabled = 1 << 0,
		EntityFlags_Static = 1 << 1,
		EntityFlags_Dynamic = 1 << 2,
		EntityFlags_Renderable = 1 << 3,
		EntityFlags_HasHull = 1 << 4,
		EntityFlags_HasCamera = 1 << 5,
		EntityFlags_Replicated = 1 << 6,
		EntityFlags_IsLocalPlayer = 1 << 7,
		EntityFlags_EnableTransformReplication = 1 << 8,
		EntityFlags_EnableTransformInterpolation = 1 << 9,
	};

	struct EntityHierarchy
	{
		std::shared_ptr<struct Entity> m_parent{};
		std::vector<std::shared_ptr<struct Entity>> m_children;
		std::weak_ptr<struct Entity> m_self{};
	};

	struct Component;
	struct Entity final : ManagedObject
	{
		OXYGENOBJECT(Entity, ManagedObject);

		auto GetLocalPosition() const -> const oxyVec3&
		{
			return m_localPosition;
		}
		auto GetLocalRotation() const -> const oxyQuat&
		{
			return m_localRotation;
		}
		auto GetLocalScale() const -> const oxyVec3&
		{
			return m_localScale;
		}
		auto GetWorldPosition() const -> oxyVec3;
		auto GetWorldRotation() const -> oxyQuat;
		auto GetWorldScale() const -> oxyVec3;
		auto GetWorldTransformMatrix() const -> oxyMat4x4;
		auto GetFlag(EntityFlags flag) const -> oxyBool
		{
			return static_cast<std::underlying_type_t<EntityFlags>>(m_flags) &
				   static_cast<std::underlying_type_t<EntityFlags>>(flag);
		}
		auto GetWorld() const -> std::shared_ptr<struct World>
		{
			return m_world.lock();
		}
		auto GetParent() const -> std::shared_ptr<Entity>
		{
			return m_hierarchy.m_parent;
		}
		auto GetRenderOcclusionMin() const -> const oxyVec3&
		{
			return m_renderOcclusionMin;
		}
		auto GetRenderOcclusionMax() const -> const oxyVec3&
		{
			return m_renderOcclusionMax;
		}
		auto SetLocalPosition(const oxyVec3& position) -> void
		{
			m_localPosition = position;
		}
		auto SetLocalRotation(const oxyQuat& rotation) -> void
		{
			m_localRotation = rotation;
		}
		auto SetLocalScale(const oxyVec3& scale) -> void
		{
			m_localScale = scale;
		}
		auto SetWorldPosition(const oxyVec3& position) -> void;
		auto SetWorldRotation(const oxyQuat& rotation) -> void;
		auto SetWorldScale(const oxyVec3& scale) -> void;
		auto SetFlag(EntityFlags flag, oxyBool state) -> void
		{
			if (state)
				m_flags = static_cast<EntityFlags>(
					static_cast<std::underlying_type_t<EntityFlags>>(m_flags) |
					static_cast<std::underlying_type_t<EntityFlags>>(flag));
			else
				m_flags = static_cast<EntityFlags>(
					static_cast<std::underlying_type_t<EntityFlags>>(m_flags) &
					~static_cast<std::underlying_type_t<EntityFlags>>(flag));
		}
		auto SetRenderOcclusionMin(const oxyVec3& min) -> void
		{
			m_renderOcclusionMin = min;
		}
		auto SetRenderOcclusionMax(const oxyVec3& max) -> void
		{
			m_renderOcclusionMax = max;
		}

		template <typename T>
		auto AddComponent(oxyObjectID id = 0) -> std::shared_ptr<T>
		{
			auto component =
				ObjectManager::GetInstance().CreateManagedObject<T>(id);
			component->m_entity = GetHardRef<Entity>();
			m_components.push_back(component);

			return component;
		}
		template <typename T> auto GetComponent() const -> std::shared_ptr<T>
		{
			for (auto& component : m_components)
			{
				if (component->IsA<T>())
				{
					return std::static_pointer_cast<T>(component);
				}
			}
			return nullptr;
		}

		auto Destroy() -> void;

		auto SetParent(std::shared_ptr<Entity> parent) -> void;

		auto Update(oxyF32 deltaTimeSeconds) -> void;
		auto Render() const -> void;

	  private:
		oxyVec3 m_localPosition{};
		oxyQuat m_localRotation{0.f, 0.f, 0.f, 1.f};
		oxyVec3 m_localScale{1.f, 1.f, 1.f};
		EntityHierarchy m_hierarchy{};
		EntityFlags m_flags{};
		std::weak_ptr<struct World> m_world{};
		std::vector<std::shared_ptr<Component>> m_components{};
		oxyVec3 m_renderOcclusionMin{};
		oxyVec3 m_renderOcclusionMax{};

		friend struct World;
	};
}; // namespace oxygen