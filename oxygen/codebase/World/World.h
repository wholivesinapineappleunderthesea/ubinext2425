#pragma once

#include "BSP.h"

namespace oxygen
{
	struct World final : ManagedObject
	{
		OXYGENOBJECT(World, ManagedObject);

		std::unique_ptr<const BSPWorldData> m_bspData{};

		auto
		GetEntityList() const -> const std::vector<std::shared_ptr<struct Entity>>&
		{
			return m_entities;
		}

		auto RandomPlayerSpawn() const -> oxyVec3;

		auto FindLeaf(const oxyVec3& position,
					  oxySize modelIndex) const -> const BSPDefines::Leaf*;

		struct LineTraceResult
		{
			oxyBool m_allSolid{};
			oxyBool m_startSolid{};
			oxyF32 m_fraction{};
			oxyVec3 m_endPos{};
			oxyVec3 m_planeNormal{};
			oxyF32 m_planeDist{};
			std::shared_ptr<struct Entity> m_hitEntity{};
		};
		auto HullTrace(CollisionHull hull, const oxyVec3& start,
					   const oxyVec3& end,
					   LineTraceResult& result) const -> oxyBool;
		auto LineTrace(const oxyVec3& start, const oxyVec3& end,
					   const struct Entity* self,
					   LineTraceResult& result) const -> oxyBool;

		auto CalculateHullSlideMovement(CollisionHull hull,
										const oxyVec3& position,
										const oxyVec3& distance) -> oxyVec3;

		auto SpawnEntity(oxyObjectID id = 0) -> std::shared_ptr<Entity>;

		auto RemoveEntity(struct Entity* ent) -> void;

		auto GetLocalPlayer() const -> std::weak_ptr<Entity>
		{
			return m_localPlayer;
		}
		auto SetLocalPlayer(std::shared_ptr<Entity> player) -> void;

	  private:
		friend struct GameManager;
		friend auto LoadWorld(std::string_view name) -> std::shared_ptr<World>;
		auto SubmitBSPFacesToRenderQueue() -> void;

		oxyVec3 m_renderCameraPosition{};

		std::weak_ptr<struct Entity> m_localPlayer{};
		std::vector<std::shared_ptr<struct Entity>> m_entities;

		std::vector<std::shared_ptr<const struct GfxTexture>> m_bspTextures;
		std::shared_ptr<const struct GfxTexture> m_lightmapTexture;
		std::vector<std::array<oxyU32, 4>> m_lightmapRects;
		oxyU32 m_lightmapSampleSize{};
		oxyU32 m_lightmapBlockWidth{};
		oxyU32 m_lightmapBlockHeight{};
		oxyU32 m_lightmapNumRects{};
		struct WorldTri
		{
			oxyVec3 m_vertices[3];
			oxyVec2 m_texcoords[3];
			oxyVec2 m_lmtexcoords[3];
			oxyU32 m_textureIndex{};
		};
		std::vector<std::vector<WorldTri>> m_bspFaces;
		std::vector<oxyVec3> m_playerStarts;
		std::vector<oxyU8> m_cameraPVS;
		std::vector<oxyS16> m_bspNodeParents;
		std::vector<oxyS16> m_bspLeafParents;
		std::bitset<BSPDefines::k_MaxMapNodes> m_nodesMarkedForRender;
		std::bitset<BSPDefines::k_MaxMapFaces> m_facesMarkedForRender;

		//auto SummonPlayer(const EntitySummonParams& params)
		//	-> std::shared_ptr<Entity>;
		//auto SummonGrenadeProjectile(const EntitySummonParams& params)
		//	-> std::shared_ptr<Entity>;
		//auto SummonDebugCube(const EntitySummonParams& params)
		//	-> std::shared_ptr<Entity>;

		auto CreateEntitiesFromBSP() -> void;

		auto
		TestBoundsIntersectVisibleNodes(const oxyVec3& mins,
										const oxyVec3& maxs) const -> oxyBool;

		auto RenderTraverseBSPNode(oxyS32 nodeIndex,
								   const oxyVec3& origin) -> void;
		auto RenderBSPLeaf(const BSPDefines::Leaf& leaf,
						   const oxyVec3& origin) -> void;
		auto RenderBSPFace(struct GfxRenderer& gfx, oxySize faceindex,
						   const oxyVec3& origin) -> void;

		auto ComputeTriFaces() -> void;

		auto MarkPVSNodesFromLeaf(const BSPDefines::Leaf* leaf,
								  oxySize modelIndex) -> void;

		auto RecursiveClipNodeLineTrace(oxyS32 clipNodeIndex,
										const oxyVec3& start,
										const oxyVec3& end,
										LineTraceResult& result) const -> bool;
		auto RecursiveNodeLineTrace(oxyS32 nodeIndex, const oxyVec3& start,
									const oxyVec3& end,
									LineTraceResult& result) const -> bool;
		auto RecursiveSlideHull(oxyS32 rootClipNode, const oxyVec3& position,
								const oxyVec3& offset, int depth) -> oxyVec3;
		auto Update(float deltaTimeSeconds) -> void;
	};
}; // namespace oxygen