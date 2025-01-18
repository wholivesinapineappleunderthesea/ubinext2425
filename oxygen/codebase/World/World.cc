#include "OxygenPCH.h"
#include "World.h"

#include "Entity/Entity.h"
#include "Component/HullComponent/HullComponent.h"
#include "Component/CameraComponent/CameraComponent.h"
// #include "Component/Pawn/Pawn.h"
// #include "Component/AnimatedMeshComponent/AnimatedMeshComponent.h"
// #include "Component/StaticMeshComponent/StaticMeshComponent.h"
// #include "Component/ProjectileComponent/ProjectileComponent.h"
#include "Component/EnvPushComponent/EnvPushComponent.h"

#include "Gfx/GfxRenderer.h"
#include "Input/InputManager.h"
#include "Platform/Platform.h"

namespace oxygen
{
	auto World::SubmitBSPFacesToRenderQueue() -> void
	{
		auto lp = m_localPlayer.lock();
		if (!lp)
			return;

		if (!m_bspFaces.size())
			ComputeTriFaces();

		m_nodesMarkedForRender.reset();
		m_facesMarkedForRender.reset();
		for (auto i = 0; i < m_bspData->m_models.size(); ++i)
		{
			const auto& model = m_bspData->m_models[i];
			const auto modelOrigin = oxyVec3{
				model.m_origin[0], model.m_origin[1], model.m_origin[2]};
			const auto camleaf =
				FindLeaf(m_renderCameraPosition - modelOrigin, i);
			if (!camleaf)
				continue;
			MarkPVSNodesFromLeaf(camleaf, i);
			RenderTraverseBSPNode(model.m_headNodes[0], modelOrigin);
		}

		for (auto& ent : m_entities)
		{
			if (ent->GetFlag(EntityFlags_Disabled))
				continue;
			if (!ent->GetFlag(EntityFlags_Renderable))
				continue;

			// ent->Render();
			const auto pos = ent->GetWorldPosition();
			const auto mins = pos + ent->GetRenderOcclusionMin();
			const auto maxs = pos + ent->GetRenderOcclusionMax();
			if (TestBoundsIntersectVisibleNodes(mins, maxs))
			{
				ent->Render();
			}
		}
	}
	auto World::Update(float deltaTimeSeconds) -> void
	{
		// Deliberately not using range-based for loop here incase entities
		// are added/removed during the loop
		for (oxySize i = 0; i < m_entities.size(); ++i)
		{
			const auto& ent = m_entities[i];
			if (ent->GetFlag(EntityFlags_Disabled))
				continue;
			ent->Update(deltaTimeSeconds);
		}

		auto lp = m_localPlayer.lock();
		if (lp)
		{
			const auto camera = lp->GetComponent<CameraComponent>();
			if (camera)
			{
				m_renderCameraPosition =
					lp->GetWorldPosition() + camera->GetCameraLocalOffset();
				GfxRenderer::GetInstance().SetViewProjectionMatrix(
					camera->GetViewProjectionMatrix());
			}
		}
	}

#if 0
	auto World::SummonPlayer(const EntitySummonParams& params)
		-> std::shared_ptr<Entity>
	{
		auto ent = SpawnEntity();
		ent->SetWorldPosition(params.m_position);
		ent->SetLocalRotation(params.m_rotation);
		ent->SetLocalScale({1.f, 1.f, 1.f});

		ent->SetFlag(EntityFlags_Dynamic, true);
		ent->SetFlag(EntityFlags_HasHull, true);
		ent->SetFlag(EntityFlags_Renderable, true);

		auto pawn = ent->AddComponent<Pawn>();

		auto smc = ent->AddComponent<AnimatedMeshComponent>();
		smc->LoadByName("Player3RD");
		pawn->m_thirdPersonMesh = smc.get();

		auto hull = ent->AddComponent<HullComponent>();
		hull->SetHull(CollisionHull_Player);
		hull->SetGravityPerSecond(700.f);
		hull->SetResponse(CollisionResponseType_Slide);

		auto camera = ent->AddComponent<CameraComponent>();
		camera->SetLocalOffset(oxyVec3{0.f, 0.f, 32.f});
		return ent;
	}

