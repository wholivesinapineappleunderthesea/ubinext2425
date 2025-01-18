#include "OxygenPCH.h"
#include "BSP.h"

#include "Platform/Platform.h"

namespace oxygen
{
	auto BSPWorldData::Load(std::string_view mapname) -> oxyBool
	{
		const auto filemap = CreateFileMap(
			std::format("{}/maps/{}.bsp", GetExecutableDirectory(), mapname));
		if (!filemap)
			return false;

		const auto GetLumpInfo =
			[&](BSPDefines::LumpIndex index) -> std::pair<const void*, oxySize> {
			const auto header =
				reinterpret_cast<const BSPDefines::Header*>(filemap->GetMap());
			if (!filemap->ValidateRange(header, sizeof(BSPDefines::Header)))
				return {};

			BSPDefines::Lump lump;
			std::memcpy(&lump, &header->m_lumps[index], sizeof(BSPDefines::Lump));
			const auto ptr = reinterpret_cast<const oxyU8*>(filemap->GetMap()) +
							 lump.m_fileOffset;
			return {ptr, static_cast<oxySize>(lump.m_length)};
		};

		const auto LoadLump = [&](auto& storage,
								  BSPDefines::LumpIndex index) -> oxyBool {
			storage.clear();
			const auto [addr, size] = GetLumpInfo(index);
			if (!addr || !size)
				return false;

			if (!filemap->ValidateRange(addr, size))
				return false;

			const auto count =
				size /
				sizeof(std::remove_reference_t<decltype(storage)>::value_type);
			storage.resize(count);
			std::memcpy(storage.data(), addr, size);
			return true;
		};

		if (!LoadLump(m_planes, BSPDefines::LumpIndex_Planes))
			return false;

		// MipTex
		{
			const auto [addr, size] = GetLumpInfo(BSPDefines::LumpIndex_Textures);
			if (!addr || !size)
				return false;

			if (!filemap->ValidateRange(addr, size))
				return false;

			const auto miptexlump =
				reinterpret_cast<const BSPDefines::MipTexLump*>(addr);
			m_miptex.clear();
			m_miptex.resize(miptexlump->m_numMipTex);
			for (oxyU32 i = 0; i < miptexlump->m_numMipTex; i++)
			{
				const auto offset = miptexlump->m_dataOffsets[i];
				const auto miptex = reinterpret_cast<const BSPDefines::MipTex*>(
					reinterpret_cast<const oxyU8*>(miptexlump) + offset);
				std::memcpy(&m_miptex[i], miptex, sizeof(BSPDefines::MipTex));
			}
		}

		// Entities
		{
			const auto [addr, size] =
				GetLumpInfo(BSPDefines::LumpIndex_Entities);
			if (addr && size)
			{
				const auto end = reinterpret_cast<const char*>(addr) + size;
				auto start = reinterpret_cast<const char*>(addr);
				if (!filemap->ValidateRange(addr, size))
					return false;
				// TODO: CLEANUP!
				while (true)
				{
					std::unordered_map<std::string, std::string> entity;
					while (true)
					{
						const auto ReadNextKeyValuePair = [&]() { 
							if (*start == '{')
								++start;
							while (*start == ' ' || *start == '\t' ||
								   *start == '\n' || *start == '\r')
								++start;
							if (*start == '\"')
								++start;
							const auto keyend = std::strchr(start, '\"');
							if (!keyend)
								return false;
							const auto key = std::string{start, keyend};
							start = keyend + 1;
							while (*start == ' ' || *start == '\t' ||
								   *start == '\n' || *start == '\r')
								++start;
							if (*start != '\"')
								return false;
							++start;
							const auto valend = std::strchr(start, '\"');
							if (!valend)
								return false;
							const auto val = std::string{start, valend};
							start = valend + 1;
							entity[key] = val;
							return true;
						};
						if (!ReadNextKeyValuePair())
							break;
					}

					if (entity.empty())
						break;
					m_entitiesText.push_back(entity);
				}
			}
		}

		if (!LoadLump(m_vertices, BSPDefines::LumpIndex_Vertexes))
			return false;
		if (!LoadLump(m_visibility, BSPDefines::LumpIndex_Visibility))
			return false;
		if (!LoadLump(m_nodes, BSPDefines::LumpIndex_Nodes))
			return false;
		if (!LoadLump(m_texinfo, BSPDefines::LumpIndex_TexInfo))
			return false;
		if (!LoadLump(m_faces, BSPDefines::LumpIndex_Faces))
			return false;
		if (!LoadLump(m_clipNodes, BSPDefines::LumpIndex_ClipNodes))
			return false;
		if (!LoadLump(m_leaves, BSPDefines::LumpIndex_Leafs))
			return false;
		if (!LoadLump(m_marksurfaces, BSPDefines::LumpIndex_MarkSurfaces))
			return false;
		if (!LoadLump(m_edges, BSPDefines::LumpIndex_Edges))
			return false;
		if (!LoadLump(m_surfedges, BSPDefines::LumpIndex_SurfEdges))
			return false;
		if (!LoadLump(m_models, BSPDefines::LumpIndex_Models))
			return false;
		return true;
	}
};

