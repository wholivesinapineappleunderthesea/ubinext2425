#pragma once

namespace oxygen
{
	template <typename T, int N> struct SPSCQueue
	{
		static_assert(N > 0, "Queue size must be greater than 0");

		SPSCQueue() : m_writePosition(0), m_readPosition(0)
		{
		}

		template <typename T> auto TryPush(T&& value) -> bool
		{
			const auto writeIndex =
				m_writePosition.load(std::memory_order_relaxed);
			const auto nextWriteIndex = (writeIndex + 1) % N;

			if (nextWriteIndex ==
				m_readPosition.load(std::memory_order_acquire))
				return false;

			m_items[writeIndex] = std::forward<T>(value);
			m_writePosition.store(nextWriteIndex, std::memory_order_release);
			return true;
		}

		auto TryPop(T& valueOut) -> bool
		{
			const auto readIndex =
				m_readPosition.load(std::memory_order_relaxed);

			if (readIndex == m_writePosition.load(std::memory_order_acquire))
				return false;

			// IMPORTANT:
			// This pop CANNOT modify any data in the items array
			// god forbid there is a mutable member...
			valueOut = T{static_cast<const T&>(m_items[readIndex])};
			m_readPosition.store((readIndex + 1) % N,
								 std::memory_order_release);
			return true;
		}

	  private:
		alignas(128) std::array<T, N> m_items;
		std::atomic<int> m_writePosition;
		std::atomic<int> m_readPosition;
		oxyU8 m_padding[128];
	};
}; // namespace oxygen