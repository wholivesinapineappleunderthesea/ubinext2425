#include "OxygenPCH.h"
#include "ResourceManager.h"

#include "Platform/Platform.h"

#include "StaticMeshResource.h"
#include "AnimatedMeshResource.h"

namespace oxygen
{
	auto ResourceManager::LoadStaticMesh(std::string_view name)
		-> std::shared_ptr<const StaticMeshResource>
	{
		const auto hash = std::hash<std::string_view>{}(name);
		if (const auto it = m_staticMeshes.find(hash);
			it != m_staticMeshes.end())
			if (auto ptr = it->second.lock(); ptr)
				return ptr;

		auto res = std::make_shared<StaticMeshResource>();
		const auto filemap = CreateFileMap(
			std::format("{}/meshes/{}.bin", GetExecutableDirectory(), name));
		if (!filemap)
			return {};
		struct Header
		{
			oxyU32 m_numPoints;
			oxyU32 m_numTris;
		};
		const auto hdr = reinterpret_cast<const Header*>(filemap->GetMap());
		if (!filemap->ValidateRange(hdr, sizeof(Header)))
			return {};
		const auto points = reinterpret_cast<const StaticMeshPointDef*>(
			reinterpret_cast<const oxyU8*>(filemap->GetMap()) + sizeof(Header));
		if (!filemap->ValidateRange(points, hdr->m_numPoints *
												sizeof(StaticMeshPointDef)) &&
			hdr->m_numPoints)
			return {};
		const auto tris = reinterpret_cast<const StaticMeshTri*>(
			reinterpret_cast<const oxyU8*>(points) +
			hdr->m_numPoints * sizeof(StaticMeshPointDef));
		if (!filemap->ValidateRange(tris,
									hdr->m_numTris * sizeof(StaticMeshTri)) &&
			hdr->m_numTris)
			return {};

		if (hdr->m_numPoints)
			res->m_points.assign(points, points + hdr->m_numPoints);
		if (hdr->m_numTris)
			res->m_tris.assign(tris, tris + hdr->m_numTris);

		// texture name is the footer, however long remains in the file
		const auto texname =
			reinterpret_cast<const char*>(tris + hdr->m_numTris);
		// calc len
		const auto len = filemap->GetSize() - sizeof(Header) -
						 hdr->m_numPoints * sizeof(StaticMeshPointDef) -
						 hdr->m_numTris * sizeof(StaticMeshTri);
		if (!filemap->ValidateRange(texname, len) && len)
			return {};
		res->m_texname = {texname, len};

		m_staticMeshes[hash] = res;
		return res;
	}
	auto ResourceManager::LoadAnimatedMesh(std::string_view name)
		-> std::shared_ptr<const AnimatedMeshResource>
	{
		const auto hash = std::hash<std::string_view>{}(name);
		if (const auto it = m_AnimatedMeshes.find(hash);
			it != m_AnimatedMeshes.end())
			if (auto ptr = it->second.lock(); ptr)
				return ptr;

		auto res = std::make_shared<AnimatedMeshResource>();
		res->m_rootPose = LoadStaticMesh(name);

		const auto filemap = CreateFileMap(std::format(
			"{}/anim/{}/anims.bin", GetExecutableDirectory(), name));
		if (!filemap)
			return {};
		auto ptr = reinterpret_cast<const oxyU8*>(filemap->GetMap());
		const auto ReadOutFrame = [&]() {
			oxyU32 hash = 0;
			if (!filemap->ValidateRange(ptr, sizeof hash))
				return false;
			hash = *reinterpret_cast<const oxyU32*>(ptr);
			ptr += sizeof hash;
			oxyU32 frameidx = 0;
			if (!filemap->ValidateRange(ptr, sizeof frameidx))
				return false;
			frameidx = *reinterpret_cast<const oxyU32*>(ptr);
			ptr += sizeof frameidx;
			oxyU32 numtris = 0;
			if (!filemap->ValidateRange(ptr, sizeof numtris))
				return false;
			numtris = *reinterpret_cast<const oxyU32*>(ptr);
			ptr += sizeof numtris;

			auto& animinfo = res->m_animations[hash];
			if (numtris)
			{
				animinfo.m_frames.resize(frameidx + 1);
				auto& vertsout = animinfo.m_frames[frameidx];
				vertsout.resize(numtris * 3);
				if (!filemap->ValidateRange(ptr, numtris * 3 * sizeof(oxyVec3)))
					return false;
				std::memcpy(vertsout.data(), ptr,
							numtris * 3 * sizeof(oxyVec3));
				ptr += numtris * 3 * sizeof(oxyVec3);
			}
			return true;
		};

		while (ReadOutFrame())
			;
		return res;
	}
}; // namespace oxygen