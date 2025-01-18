#pragma once

namespace oxygen
{
	/**
	 * @brief Singleton instance, should only be in an object templated to
	 * SingletonHolder. Contains the actual storage buffer for the type T.
	 * @tparam T The type of the singleton.
	 */
	template <typename T> struct SingletonInstance : NonCopyable
	{
		template <typename... Args> SingletonInstance(Args&&... args)
		{
			if (!s_inLifeTime)
			{
				new (s_singletonBuffer) T(std::forward<Args>(args)...);
				s_inLifeTime = true;
			}
		}
		~SingletonInstance()
		{
			if (s_inLifeTime)
			{
				reinterpret_cast<T*>(s_singletonBuffer)->~T();
				s_inLifeTime = false;
			}
		}
		static auto GetInstance() -> T&
		{
			return *reinterpret_cast<T*>(s_singletonBuffer);
		}

	  private:
		static inline bool s_inLifeTime{};
		static inline alignas(T) unsigned char s_singletonBuffer[sizeof(T)]{};
	};

	/**
	 * @brief Singleton base class. All singletons should inherit from this
	 * class. Uses the curiously recurring template pattern and provides the
	 * GetInstance() static method to access the singleton.
	 * @tparam CRTPType The type of the singleton.
	 */
	template <typename CRTPType> struct SingletonBase : NonCopyable
	{
		SingletonBase()
		{
			OXYCHECK(!s_instance);
			s_instance = static_cast<CRTPType*>(this);
		}
		~SingletonBase()
		{
			s_instance = nullptr;
		}
		static auto GetInstance() -> CRTPType&
		{
			return SingletonInstance<CRTPType>::GetInstance();
		}

	  private:
		// Purely for debugging:
		static inline CRTPType* s_instance{};
	};

	/**
	 * @brief Storage of a group of singletons. Used to explicitly construct and
	 * destruct singletons.
	 * @tparam SingletonsStruct A struct containing SingletonInstance objects.
	 */
	template <typename SingletonsStruct> struct SingletonHolder : NonCopyable
	{
		static auto Construct() -> void
		{
			if (!m_storage.has_value())
			{
				m_storage.emplace();
			}
		}
		static auto Destruct() -> void
		{
			m_storage.reset();
		}

	  private:
		// The global optional will have its own atexit destructor guaranteed.
		// Thus, all singletons are also guaranteed to be destructed.
		static inline std::optional<SingletonsStruct> m_storage{};
	};
} // namespace oxygen