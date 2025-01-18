#pragma once

#include "Singleton/Singleton.h"

namespace oxygen
{
	struct ObjectManager : SingletonBase<ObjectManager>
	{
		~ObjectManager();

		template <typename T>
		auto NewObject(oxyObjectID id = 0) -> T*
			requires std::is_base_of_v<Object, T> &&
					 std::is_same_v<typename T::SelfType, T>;
		auto NewObject(const ObjectDescription& desc,
					   oxyObjectID id = 0) -> Object*;
		auto DeleteObject(Object* obj, oxyObjectID id = 0) -> void;

		template <typename T>
		auto CreateManagedObject(oxyObjectID id = 0) -> std::shared_ptr<T>
			requires std::is_base_of_v<ManagedObject, T> &&
					 std::is_same_v<typename T::SelfType, T>;
		auto CreateManagedObject(const ObjectDescription& desc,
								 oxyObjectID id = 0)
			-> std::shared_ptr<ManagedObject>;

		template <typename T>
		auto GetObjectPtr(oxyObjectID id) const -> T*
			requires std::is_base_of_v<Object, T>;
		auto GetObjectPtr(oxyObjectID id) const -> Object*;

		auto GetObjectID(Object* obj) const -> oxyObjectID;

		template <typename T>
		auto GetManagedRef(oxyObjectID id) const -> std::shared_ptr<T>
			requires std::is_base_of_v<ManagedObject, T>;
		auto
		GetManagedRef(oxyObjectID id) const -> std::shared_ptr<ManagedObject>;

	  private:
		oxyObjectID m_nextDynamicID{0x40000001};
		std::unordered_map<oxyObjectID, Object*> m_objects{};
		std::unordered_map<Object*, oxyObjectID> m_objectIDs{};

		oxyObjectID m_nextManagedID{0x80000001};
		std::unordered_map<oxyObjectID, std::weak_ptr<ManagedObject>>
			m_managedObjects{};
		struct ManagedObjectDeleter
		{
			auto operator()(ManagedObject* obj) -> void;
		};
	};

	template <typename T>
	auto ObjectManager::NewObject(oxyObjectID id) -> T*
		requires std::is_base_of_v<Object, T> &&
				 std::is_same_v<typename T::SelfType, T>
	{
		if (!id)
			id = ++m_nextDynamicID;
		else
		{
			const auto it = m_objects.find(id);
			if (it != m_objects.end())
				return static_cast<T*>(it->second);
		}
		const auto storage =
			::operator new[](sizeof(T), std::align_val_t{alignof(T)});
		const auto obj = new (storage) T{};
		OXYCHECK(storage == obj);
		m_objects.emplace(id, obj);
		m_objectIDs.emplace(obj, id);
		return obj;
	}

	template <typename T>
	auto
	ObjectManager::CreateManagedObject(oxyObjectID id) -> std::shared_ptr<T>
		requires std::is_base_of_v<ManagedObject, T> &&
				 std::is_same_v<typename T::SelfType, T>
	{
		if (!id)
			id = ++m_nextManagedID;
		else
		{
			const auto it = m_managedObjects.find(id);
			if (it != m_managedObjects.end())
			{
				auto sptr = it->second.lock();
				if (sptr)
					return std::static_pointer_cast<T>(std::move(sptr));
				return nullptr;
			}
		}
		const auto ptr = NewObject<T>(id);
		auto sptr = std::shared_ptr<T>(ptr, ManagedObjectDeleter{});
		ptr->m_id = id;
		ptr->m_self = sptr;
		m_managedObjects.emplace(id, sptr);
		return sptr;
	}

	template <typename T>
	auto ObjectManager::GetObjectPtr(oxyObjectID id) const -> T*
		requires std::is_base_of_v<Object, T>
	{
		return static_cast<T*>(GetObjectPtr(id));
	}

	template <typename T>
	auto
	ObjectManager::GetManagedRef(oxyObjectID id) const -> std::shared_ptr<T>
		requires std::is_base_of_v<ManagedObject, T>
	{
		auto ref = std::static_pointer_cast<T>(GetManagedRef(id));
		if (ref)
		{
			if (!ref->IsA<T>())
				ref = {};
		}
		return ref;
	}
}; // namespace oxygen