	auto World::SummonGrenadeProjectile(const EntitySummonParams& params)
		-> std::shared_ptr<Entity>
	{
		auto ent = SpawnEntity();
		ent->SetWorldPosition(params.m_position);
		ent->SetLocalRotation(params.m_rotation);
		ent->SetLocalScale({1.f, 1.f, 1.f});

		ent->SetFlag(EntityFlags_Dynamic, true);
		ent->SetFlag(EntityFlags_HasHull, true);
		ent->SetFlag(EntityFlags_Renderable, true);

		auto smc = ent->AddComponent<StaticMeshComponent>();
		smc->LoadByName("DebugCube");
		smc->SetHidden(false);

		auto hull = ent->AddComponent<HullComponent>();
		hull->SetHull(CollisionHull_Grenade);
		hull->SetGravityPerSecond(800.f);
		hull->SetResponse(CollisionResponseType_Bounce);
		hull->SetVelocity(params.m_velocity);

		auto proj = ent->AddComponent<ProjectileComponent>();
		proj->SetHull(hull.get());
		proj->SetBouncesLeft(3);
		proj->SetDamage(100.f);
		proj->SetDamageRadius(100.f);
		return ent;
	}

	auto World::SummonDebugCube(const EntitySummonParams& params)
		-> std::shared_ptr<Entity>
	{
		auto ent = SpawnEntity();
		ent->SetWorldPosition(params.m_position);
		ent->SetLocalRotation(params.m_rotation);
		ent->SetLocalScale({1.f, 1.f, 1.f});

		ent->SetFlag(EntityFlags_Dynamic, true);
		ent->SetFlag(EntityFlags_Renderable, true);

		auto smc = ent->AddComponent<StaticMeshComponent>();
		smc->LoadByName("DebugCube");
		smc->SetHidden(false);
		return ent;
	}
#endif

