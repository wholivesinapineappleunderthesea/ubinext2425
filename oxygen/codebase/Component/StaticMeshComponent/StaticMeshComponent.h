#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct StaticMeshResource;

	struct StaticMeshComponent final : Component
	{
		OXYGENOBJECT(StaticMeshComponent, Component);

		auto LoadByName(std::string_view name) -> oxyBool;

		auto SetLocalOffset(const oxyVec3& offset) -> void
		{
			m_localOffset = offset;
		}

	  protected:
		auto Render() const -> void override;

	  private:
		oxyVec3 m_localOffset{};
		std::shared_ptr<const StaticMeshResource> m_resource;
		std::shared_ptr<const struct GfxTexture> m_texture;
	};
}; // namespace oxygen