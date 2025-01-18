#pragma once



namespace oxygen
{
	inline std::random_device g_randomDevice;
	inline std::mt19937_64 g_randomEngine{g_randomDevice()};
	inline auto RandomS32(oxyS32 minInclusive, oxyS32 maxInclusive) -> oxyS32
	{
		std::uniform_int_distribution<oxyS32> dist{minInclusive, maxInclusive};
		return dist(g_randomEngine);
	}
	inline auto RandomU32(oxyU32 minInclusive, oxyU32 maxInclusive) -> oxyU32
	{
		std::uniform_int_distribution<oxyU32> dist{minInclusive, maxInclusive};
		return dist(g_randomEngine);
	}
	inline auto RandomS64(oxyS64 minInclusive, oxyS64 maxInclusive) -> oxyS64
	{
		std::uniform_int_distribution<oxyS64> dist{minInclusive, maxInclusive};
		return dist(g_randomEngine);
	}
	inline auto RandomU64(oxyU64 minInclusive, oxyU64 maxInclusive) -> oxyU64
	{
		std::uniform_int_distribution<oxyU64> dist{minInclusive, maxInclusive};
		return dist(g_randomEngine);
	}
	inline auto RandomF32(oxyF32 minInclusive, oxyF32 maxInclusive) -> oxyF32
	{
		std::uniform_real_distribution<oxyF32> dist{minInclusive, maxInclusive};
		return dist(g_randomEngine);
	}
	inline auto RandomF64(oxyF64 minInclusive, oxyF64 maxInclusive) -> oxyF64
	{
		std::uniform_real_distribution<oxyF64> dist{minInclusive, maxInclusive};
		return dist(g_randomEngine);
	}
	inline auto RandomBool() -> oxyBool
	{
		std::bernoulli_distribution dist;
		return dist(g_randomEngine);
	}

}; // namespace oxygen