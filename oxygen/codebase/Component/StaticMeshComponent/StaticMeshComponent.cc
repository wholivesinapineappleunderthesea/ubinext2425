#include "OxygenPCH.h"
#include "StaticMeshComponent.h"
#include "Entity/Entity.h"

#include "Resources/ResourceManager.h"
#include "Resources/StaticMeshResource.h"

#include "Gfx/GfxRenderer.h"
#include "Platform/Platform.h"

namespace oxygen
{
	auto StaticMeshComponent::LoadByName(std::string_view name) -> oxyBool
	{
		m_resource = ResourceManager::GetInstance().LoadStaticMesh(name);
		if (m_resource)
		{
			m_texture = GfxRenderer::GetInstance().LoadTexture(
				std::format("{}/textures/{}.png", GetExecutableDirectory(),
							m_resource->m_texname));
		}
		return m_resource != nullptr;
	}
	auto StaticMeshComponent::Render() const -> void
	{
		if (!m_resource)
			return;
		const auto ent = GetEntity();
		const auto pos = ent->GetWorldPosition() + m_localOffset;
		const auto rot = ent->GetWorldRotation();
		const auto scale = ent->GetWorldScale();
		oxyMat4x4 worldMtx = oxyMat4x4::Identity();
		worldMtx = Math::Translate(worldMtx, pos);
		worldMtx = Math::Rotate(worldMtx, rot);
		worldMtx = Math::Scale(worldMtx, scale);
		const auto vp = GfxRenderer::GetInstance().GetViewProjectionMatrix();
		for (const auto& tri : m_resource->m_tris)
		{
			GfxTri gfxtri;
			for (auto i = 0; i < 3; ++i)
			{
				gfxtri.m_vertices[i].m_position =
					oxyVec4{tri.m_vertices[i].m_position, 1.f} * worldMtx *
					vp;
				gfxtri.m_vertices[i].m_uv = tri.m_vertices[i].m_uv;
			}
			gfxtri.m_colour = {1.f, 1.f, 1.f};
			gfxtri.m_texture = m_texture.get();
			gfxtri.m_cullType = GfxCullType_Frontface;
			GfxRenderer::GetInstance().SubmitTriToQueue(
				gfxtri, GfxRenderStrategy_SoftwareDepthRasterize);
		}
	}
}; // namespace oxygen