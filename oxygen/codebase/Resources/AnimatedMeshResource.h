#pragma once

namespace oxygen
{
	struct AnimationInfo
	{
		std::vector<std::vector<oxyVec3>> m_frames;
	};
	struct AnimatedMeshResource
	{
		std::shared_ptr<const struct StaticMeshResource> m_rootPose;
		std::unordered_map<oxyU32, AnimationInfo> m_animations;
	};
};