	auto World::CreateEntitiesFromBSP() -> void
	{
		const auto ParseVec3 = [](const std::string& s) -> oxyVec3 {
			oxyVec3 v{};
			sscanf_s(s.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
			return v;
		};

		for (const auto& ent : m_bspData->m_entitiesText)
		{
			auto classname = ent.find("classname");
			if (classname == ent.end())
				continue;
			if (classname->second == "env_push")
			{
				auto origin = ent.find("origin");
				if (origin == ent.end())
					continue;
				auto vel = ent.find("vel");
				if (vel == ent.end())
					continue;
				auto radius = ent.find("radius");
				if (radius == ent.end())
					continue;

				const oxyVec3 pos = ParseVec3(origin->second);
				const oxyVec3 velocity = ParseVec3(vel->second);
				const oxyF32 rad = std::stof(radius->second);

				auto ent = SpawnEntity();
				ent->SetWorldPosition(pos);
				auto envpush = ent->AddComponent<EnvPushComponent>();
				envpush->SetVelocity(velocity);
				envpush->SetRadius(rad);
			}
			else if (classname->second == "info_player_start")
			{
				auto origin = ent.find("origin");
				if (origin == ent.end())
					continue;
				m_playerStarts.push_back(ParseVec3(origin->second));
			}
		}
	}

	auto
	World::TestBoundsIntersectVisibleNodes(const oxyVec3& mins,
										   const oxyVec3& maxs) const -> oxyBool
	{
		// NOTE: this won't work because the root world node is always marked
		// (it goes up the hierarchy, ugh!)
		// maybe store the pvs decompress data separately?

		// OR:
		// walk the tree (recurse) and find the smallest node that contains this
		// box, if that isnt marked, we skip the entity

		for (oxyU32 i = 0; i < m_cameraPVS.size() * 8; ++i)
		{
			const auto pvsIndex = i >> 3;
			const auto maskValue = 1 << (i & 7);
			if (m_cameraPVS[pvsIndex] & maskValue)
			{
				const auto& leaf = m_bspData->m_leaves[i + 1];
				const auto leafMin =
					oxyVec3{static_cast<oxyF32>(leaf.m_mins[0]),
							static_cast<oxyF32>(leaf.m_mins[1]),
							static_cast<oxyF32>(leaf.m_mins[2])};
				const auto leafMax =
					oxyVec3{static_cast<oxyF32>(leaf.m_maxs[0]),
							static_cast<oxyF32>(leaf.m_maxs[1]),
							static_cast<oxyF32>(leaf.m_maxs[2])};
				// check for any overlap
				if (mins.x > leafMax.x || maxs.x < leafMin.x)
					continue;
				if (mins.y > leafMax.y || maxs.y < leafMin.y)
					continue;
				if (mins.z > leafMax.z || maxs.z < leafMin.z)
					continue;
				return true;
			}
		}

		return false;
	}

	auto World::RenderTraverseBSPNode(oxyS32 nodeIndex,
									  const oxyVec3& origin) -> void
	{
		if (nodeIndex < 0)
		{
			// Leaf
			const auto leafIdx = -nodeIndex - 1;

			if (leafIdx > 0)
			{
				const auto pvsIndex = (leafIdx -1) >> 3;
				const auto maskValue = 1 << ((leafIdx-1) & 7);
				if (!(m_cameraPVS[pvsIndex] & maskValue))
				{
					return;
				}
			}

			const auto& leaf = m_bspData->m_leaves[leafIdx];
			RenderBSPLeaf(leaf, origin);

			return;
		}
		if (!m_nodesMarkedForRender.test(nodeIndex))
			return;
		const auto& node = m_bspData->m_nodes[nodeIndex];
		const auto& plane = m_bspData->m_planes[node.m_planeIndex];
		const auto& planeNormal = plane.m_normal;
		const auto& planeDist = plane.m_dist;

		const auto dist =
			oxyVec3{planeNormal[0], planeNormal[1], planeNormal[2]}.DotProduct(
				m_renderCameraPosition) -
			planeDist;
		const auto side = dist >= 0.f ? 1 : 0;

		RenderTraverseBSPNode(node.m_children[side], origin);
		RenderTraverseBSPNode(node.m_children[1 - side], origin);
	}

	auto World::RenderBSPLeaf(const BSPDefines::Leaf& leaf,
							  const oxyVec3& origin) -> void
	{
		for (auto i = 0; i < leaf.m_markSurfaceCount; ++i)
		{
			const auto faceIdx =
				m_bspData->m_marksurfaces[leaf.m_firstMarkSurfaceIndex + i];
			if (!m_facesMarkedForRender.test(faceIdx))
			{
				m_facesMarkedForRender.set(faceIdx);
				RenderBSPFace(GfxRenderer::GetInstance(), faceIdx, origin);
			}
		}
	}

	auto World::RenderBSPFace(GfxRenderer& gfx, oxySize faceindex,
							  const oxyVec3& origin) -> void
	{
		const auto& tris = m_bspFaces[faceindex];
		const auto& bspface = m_bspData->m_faces[faceindex];
		for (const auto& tri : tris)
		{
			if (m_lightmapTexture && bspface.m_lightMapOffset != -1 &&
				bspface.m_lightStyles[0] != 255 &&
				m_lightmapRects.size() > faceindex)
			{ 
				oxyVec4 v0 =
					oxyVec4{tri.m_vertices[0], 1.f} + oxyVec4{origin, 0.0f};
				oxyVec4 v1 =
					oxyVec4{tri.m_vertices[1], 1.f} + oxyVec4{origin, 0.0f};
				oxyVec4 v2 =
					oxyVec4{tri.m_vertices[2], 1.f} + oxyVec4{origin, 0.0f};
				GfxTri triclip;
				triclip.m_vertices[0].m_position = v0;
				triclip.m_vertices[1].m_position = v1;
				triclip.m_vertices[2].m_position = v2;
				triclip.m_vertices[0].m_uv = tri.m_texcoords[0];
				triclip.m_vertices[1].m_uv = tri.m_texcoords[1];
				triclip.m_vertices[2].m_uv = tri.m_texcoords[2];
				triclip.m_colour = {1.f, 1.f, 1.f};
				triclip.m_texture = m_bspTextures[tri.m_textureIndex].get();
				triclip.m_cullType = GfxCullType_Backface;
				for (auto& vert : triclip.m_vertices)
				{
					vert.m_position =
						vert.m_position * gfx.GetViewProjectionMatrix();
				}
				gfx.SubmitTriToQueue(
					triclip, GfxRenderStrategy_SoftwareDepthRasterizePreSorted);

				triclip.m_vertices[0].m_uv = tri.m_lmtexcoords[0];
				triclip.m_vertices[1].m_uv = tri.m_lmtexcoords[1];
				triclip.m_vertices[2].m_uv = tri.m_lmtexcoords[2];
				triclip.m_texture = m_lightmapTexture.get();
				gfx.SubmitTriToQueue(
					triclip, GfxRenderStrategy_SoftwareDepthRasterizePreSortedOverlay);
			}
			else if (!m_lightmapTexture || bspface.m_lightMapOffset == -1)
			{
				oxyVec4 v0 =
					oxyVec4{tri.m_vertices[0], 1.f} + oxyVec4{origin, 0.0f};
				oxyVec4 v1 =
					oxyVec4{tri.m_vertices[1], 1.f} + oxyVec4{origin, 0.0f};
				oxyVec4 v2 =
					oxyVec4{tri.m_vertices[2], 1.f} + oxyVec4{origin, 0.0f};
				GfxTri triclip;
				triclip.m_vertices[0].m_position = v0;
				triclip.m_vertices[1].m_position = v1;
				triclip.m_vertices[2].m_position = v2;
				triclip.m_vertices[0].m_uv = tri.m_texcoords[0];
				triclip.m_vertices[1].m_uv = tri.m_texcoords[1];
				triclip.m_vertices[2].m_uv = tri.m_texcoords[2];
				triclip.m_colour = {1.f, 1.f, 1.f};
				triclip.m_texture = m_bspTextures[tri.m_textureIndex].get();
				for (auto& vert : triclip.m_vertices)
				{
					vert.m_position =
						vert.m_position * gfx.GetViewProjectionMatrix();
				}
				gfx.SubmitTriToQueue(
					triclip, GfxRenderStrategy_SoftwareDepthRasterizePreSorted);
			}
		}
	}
	auto World::ComputeTriFaces() -> void
	{
		m_bspFaces.resize(m_bspData->m_faces.size());
		for (oxySize faceindex = 0; faceindex < m_bspData->m_faces.size();
			 ++faceindex)
		{
			const auto& face = m_bspData->m_faces[faceindex];
			static constexpr auto k_maxFaceVertices = 128;
			oxyVec3 faceVerts[k_maxFaceVertices];
			oxyVec2 faceUVs[k_maxFaceVertices];
			oxySize numVerts{};

			const auto& surfedges = m_bspData->m_surfedges;
			const auto& edges = m_bspData->m_edges;
			const auto& vertices = m_bspData->m_vertices;

			const auto& texinfo = m_bspData->m_texinfo[face.m_texInfoIndex];
			const auto& miptex = m_bspData->m_miptex[texinfo.m_mipTexIndex];
			const auto& texvecs = texinfo.m_vecs[0];
			const auto& texvect = texinfo.m_vecs[1];

			const auto invWidth = 1.f / miptex.m_width;
			const auto invHeight = 1.f / miptex.m_height;

			const auto texidx = texinfo.m_mipTexIndex;

			const auto firstEdgeIdx = face.m_firstEdgeIndex;
			const auto edgeCount = face.m_edgeCount;

			oxyVec2 minuv{(std::numeric_limits<float>::max)(),
						  (std::numeric_limits<float>::max)()};
			oxyVec2 maxuv{(std::numeric_limits<float>::min)(),
						  (std::numeric_limits<float>::min)()};
			for (auto j = firstEdgeIdx;
				 j < firstEdgeIdx + edgeCount && numVerts < k_maxFaceVertices;
				 ++j)
			{
				const auto surfedge = surfedges[j];
				const auto& edge = edges[std::abs(surfedge)];
				const auto& v0 = vertices[edge.m_vertexIndices[0]].m_position;
				const auto& v1 = vertices[edge.m_vertexIndices[1]].m_position;

				const auto cvert = surfedge < 0 ? v1 : v0;
				const auto u = (texvecs[0] * cvert[0] + texvecs[1] * cvert[1] +
								texvecs[2] * cvert[2]) +
							   texvecs[3];
				const auto v = (texvect[0] * cvert[0] + texvect[1] * cvert[1] +
								texvect[2] * cvert[2]) +
							   texvect[3];

				faceVerts[numVerts] = oxyVec3{cvert[0], cvert[1], cvert[2]};
				faceUVs[numVerts] = {u, v};
				++numVerts;
				minuv.x = (std::min)(minuv.x, u);
				minuv.y = (std::min)(minuv.y, v);
				maxuv.x = (std::max)(maxuv.x, u);
				maxuv.y = (std::max)(maxuv.y, v);
			}

			auto& outtris = m_bspFaces[faceindex];
			for (auto i = 0; i < numVerts - 1; i++)
			{
				const auto& vtx0 = faceVerts[0];
				const auto& vtx1 = faceVerts[i];
				const auto& vtx2 = faceVerts[i + 1];

				const auto rect = m_lightmapRects[faceindex];
				const auto sampleSize = m_lightmapSampleSize;
				const auto blockWidth = m_lightmapBlockWidth;
				const auto blockHeight = m_lightmapBlockHeight;

				const auto CalcLightmapUV =
					[&](const oxyVec2& texuv) -> oxyVec2 {
					// interpolate texuv into minuv and maxuv

					const auto ttexu =
						(texuv.x - minuv.x) / (maxuv.x - minuv.x);
					const auto ttexv =
						(texuv.y - minuv.y) / (maxuv.y - minuv.y);
					// then interpolate into rect
					const auto lminx = rect[0] + 1;
					const auto lminy = rect[1] + 1;
					const auto lmaxx = rect[0] + rect[2] - 1;
					const auto lmaxy = rect[1] + rect[3] - 1;
					const auto lmu = lminx + ttexu * (lmaxx - lminx);
					const auto lmv = lminy + ttexv * (lmaxy - lminy);
					return {lmu / m_lightmapTexture->m_width,
							lmv / m_lightmapTexture->m_height};
				};

				WorldTri tri;
				tri.m_vertices[0] = vtx0;
				tri.m_vertices[1] = vtx1;
				tri.m_vertices[2] = vtx2;
				tri.m_texcoords[0] = faceUVs[0] * oxyVec2{invWidth, invHeight};
				tri.m_texcoords[1] = faceUVs[i] * oxyVec2{invWidth, invHeight};
				tri.m_texcoords[2] =
					faceUVs[i + 1] * oxyVec2{invWidth, invHeight};
				if (m_lightmapTexture && face.m_lightMapOffset != -1 &&
					face.m_lightStyles[0] != 255 &&
					m_lightmapRects.size() > faceindex)
				{
					tri.m_lmtexcoords[0] = CalcLightmapUV(faceUVs[0]);
					tri.m_lmtexcoords[1] = CalcLightmapUV(faceUVs[i]);
					tri.m_lmtexcoords[2] = CalcLightmapUV(faceUVs[i + 1]);
				}
				tri.m_textureIndex = texidx;
				outtris.push_back(tri);
			}
		}
	}

	auto World::MarkPVSNodesFromLeaf(const BSPDefines::Leaf* leaf,
									 oxySize modelIndex) -> void
	{
		const auto& model = m_bspData->m_models[modelIndex];
		const auto visclusters = model.m_visLeafs;
		if (leaf->m_visOffset == -1)
		{
			// mark all nodes in model if no PVS
			const auto MarkNodesRecurse = [&](auto&& self, oxyS32 nodeIndex) {
				if (nodeIndex < 0)
					return;
				m_nodesMarkedForRender.set(nodeIndex);
				self(self, m_bspData->m_nodes[nodeIndex].m_children[0]);
				self(self, m_bspData->m_nodes[nodeIndex].m_children[1]);
			};
			MarkNodesRecurse(MarkNodesRecurse, model.m_headNodes[0]);
		}
		const auto visbytes = (visclusters + 7) >> 3;
		const auto DecompressVis = [&](const oxyU8* in,
									   std::span<oxyU8> out) -> void {
			auto dst = out.data();
			while (dst < out.data() + out.size_bytes())
			{
				if (*in)
				{
					*dst++ = *in++;
				}
				else
				{
					auto c = in[1];
					if (c > out.data() + out.size_bytes() - dst)
						c = static_cast<oxyU8>(out.data() + out.size_bytes() -
											   dst);
					std::memset(dst, 0, c);
					in += 2;
					dst += c;
				}
			}
		};
		const auto visStart =
			m_bspData->m_visibility.data() + leaf->m_visOffset;
		m_cameraPVS.clear();
		m_cameraPVS.resize(visbytes);
		DecompressVis(visStart, {m_cameraPVS.data(), visbytes});
		const auto& leaves = m_bspData->m_leaves;
		for (oxyU32 i = 0; i < visbytes * 8; ++i)
		{
			const auto pvsIndex = i >> 3;
			const auto maskValue = 1 << (i & 7);
			if (m_cameraPVS[pvsIndex] & maskValue)
			{
				auto nodeIndex =
					m_bspLeafParents[i + 1]; // +1 because 0 is void
				while (nodeIndex >= 0)
				{
					if (m_nodesMarkedForRender.test(nodeIndex))
						break;
					m_nodesMarkedForRender.set(nodeIndex);
					nodeIndex = m_bspNodeParents[nodeIndex];
				}
			}
		}
	}

	auto
	World::RecursiveClipNodeLineTrace(oxyS32 clipNodeIndex,
									  const oxyVec3& start, const oxyVec3& end,
									  LineTraceResult& result) const -> bool
	{
		if (clipNodeIndex < 0)
		{
			// negative clipnode children are contents not leaf indices
			const auto contents = clipNodeIndex;
			if (contents == BSPDefines::Contents_Solid)
			{
				result.m_startSolid = true;
				result.m_endPos = start;
				return true;
			}
			result.m_allSolid = false;
			return false;
		}
		const auto& node = m_bspData->m_clipNodes[clipNodeIndex];
		const auto& plane = m_bspData->m_planes[node.m_planeIndex];
		const auto t1 = start.DotProduct({plane.m_normal[0], plane.m_normal[1],
										  plane.m_normal[2]}) -
						plane.m_dist;
		const auto t2 = end.DotProduct({plane.m_normal[0], plane.m_normal[1],
										plane.m_normal[2]}) -
						plane.m_dist;
		if (t1 >= 0 && t2 >= 0)
			return RecursiveClipNodeLineTrace(node.m_children[0], start, end,
											  result);
		if (t1 < 0 && t2 < 0)
			return RecursiveClipNodeLineTrace(node.m_children[1], start, end,
											  result);

		// Resolves issues where the start is **just** inside of a solid leaf
		constexpr auto k_lineTraceDistEpsilon = 0.032f;
		oxyF32 frac;
		if (t1 < 0)
			frac = (t1 + k_lineTraceDistEpsilon) / (t1 - t2);
		else
			frac = (t1 - k_lineTraceDistEpsilon) / (t1 - t2);
		if (frac < 0)
			frac = 0;
		if (frac > 1)
			frac = 1;
		const auto mid = start + (end - start) * frac;
		const auto side = (t1 >= 0) ? 0 : 1;
		if (!side)
		{
			result.m_planeNormal = {plane.m_normal[0], plane.m_normal[1],
									plane.m_normal[2]};
			result.m_planeDist = plane.m_dist;
		}
		else
		{
			result.m_planeNormal = {-plane.m_normal[0], -plane.m_normal[1],
									-plane.m_normal[2]};
			result.m_planeDist = -plane.m_dist;
		}
		if (RecursiveClipNodeLineTrace(node.m_children[side], start, mid,
									   result))
			return true;
		return RecursiveClipNodeLineTrace(node.m_children[1 - side], mid, end,
										  result);
	}
	auto World::RecursiveNodeLineTrace(oxyS32 nodeIndex, const oxyVec3& start,
									   const oxyVec3& end,
									   LineTraceResult& result) const -> bool
	{
		if (nodeIndex < 0)
		{
			const auto& leaf = m_bspData->m_leaves[-nodeIndex - 1];
			const auto contents = leaf.m_contents;
			if (contents == BSPDefines::Contents_Solid)
			{
				result.m_startSolid = true;
				result.m_endPos = start;
				return true;
			}
			result.m_allSolid = false;
			return false;
		}
		const auto& node = m_bspData->m_nodes[nodeIndex];
		const auto& plane = m_bspData->m_planes[node.m_planeIndex];
		const auto t1 = start.DotProduct({plane.m_normal[0], plane.m_normal[1],
										  plane.m_normal[2]}) -
						plane.m_dist;
		const auto t2 = end.DotProduct({plane.m_normal[0], plane.m_normal[1],
										plane.m_normal[2]}) -
						plane.m_dist;
		if (t1 >= 0 && t2 >= 0)
			return RecursiveNodeLineTrace(node.m_children[0], start, end,
										  result);
		if (t1 < 0 && t2 < 0)
			return RecursiveNodeLineTrace(node.m_children[1], start, end,
										  result);

		// Resolves issues where the start is **just** inside of a solid leaf
		constexpr auto k_lineTraceDistEpsilon = 0.032f;
		oxyF32 frac;
		if (t1 < 0)
			frac = (t1 + k_lineTraceDistEpsilon) / (t1 - t2);
		else
			frac = (t1 - k_lineTraceDistEpsilon) / (t1 - t2);
		if (frac < 0)
			frac = 0;
		if (frac > 1)
			frac = 1;
		const auto mid = start + (end - start) * frac;
		const auto side = (t1 >= 0) ? 0 : 1;
		if (!side)
		{
			result.m_planeNormal = {plane.m_normal[0], plane.m_normal[1],
									plane.m_normal[2]};
			result.m_planeDist = plane.m_dist;
		}
		else
		{
			result.m_planeNormal = {-plane.m_normal[0], -plane.m_normal[1],
									-plane.m_normal[2]};
			result.m_planeDist = -plane.m_dist;
		}
		if (RecursiveNodeLineTrace(node.m_children[side], start, mid, result))
			return true;
		return RecursiveNodeLineTrace(node.m_children[1 - side], mid, end,
									  result);
	}
	auto World::RandomPlayerSpawn() const -> oxyVec3
	{
		if (m_playerStarts.empty())
			return {};
		auto rnd = RandomS32(0, m_playerStarts.size() - 1);
		return m_playerStarts[rnd];
	}
	auto World::FindLeaf(const oxyVec3& position,
						 oxySize modelIndex) const -> const BSPDefines::Leaf*
	{
		const auto& leaves = m_bspData->m_leaves;
		const auto& nodes = m_bspData->m_nodes;
		oxyS32 nodeIndex = m_bspData->m_models[modelIndex].m_headNodes[0];
		while (nodeIndex >= 0)
		{
			const auto& node = nodes[nodeIndex];
			const auto& plane = m_bspData->m_planes[node.m_planeIndex];
			const auto& planeNormal = plane.m_normal;
			const auto& planeDist = plane.m_dist;

			const auto dist = planeNormal[0] * position.x +
							  planeNormal[1] * position.y +
							  planeNormal[2] * position.z - planeDist;
			const auto side = dist >= 0.f ? 0 : 1;
			nodeIndex = node.m_children[side];
		}
		const auto leafIdx = -nodeIndex - 1;
		return &leaves[leafIdx];
	}
	auto World::HullTrace(CollisionHull hull, const oxyVec3& start,
						  const oxyVec3& end,
						  LineTraceResult& result) const -> oxyBool
	{
		if (hull == CollisionHull::CollisionHull_None)
			return false;
		const auto rootClipNode =
			m_bspData->m_models[0].m_headNodes[static_cast<int>(hull)];
		const auto bspTrace =
			RecursiveClipNodeLineTrace(rootClipNode, start, end, result);
		return bspTrace;
	}
	auto World::LineTrace(const oxyVec3& start, const oxyVec3& end,
						  const Entity* self,
						  LineTraceResult& result) const -> oxyBool
	{
		const auto rootNode = m_bspData->m_models[0].m_headNodes[0];
		const auto bspTrace =
			RecursiveNodeLineTrace(rootNode, start, end, result);
		oxyF32 minDistSq = (end - start).MagnitudeSquared();
		if (bspTrace)
			minDistSq = (result.m_endPos - start).MagnitudeSquared();
		oxyBool hitHull = false;
		for (const auto& ent : m_entities)
		{
			if (ent.get() == self)
				continue;
			if (ent->GetFlag(EntityFlags_Disabled))
				continue;
			if (!ent->GetFlag(EntityFlags_HasHull))
				continue;
			const auto hullcomp = ent->GetComponent<HullComponent>();
			if (!hullcomp)
				continue;
			if (!hullcomp->IsEnabled())
				continue;
			if (hullcomp->DoesIgnoreEntity(self))
				continue;
			oxyVec3 hullHitPos;
			oxyVec3 hullHitNormal;
			if (hullcomp->TraceLine(start, end, hullHitPos, hullHitNormal))
			{
				const auto distSq = (hullHitPos - start).MagnitudeSquared();
				if (distSq < minDistSq)
				{
					minDistSq = distSq;
					result.m_endPos = hullHitPos;
					result.m_planeNormal = hullHitNormal;
					result.m_planeDist = 1.f;
					result.m_hitEntity = ent;
					hitHull = true;
				}
			}
		}
		return bspTrace; // || hitHull;
	}
	auto World::CalculateHullSlideMovement(CollisionHull hull,
										   const oxyVec3& position,
										   const oxyVec3& distance) -> oxyVec3
	{
		if (hull == CollisionHull::CollisionHull_None)
			return position + distance;
		const auto rootNode =
			m_bspData->m_models[0].m_headNodes[static_cast<int>(hull)];
		oxyVec3 step{};
		step += RecursiveSlideHull(rootNode, position,
								   distance * oxyVec3{1, 0, 0}, 0);
		step += RecursiveSlideHull(rootNode, position + step,
								   distance * oxyVec3{0, 1, 0}, 0);
		step += RecursiveSlideHull(rootNode, position + step,
								   distance * oxyVec3{0, 0, 1}, 0);
		auto finalPos = position + step;
		// for (const auto& ent : m_entities)
		//{
		//	if (ent.get() == self)
		//		continue;
		//	const auto hullcomp = ent->GetComponent<HullComponent>();
		//	if (!hullcomp)
		//		continue;
		//	if (hullcomp->DoesIgnoreEntity(self))
		//		continue;
		//
		//	oxyVec3 hullCollidePos;
		//	oxyVec3 hullCollideNormal;
		//	if (hullcomp->CollidesWithHull(finalPos, hull, hullCollidePos,
		//								   hullCollideNormal))
		//	{
		//		// push away from the hull
		//		// the collide normal is scaled by the penetration depth
		//		finalPos -= hullCollideNormal;
		//	}
		// }

		return finalPos;
	}
	auto World::SpawnEntity(oxyObjectID id) -> std::shared_ptr<Entity>
	{
		auto ent = ObjectManager::GetInstance().CreateManagedObject<Entity>(id);
		ent->m_world = GetHardRef<World>();
		m_entities.push_back(ent);
		return ent;
	}
	auto World::RemoveEntity(Entity* ent) -> void
	{
		auto it = std::find_if(m_entities.begin(), m_entities.end(),
							   [ent](const auto& e) { return e.get() == ent; });
		if (it != m_entities.end())
		{
			(*it)->m_world.reset();
			m_entities.erase(it);
		}
	}
	auto World::SetLocalPlayer(std::shared_ptr<Entity> player) -> void
	{
		const auto oldplayer = m_localPlayer.lock();
		if (oldplayer)
		{
			oldplayer->SetFlag(EntityFlags_IsLocalPlayer, false);
		}
		m_localPlayer = player;
		if (player)
		{
			player->SetFlag(EntityFlags_IsLocalPlayer, true);
		}
	}
	auto World::RecursiveSlideHull(oxyS32 rootClipNode, const oxyVec3& position,
								   const oxyVec3& offset, int depth) -> oxyVec3
	{
		if (depth > 4)
			return {};
		LineTraceResult hitResult;
		if (RecursiveClipNodeLineTrace(rootClipNode, position,
									   position + offset, hitResult))
		{
			// Project offset onto the plane normal
			auto invNormal =
				-oxyVec3{hitResult.m_planeNormal.x, hitResult.m_planeNormal.y,
						 hitResult.m_planeNormal.z};
			invNormal = invNormal * (offset * invNormal).Magnitude();
			auto wallDir = offset - invNormal;
			auto newPos = position + wallDir;
			auto newOffset = newPos - position;
			return RecursiveSlideHull(rootClipNode, newPos, newOffset,
									  depth + 1);
		}
		return offset;
	}

}; // namespace oxygen
