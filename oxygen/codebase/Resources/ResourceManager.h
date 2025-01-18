#pragma once

#include "Singleton/Singleton.h"

namespace oxygen
{
	struct StaticMeshResource;
	struct AnimatedMeshResource;

	struct ResourceManager : SingletonBase<ResourceManager>
	{
		auto LoadStaticMesh(std::string_view name)
			-> std::shared_ptr<const StaticMeshResource>;
		auto LoadAnimatedMesh(std::string_view name)
			-> std::shared_ptr<const AnimatedMeshResource>;

	  private:
		std::unordered_map<std::size_t, std::weak_ptr<const StaticMeshResource>>
			m_staticMeshes;
		std::unordered_map<std::size_t,
						   std::weak_ptr<const AnimatedMeshResource>>
			m_AnimatedMeshes;
	};
}; // namespace oxygen