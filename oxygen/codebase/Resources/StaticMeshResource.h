#pragma once

namespace oxygen
{
	struct StaticMeshVertex
	{
		oxyVec3 m_position;
		oxyVec2 m_uv;
	};

	struct StaticMeshTri
	{
		StaticMeshVertex m_vertices[3];
	};

	struct StaticMeshPointDef
	{
		oxyU32 m_hash;
		oxyVec3 m_position;
	};

	struct StaticMeshResource
	{
		std::vector<StaticMeshPointDef> m_points;
		std::vector<StaticMeshTri> m_tris;
		std::string m_texname;
	};
};