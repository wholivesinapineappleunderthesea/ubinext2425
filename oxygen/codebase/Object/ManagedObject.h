#pragma once

namespace oxygen
{
	struct ManagedObject : Object
	{
		OXYGENOBJECT(ManagedObject, Object);

		auto GetObjectID() const -> oxyObjectID
		{
			return m_id;
		}

		template <typename RefType>
		auto GetHardRef() const -> std::shared_ptr<RefType>
			requires std::is_base_of_v<ManagedObject, RefType>
		{
			const auto sptr = m_self.lock();
			if (sptr)
				return std::static_pointer_cast<RefType>(std::move(sptr));
			return nullptr;
		}

	  private:
		oxyObjectID m_id{};
		std::weak_ptr<ManagedObject> m_self{};

		friend struct ObjectManager;
	};
}; // namespace oxygen