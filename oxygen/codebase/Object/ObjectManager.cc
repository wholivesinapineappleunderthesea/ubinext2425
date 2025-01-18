#include "OxygenPCH.h"
#include "ObjectManager.h"

namespace oxygen
{
	ObjectManager::~ObjectManager()
	{
		OXYCHECK(m_objects.empty());
	}

	auto ObjectManager::NewObject(const ObjectDescription& desc,
								  oxyObjectID id) -> Object*
	{
		if (!id)
			id = m_nextDynamicID++;
		else
		{
			const auto it = m_objects.find(id);
			if (it != m_objects.end())
				return it->second;
		}
		const auto storage =
			::operator new[](desc.m_size, std::align_val_t{desc.m_align});
		const auto obj = desc.m_constructor(storage);
		OXYCHECK(storage == obj);
		m_objects.emplace(id, obj);
		m_objectIDs.emplace(obj, id);
		return obj;
	};

	auto ObjectManager::DeleteObject(Object* obj, oxyObjectID id) -> void
	{
		if (!obj && !id)
			return;
		auto idit = m_objectIDs.end();
		auto objit = m_objects.end();
		if (id)
		{
			objit = m_objects.find(id);
			OXYCHECK(objit != m_objects.end());
			obj = objit->second;
		}
		if (obj)
		{
			idit = m_objectIDs.find(obj);
			OXYCHECK(idit != m_objectIDs.end());
			id = idit->second;
		}
		OXYCHECK(obj);
		OXYCHECK(id);
		if (idit == m_objectIDs.end())
		{
			idit = m_objectIDs.find(obj);
		}
		if (objit == m_objects.end())
		{
			objit = m_objects.find(id);
		}
		OXYCHECK(idit != m_objectIDs.end());
		OXYCHECK(objit != m_objects.end());
		OXYCHECK(idit->second == objit->first);
		m_objectIDs.erase(idit);
		m_objects.erase(objit);

		const auto& desc = obj->GetDescription();

		// destroy object
		obj->~Object();
		::operator delete[](obj, std::align_val_t{desc.m_align});
	}

	auto ObjectManager::CreateManagedObject(const ObjectDescription& desc,
											oxyObjectID id)
		-> std::shared_ptr<ManagedObject>
	{
		if (!id)
			id = m_nextManagedID++;
		else
		{
			const auto it = m_managedObjects.find(id);
			if (it != m_managedObjects.end())
			{
				const auto sptr = it->second.lock();
				return sptr;
			}
		}
		const auto ptr = static_cast<ManagedObject*>(NewObject(desc, id));
		const auto sptr =
			std::shared_ptr<ManagedObject>(ptr, ManagedObjectDeleter{});
		ptr->m_id = id;
		ptr->m_self = sptr;
		m_managedObjects.emplace(id, sptr);
		return sptr;
	}

	auto ObjectManager::GetObjectPtr(oxyObjectID id) const -> Object*
	{
		const auto it = m_objects.find(id);
		if (it == m_objects.end())
			return nullptr;
		return it->second;
	}

	auto ObjectManager::GetObjectID(Object* obj) const -> oxyObjectID
	{
		const auto it = m_objectIDs.find(obj);
		if (it == m_objectIDs.end())
			return 0;
		return it->second;
	}

	auto ObjectManager::GetManagedRef(oxyObjectID id) const
		-> std::shared_ptr<ManagedObject>
	{
		const auto it = m_managedObjects.find(id);
		if (it == m_managedObjects.end())
			return nullptr;
		return it->second.lock();
	}

	auto
	ObjectManager::ManagedObjectDeleter::operator()(ManagedObject* obj) -> void
	{
		if (obj)
		{
			auto& om = ObjectManager::GetInstance();
			const auto id = obj->GetObjectID();
			om.DeleteObject(obj);
			om.m_managedObjects.erase(id);
		}
	}
}; // namespace oxygen