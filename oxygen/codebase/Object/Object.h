#pragma once

namespace oxygen
{
	struct Object;
	struct ObjectDescription
	{
		const ObjectDescription* m_parent{};
		std::string_view m_name{};
		oxyU64 m_id{};
		oxySize m_size{};
		oxySize m_align{};
		using constructor_t = Object* (*)(void* p);
		constructor_t m_constructor{};
	};

	inline auto GetObjectDescriptionMap()
		-> std::unordered_map<oxyU64, const ObjectDescription*>&
	{
		static std::unordered_map<oxyU64, const ObjectDescription*> map{};
		return map;
	}

#define OXYGENOBJECT(Name, Parent)                                             \
	Name() = default;                                                          \
	~Name() = default;                                                         \
	using SelfType = Name;                                                     \
	using Super = Parent;                                                      \
	static inline auto GetStaticDescription() -> const ObjectDescription&      \
	{                                                                          \
		return ObjectInternalDef::g_staticDescriptor;                          \
	}                                                                          \
	virtual auto GetDescription() const -> const ObjectDescription& override   \
	{                                                                          \
		return GetStaticDescription();                                         \
	}                                                                          \
                                                                               \
  private:                                                                     \
	struct ObjectInternalDef                                                   \
	{                                                                          \
		static auto GetStaticDescription() -> const ObjectDescription&         \
		{                                                                      \
			static_assert(std::is_base_of_v<Object, Parent>,                   \
						  "Parent must be a subclass of Object");              \
			static_assert(std::is_base_of_v<Parent, Name>,                     \
						  "Name must be a subclass of Parent");                \
			static ObjectDescription desc{                                     \
				.m_parent = &Parent::GetStaticDescription(),                   \
				.m_name = #Name,                                               \
				.m_id = CRC64Eval(reinterpret_cast<const oxyU8*>(#Name),       \
								  sizeof(#Name) - 1),                          \
				.m_size = sizeof(Name),                                        \
				.m_align = alignof(Name),                                      \
				.m_constructor =                                               \
					[](void* p) {                                              \
						return static_cast<Object*>(::new (p) Name{});         \
					},                                                         \
			};                                                                 \
			const auto mm = reinterpret_cast<Name*>(GetStaticDescription);     \
			OXYCHECK((void*)static_cast<Object*>(mm) == (void*)mm);            \
			return desc;                                                       \
		}                                                                      \
		static inline const ObjectDescription& g_staticDescriptor{             \
			GetStaticDescription()};                                           \
		static inline oxyChar g_staticDescriptorInit = []() {                  \
			GetObjectDescriptionMap().emplace(GetStaticDescription().m_id,     \
											  &g_staticDescriptor);            \
			return 0;                                                          \
		}();                                                                   \
	};                                                                         \
                                                                               \
  public:

	struct Object
	{
		using SelfType = Object;
		using Super = Object;

		Object() = default;
		virtual ~Object() = default;

		static inline auto GetStaticDescription() -> const ObjectDescription&
		{
			return ObjectInternalDef::g_staticDescriptor;
		}
		virtual auto GetDescription() const -> const ObjectDescription&
		{
			return GetStaticDescription();
		}

		auto IsA(const ObjectDescription& desc) const -> bool
		{
			auto* p = &GetDescription();
			while (p)
			{
				if (p->m_id == desc.m_id)
					return true;
				p = p->m_parent;
			}
			return false;
		}
		template <typename T>
		auto IsA() const -> bool
			requires std::is_base_of_v<Object, T>
		{
			return IsA(T::GetStaticDescription());
		}
		template <typename T>
		auto Cast() -> T*
			requires std::is_base_of_v<Object, T>
		{
			return IsA<T>() ? static_cast<T*>(this) : nullptr;
		}
		template <typename T>
		auto Cast() const -> const T*
			requires std::is_base_of_v<Object, T>
		{
			return IsA<T>() ? static_cast<const T*>(this) : nullptr;
		}

	  private:
		struct ObjectInternalDef
		{
			static auto GetStaticDescription() -> const ObjectDescription&
			{
				static ObjectDescription desc{
					.m_parent = nullptr,
					.m_name = "Object",
					.m_id = CRC64Eval(reinterpret_cast<const oxyU8*>("Object"),
									  sizeof("Object") - 1),
					.m_size = sizeof(Object),
					.m_align = alignof(Object),
					.m_constructor = [](void* p) { return ::new (p) Object{}; },
				};
				return desc;
			}
			static inline const ObjectDescription& g_staticDescriptor{
				GetStaticDescription()};
			static inline oxyChar g_staticDescriptorInit = []() {
				GetObjectDescriptionMap().emplace(GetStaticDescription().m_id,
												  &g_staticDescriptor);
				return 1;
			}();
		};
	};

}; // namespace oxygen