#include <OxygenPCH.h>
#include "AnimatedMeshComponent.h"
#include "Entity/Entity.h"

#include "Resources/ResourceManager.h"
#include "Resources/AnimatedMeshResource.h"
#include "Resources/StaticMeshResource.h"

#include "Gfx/GfxRenderer.h"
#include "Platform/Platform.h"

namespace oxygen
{
	auto AnimatedMeshComponent::LoadByName(std::string_view name) -> oxyBool
	{
		m_resource = ResourceManager::GetInstance().LoadAnimatedMesh(name);
		if (m_resource)
		{
			OXYCHECK(m_resource->m_rootPose);
			m_texture = GfxRenderer::GetInstance().LoadTexture(
				std::format("{}/textures/{}.png", GetExecutableDirectory(),
							m_resource->m_rootPose->m_texname));
		}
		return m_resource != nullptr;
	}

	auto AnimatedMeshComponent::BeginAnimation(oxyU32 animHash, oxyBool loop) -> void
	{
		m_animHash = animHash;
		m_animTotalTime = 0.0f;
		m_loopAnim = loop;
		m_currentFrame = nullptr;
		m_nextFrame = nullptr;
		auto it = m_resource->m_animations.find(animHash);
		if (it != m_resource->m_animations.end())
		{
			m_currentFrame = &it->second.m_frames[0];
			if (it->second.m_frames.size() > 1)
				m_nextFrame = &it->second.m_frames[1];
		}
	}

	auto AnimatedMeshComponent::Update(float deltaTimeSeconds) -> void
	{
		if (!m_resource)
			return;
		if (m_currentFrame)
		{
			const auto fps = 30.0f; // eww!
			const auto lastUpdateAnimTime = m_animTotalTime;
			
			m_animTotalTime += deltaTimeSeconds;
			

			auto it = m_resource->m_animations.find(m_animHash);
			if (it != m_resource->m_animations.end())
			{
				const auto frameIndex =
					std::clamp(static_cast<oxyU32>(m_animTotalTime * fps), 0u,
							   static_cast<oxyU32>(it->second.m_frames.size()));
				const auto lastUpdateIndex = std::clamp(
					static_cast<oxyU32>(lastUpdateAnimTime * fps), 0u,
					static_cast<oxyU32>(it->second.m_frames.size()));
				if (frameIndex >= it->second.m_frames.size())
				{
					if (m_loopAnim)
					{
						// TODO: FIX:
						// what if we went over? setting it to exactly 0 is not
						// correct it should be modulo?
						m_animTotalTime = 0.0f;
						m_animCurrentFrameTime = 0.0f;
						m_currentFrame = nullptr;
						m_nextFrame = nullptr;
						if (it != m_resource->m_animations.end())
						{
							m_currentFrame = &it->second.m_frames[0];
							if (it->second.m_frames.size() > 1)
								m_nextFrame = &it->second.m_frames[1];
						}
					}
				}
				else if (frameIndex != lastUpdateIndex)
				{
					// TODO: FIX:
					// again, modulo
					m_animCurrentFrameTime = 0.0f;
					m_currentFrame = m_nextFrame;
					m_nextFrame = nullptr;
					if (m_currentFrame)
					{
						auto it = m_resource->m_animations.find(m_animHash);
						if (it != m_resource->m_animations.end())
						{
							const auto nextFrameIndex = frameIndex + 1;
							if (nextFrameIndex < it->second.m_frames.size())
								m_nextFrame =
									&it->second.m_frames[nextFrameIndex];
						}
					}
				}
				else
				{
					// increase m_animCurrentFrameTime
					m_animCurrentFrameTime += deltaTimeSeconds;
					// lerp alpha
					m_animLerpAlpha = m_animCurrentFrameTime / (1.0f / fps);
				}
			}
		}
	}

	auto AnimatedMeshComponent::Render() const -> void
	{
		if (!m_resource)
			return;

		const auto ent = GetEntity();
		const auto pos = ent->GetWorldPosition();
		const auto rot = ent->GetWorldRotation();
		const auto scale = ent->GetWorldScale();
		oxyMat4x4 localMtx = oxyMat4x4::Identity();
		localMtx = Math::Translate(localMtx, m_localOffset);
		localMtx = Math::Rotate(localMtx, m_localRotation);
		oxyMat4x4 worldMtx = oxyMat4x4::Identity();
		worldMtx = Math::Translate(worldMtx, pos);
		worldMtx = Math::Rotate(worldMtx, rot);
		worldMtx = Math::Scale(worldMtx, scale);
		const auto vp = GfxRenderer::GetInstance().GetViewProjectionMatrix();

		const auto animit = m_resource->m_animations.find(m_animHash);

		size_t vertIndex{};
		for (const auto& tri : m_resource->m_rootPose->m_tris)
		{
			GfxTri gfxtri;
			for (int i = 0; i < 3; ++i)
			{
				oxyVec4 pos{tri.m_vertices[i].m_position, 1.f};

				//if (animit != m_resource->m_animations.end())
				//{
				//	const auto& anim = animit->second;
				//	const auto& frame = anim.m_frames[m_animFrameIndex];
				//	const auto& animvert = frame[vertIndex];
				//	pos = {animvert.x, animvert.y,
				//		   animvert.z,
				//		   1.f};
				//}

				if (m_currentFrame)
				{
					const auto curvt = (*m_currentFrame)[vertIndex];
					if (m_nextFrame)
					{
						const auto nextvt = (*m_nextFrame)[vertIndex];
						const auto lerpvt =
							(nextvt - curvt) * m_animLerpAlpha + curvt;
						pos = {lerpvt.x, lerpvt.y, lerpvt.z, 1.f};
					}
					else
					{
						pos = {curvt.x, curvt.y, curvt.z, 1.f};
					}
				}

				pos = pos * localMtx * worldMtx * vp;
				gfxtri.m_vertices[i].m_position = pos;
				gfxtri.m_vertices[i].m_uv = tri.m_vertices[i].m_uv;

				vertIndex++;
			}
			gfxtri.m_colour = {1.f, 1.f, 1.f};
			gfxtri.m_texture = m_texture.get();
			gfxtri.m_cullType =
				GfxCullType_Frontface; // the winding order appears to be... not
									   // what i expected!
			GfxRenderer::GetInstance().SubmitTriToQueue(
				gfxtri, GfxRenderStrategy_SoftwareDepthRasterize);
		}
	}

}; // namespace oxygen
