#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct AnimatedMeshResource;
	struct AnimatedMeshComponent final : Component
	{
		OXYGENOBJECT(AnimatedMeshComponent, Component);

		auto LoadByName(std::string_view name) -> oxyBool;

		auto BeginAnimation(oxyU32 animHash, oxyBool loop = true) -> void;

		auto GetCurrenntAnimationHash() const -> oxyU32
		{
			return m_animHash;
		}

		auto SetLocalOffset(const oxyVec3& offset) -> void
		{
			m_localOffset = offset;
		}

		auto SetLocalRotation(const oxyQuat& rotation) -> void
		{
			m_localRotation = rotation;
		}

	  protected:
		auto Update(float deltaTimeSeconds) -> void override;
		auto Render() const -> void override;

	  private:
		oxyU32 m_animHash{};
		oxyF32 m_animTotalTime{};
		oxyF32 m_animCurrentFrameTime{};
		oxyF32 m_animLerpAlpha{};
		oxyBool m_loopAnim{};
		oxyVec3 m_localOffset{};
		oxyQuat m_localRotation{};
		const std::vector<oxyVec3>* m_currentFrame{};
		const std::vector<oxyVec3>* m_nextFrame{};
		std::shared_ptr<const AnimatedMeshResource> m_resource;
		std::shared_ptr<const struct GfxTexture> m_texture;
	};
} // namespace oxygen