#include "OxygenPCH.h"
#include "WorldLoader.h"
#include "World.h"
#include "BSP.h"
#include "Gfx/GfxRenderer.h"
#include "Platform/Platform.h"

namespace oxygen
{
	auto LoadWorld(std::string_view name) -> std::shared_ptr<World>
	{
		auto world = ObjectManager::GetInstance().CreateManagedObject<World>();

		auto bspData = std::make_unique<BSPWorldData>();
		if (!bspData->Load(name))
			return {};
		world->m_bspData = std::move(bspData);

		world->m_bspTextures.reserve(world->m_bspData->m_miptex.size());
		auto& gfx = GfxRenderer::GetInstance();
		for (const auto& mip : world->m_bspData->m_miptex)
		{
			const auto relpath = std::format(
				"{}/textures/{}.png", GetExecutableDirectory(), mip.m_name);
			world->m_bspTextures.push_back(gfx.LoadTexture(relpath));
		}
		{
			
			const auto lmpath = std::format("{}/textures/{}_lightmap0.png",
											GetExecutableDirectory(), name);
			world->m_lightmapTexture = gfx.LoadTexture(lmpath);
		}

		if (world->m_lightmapTexture)
		{
			const auto rectspath =
				std::format("{}/maps/{}_rects.bin", GetExecutableDirectory(), name);
			const auto lmwidth = world->m_lightmapTexture->m_width;
			const auto lmheight = world->m_lightmapTexture->m_height;
			const auto rectfile = CreateFileMap(rectspath);
			if (rectfile)
			{
				const auto u32ptr =
					reinterpret_cast<const oxyU32*>(rectfile->GetMap());
				if (rectfile->ValidateRange(u32ptr, 4 * sizeof(oxyU32)))
				{
					world->m_lightmapSampleSize = u32ptr[0];
					world->m_lightmapBlockWidth = u32ptr[1];
					world->m_lightmapBlockHeight = u32ptr[2];
					world->m_lightmapNumRects = u32ptr[3];
					if (rectfile->ValidateRange(u32ptr + 4,
												world->m_lightmapNumRects * 4 *
													sizeof(oxyU32)))
					{
						world->m_lightmapRects.reserve(
							world->m_lightmapNumRects);
						for (oxyU32 i = 0; i < world->m_lightmapNumRects; ++i)
						{
							const auto minx = u32ptr[4 + 4 * i];
							const auto miny = u32ptr[4 + 4 * i + 1];
							const auto maxx = u32ptr[4 + 4 * i + 2];
							const auto maxy = u32ptr[4 + 4 * i + 3];
							world->m_lightmapRects.push_back(
								std::array<oxyU32, 4>{minx, miny, maxx, maxy});
						}
					}
				}
			}
		}

		// Recurse nodes and store parents
		{
			world->m_bspNodeParents.resize(world->m_bspData->m_nodes.size());
			world->m_bspLeafParents.resize(world->m_bspData->m_leaves.size());
			const auto& nodes = world->m_bspData->m_nodes;
			OXYCHECK(nodes.size());
			const auto RecurseNode = [&](auto&& Self, oxyS16 nodeIndex,
										 oxyS16 parentIndex) {
				if (nodeIndex < 0)
				{
					const auto leafIndex = -nodeIndex - 1;
					world->m_bspLeafParents[leafIndex] = parentIndex;
					return;
				}
				world->m_bspNodeParents[nodeIndex] = parentIndex;
				const auto& node = nodes[nodeIndex];
				Self(Self, node.m_children[0], nodeIndex);
				Self(Self, node.m_children[1], nodeIndex);
			};
			RecurseNode(RecurseNode, 0, -1);
		}
		world->CreateEntitiesFromBSP();
		return world;
	}
}; // namespace oxygen