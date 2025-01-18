#pragma once

namespace oxygen
{
	namespace BSPDefines
	{
		inline constexpr auto k_BSPVersion = oxyS32{30};
		inline constexpr auto k_ToolVersion = oxyS32{2};

		inline constexpr auto k_MaxMapHulls = oxySize{4};
		inline constexpr auto k_MaxMapModels = oxySize{400};
		inline constexpr auto k_MaxMapBrushes = oxySize{4096};
		inline constexpr auto k_MaxMapEntityString = oxySize{128 * 1024};
		inline constexpr auto k_MaxMapPlanes = oxySize{32767};
		inline constexpr auto k_MaxMapNodes = oxySize{32767};
		inline constexpr auto k_MaxMapClipNodes = oxySize{32767};
		inline constexpr auto k_MaxMapLeafs = oxySize{8192};
		inline constexpr auto k_MaxMapVertices = oxySize{65535};
		inline constexpr auto k_MaxMapFaces = oxySize{65535};
		inline constexpr auto k_MaxMapMarkSurfaces = oxySize{65535};
		inline constexpr auto k_MaxMapTexInfo = oxySize{8192};
		inline constexpr auto k_MaxMapEdges = oxySize{256000};
		inline constexpr auto k_MaxMapSurfEdges = oxySize{512000};
		inline constexpr auto k_MaxMapTextures = oxySize{512};
		inline constexpr auto k_MaxMapMipTex = oxySize{0x200000};
		inline constexpr auto k_MaxMapLighting = oxySize{0x200000};
		inline constexpr auto k_MaxMapVis = oxySize{0x200000};
		inline constexpr auto k_MaxMapPortals = oxySize{65536};

		inline constexpr auto k_NumMipLevels = 4;
		inline constexpr auto k_TexSpecial =
			1; // "sky or slime, no lightmap or 256 subdivision"
		inline constexpr auto k_MaxLightMaps = 4;
		inline constexpr auto k_NumAmbients = 4;

		enum Contents
		{
			Contents_Empty = -1,
			Contents_Solid = -2,
			Contents_Water = -3,
			Contents_Slime = -4,
			Contents_Lava = -5,
			Contents_Sky = -6,
			Contents_Origin = -7, // CSG brush (removed during build)
			Contents_Clip = -8,	  // Legacy (changed to Solid)
			Contents_Current0 = -9,
			Contents_Current90 = -10,
			Contents_Current180 = -11,
			Contents_Current270 = -12,
			Contents_CurrentUp = -13,
			Contents_CurrentDown = -14,
			Contents_Translucent = -15,
		};

		struct Lump
		{
			oxyU32 m_fileOffset;
			oxyU32 m_length;
		};
		static_assert(sizeof(Lump) == 8, "Lump struct size is not 8 bytes");
		static_assert(alignof(Lump) == 4,
					  "Lump struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Lump>,
					  "Lump struct is not a trivial type");
		static_assert(offsetof(Lump, m_fileOffset) == 0,
					  "Lump struct m_fileOffset offset is not 0");
		static_assert(offsetof(Lump, m_length) == 4,
					  "Lump struct m_length offset is not 4");

		enum LumpIndex
		{
			LumpIndex_Entities = 0,
			LumpIndex_Planes,
			LumpIndex_Textures,
			LumpIndex_Vertexes,
			LumpIndex_Visibility,
			LumpIndex_Nodes,
			LumpIndex_TexInfo,
			LumpIndex_Faces,
			LumpIndex_Lighting,
			LumpIndex_ClipNodes,
			LumpIndex_Leafs,
			LumpIndex_MarkSurfaces,
			LumpIndex_Edges,
			LumpIndex_SurfEdges,
			LumpIndex_Models,
			LumpIndex_Count
		};

		struct Header
		{
			oxyS32 m_version;
			Lump m_lumps[LumpIndex_Count];
		};
		static_assert(sizeof(Header) == 124,
					  "Header struct size is not 124 bytes");
		static_assert(alignof(Header) == 4,
					  "Header struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Header>,
					  "Header struct is not a trivial type");
		static_assert(offsetof(Header, m_version) == 0,
					  "Header struct m_version offset is not 0");
		static_assert(offsetof(Header, m_lumps) == 4,
					  "Header struct m_lumps offset is not 4");

		enum PlaneType
		{
			Plane_X = 0,
			Plane_Y,
			Plane_Z,
			Plane_AnyX,
			Plane_AnyY,
			Plane_AnyZ
		};

		struct Plane
		{
			oxyF32 m_normal[3];
			oxyF32 m_dist;
			oxyU32 m_type;
		};
		static_assert(sizeof(Plane) == 20, "Plane struct size is not 20 bytes");
		static_assert(alignof(Plane) == 4,
					  "Plane struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Plane>,
					  "Plane struct is not a trivial type");
		static_assert(offsetof(Plane, m_normal) == 0,
					  "Plane struct m_normal offset is not 0");
		static_assert(offsetof(Plane, m_dist) == 12,
					  "Plane struct m_dist offset is not 12");
		static_assert(offsetof(Plane, m_type) == 16,
					  "Plane struct m_type offset is not 16");

		struct MipTexLump
		{
			oxyU32 m_numMipTex;
			oxyU32 m_dataOffsets[4]; // [m_numMipTex]
		};
		static_assert(sizeof(MipTexLump) == 20,
					  "MipTexLump struct size is not 20 bytes");
		static_assert(alignof(MipTexLump) == 4,
					  "MipTexLump struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<MipTexLump>,
					  "MipTexLump struct is not a trivial type");
		static_assert(offsetof(MipTexLump, m_numMipTex) == 0,
					  "MipTexLump struct m_numMipTex offset is not 0");
		static_assert(offsetof(MipTexLump, m_dataOffsets) == 4,
					  "MipTexLump struct m_dataOffsets offset is not 4");

		struct MipTex
		{
			oxyChar m_name[16];
			oxyU32 m_width;
			oxyU32 m_height;
			oxyU32 m_offsets[k_NumMipLevels];
		};
		static_assert(sizeof(MipTex) == 40,
					  "MipTex struct size is not 40 bytes");
		static_assert(alignof(MipTex) == 4,
					  "MipTex struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<MipTex>,
					  "MipTex struct is not a trivial type");
		static_assert(offsetof(MipTex, m_name) == 0,
					  "MipTex struct m_name offset is not 0");
		static_assert(offsetof(MipTex, m_width) == 16,
					  "MipTex struct m_width offset is not 16");
		static_assert(offsetof(MipTex, m_height) == 20,
					  "MipTex struct m_height offset is not 20");
		static_assert(offsetof(MipTex, m_offsets) == 24,
					  "MipTex struct m_offsets offset is not 24");

		struct Vertex
		{
			oxyF32 m_position[3];
		};
		static_assert(sizeof(Vertex) == 12,
					  "Vertex struct size is not 12 bytes");
		static_assert(alignof(Vertex) == 4,
					  "Vertex struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Vertex>,
					  "Vertex struct is not a trivial type");
		static_assert(offsetof(Vertex, m_position) == 0,
					  "Vertex struct m_position offset is not 0");

		struct Node
		{
			oxyU32 m_planeIndex;
			oxyS16 m_children[2]; // Negative numbers are -(leafs+1)
			oxyS16 m_mins[3];
			oxyS16 m_maxs[3];
			oxyU16 m_firstFaceIndex;
			oxyU16 m_faceCount;
		};
		static_assert(sizeof(Node) == 24, "Node struct size is not 32 bytes");
		static_assert(alignof(Node) == 4,
					  "Node struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Node>,
					  "Node struct is not a trivial type");
		static_assert(offsetof(Node, m_planeIndex) == 0,
					  "Node struct m_planeIndex offset is not 0");
		static_assert(offsetof(Node, m_children) == 4,
					  "Node struct m_children offset is not 4");
		static_assert(offsetof(Node, m_mins) == 8,
					  "Node struct m_mins offset is not 8");
		static_assert(offsetof(Node, m_maxs) == 14,
					  "Node struct m_maxs offset is not 14");
		static_assert(offsetof(Node, m_firstFaceIndex) == 20,
					  "Node struct m_firstFaceIndex offset is not 20");
		static_assert(offsetof(Node, m_faceCount) == 22,
					  "Node struct m_faceCount offset is not 22");

		struct TexInfo
		{
			oxyF32 m_vecs[2][4]; // [s/t][xyz offset]
			oxyS32 m_mipTexIndex;
			oxyS32 m_flags;
		};
		static_assert(sizeof(TexInfo) == 40,
					  "TexInfo struct size is not 40 bytes");
		static_assert(alignof(TexInfo) == 4,
					  "TexInfo struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<TexInfo>,
					  "TexInfo struct is not a trivial type");
		static_assert(offsetof(TexInfo, m_vecs) == 0,
					  "TexInfo struct m_vecs offset is not 0");
		static_assert(offsetof(TexInfo, m_mipTexIndex) == 32,
					  "TexInfo struct m_mipTexIndex offset is not 32");
		static_assert(offsetof(TexInfo, m_flags) == 36,
					  "TexInfo struct m_flags offset is not 36");

		struct Face
		{
			oxyU16 m_planeIndex;
			oxyU16 m_side;
			oxyU32 m_firstEdgeIndex;
			oxyU16 m_edgeCount;
			oxyU16 m_texInfoIndex;
			oxyU8 m_lightStyles[k_MaxLightMaps];
			oxyU32 m_lightMapOffset;
		};
		static_assert(sizeof(Face) == 20, "Face struct size is not 20 bytes");
		static_assert(alignof(Face) == 4,
					  "Face struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Face>,
					  "Face struct is not a trivial type");
		static_assert(offsetof(Face, m_planeIndex) == 0,
					  "Face struct m_planeIndex offset is not 0");
		static_assert(offsetof(Face, m_side) == 2,
					  "Face struct m_side offset is not 2");
		static_assert(offsetof(Face, m_firstEdgeIndex) == 4,
					  "Face struct m_firstEdgeIndex offset is not 4");
		static_assert(offsetof(Face, m_edgeCount) == 8,
					  "Face struct m_edgeCount offset is not 8");
		static_assert(offsetof(Face, m_texInfoIndex) == 10,
					  "Face struct m_texInfoIndex offset is not 10");
		static_assert(offsetof(Face, m_lightStyles) == 12,
					  "Face struct m_lightStyles offset is not 12");
		static_assert(offsetof(Face, m_lightMapOffset) == 16,
					  "Face struct m_lightMapOffset offset is not 16");

		struct ClipNode
		{
			oxyU32 m_planeIndex;
			oxyS16 m_children[2]; // Negatives are contents
		};
		static_assert(sizeof(ClipNode) == 8,
					  "ClipNode struct size is not 8 bytes");
		static_assert(alignof(ClipNode) == 4,
					  "ClipNode struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<ClipNode>,
					  "ClipNode struct is not a trivial type");
		static_assert(offsetof(ClipNode, m_planeIndex) == 0,
					  "ClipNode struct m_planeIndex offset is not 0");
		static_assert(offsetof(ClipNode, m_children) == 4,
					  "ClipNode struct m_children offset is not 4");

		// "leaf 0 is the generic CONTENTS_SOLID leaf"
		struct Leaf
		{
			oxyS32 m_contents;
			oxyS32 m_visOffset; // -1 = none
			oxyS16 m_mins[3];	// "for frustum culling"
			oxyS16 m_maxs[3];
			oxyU16 m_firstMarkSurfaceIndex;
			oxyU16 m_markSurfaceCount;
			oxyU8 m_ambientLevels[k_NumAmbients];
		};
		static_assert(sizeof(Leaf) == 28, "Leaf struct size is not 28 bytes");
		static_assert(alignof(Leaf) == 4,
					  "Leaf struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Leaf>,
					  "Leaf struct is not a trivial type");
		static_assert(offsetof(Leaf, m_contents) == 0,
					  "Leaf struct m_contents offset is not 0");
		static_assert(offsetof(Leaf, m_visOffset) == 4,
					  "Leaf struct m_visOffset offset is not 4");
		static_assert(offsetof(Leaf, m_mins) == 8,
					  "Leaf struct m_mins offset is not 8");
		static_assert(offsetof(Leaf, m_maxs) == 14,
					  "Leaf struct m_maxs offset is not 14");
		static_assert(offsetof(Leaf, m_firstMarkSurfaceIndex) == 20,
					  "Leaf struct m_firstMarkSurfaceIndex offset is not 20");
		static_assert(offsetof(Leaf, m_markSurfaceCount) == 22,
					  "Leaf struct m_markSurfaceCount offset is not 22");
		static_assert(offsetof(Leaf, m_ambientLevels) == 24,
					  "Leaf struct m_ambientLevels offset is not 24");

		struct Edge
		{
			oxyU16 m_vertexIndices[2];
		};
		static_assert(sizeof(Edge) == 4, "Edge struct size is not 4 bytes");
		static_assert(alignof(Edge) == 2,
					  "Edge struct alignment is not 2 bytes");
		static_assert(std::is_trivial_v<Edge>,
					  "Edge struct is not a trivial type");
		static_assert(offsetof(Edge, m_vertexIndices) == 0,
					  "Edge struct m_vertexIndices offset is not 0");

		struct Model
		{
			oxyF32 m_mins[3];
			oxyF32 m_maxs[3];
			oxyF32 m_origin[3];
			oxyU32 m_headNodes[k_MaxMapHulls];
			oxyU32 m_visLeafs; // "not including the solid leaf 0"
			oxyU32 m_firstFaceIndex;
			oxyU32 m_faceCount;
		};
		static_assert(sizeof(Model) == 64, "Model struct size is not 64 bytes");
		static_assert(alignof(Model) == 4,
					  "Model struct alignment is not 4 bytes");
		static_assert(std::is_trivial_v<Model>,
					  "Model struct is not a trivial type");
		static_assert(offsetof(Model, m_mins) == 0,
					  "Model struct m_mins offset is not 0");
		static_assert(offsetof(Model, m_maxs) == 12,
					  "Model struct m_maxs offset is not 12");
		static_assert(offsetof(Model, m_origin) == 24,
					  "Model struct m_origin offset is not 24");
		static_assert(offsetof(Model, m_headNodes) == 36,
					  "Model struct m_headNodes offset is not 36");
		static_assert(offsetof(Model, m_visLeafs) == 52,
					  "Model struct m_visLeafs offset is not 52");
		static_assert(offsetof(Model, m_firstFaceIndex) == 56,
					  "Model struct m_firstFaceIndex offset is not 56");
		static_assert(offsetof(Model, m_faceCount) == 60,
					  "Model struct m_faceCount offset is not 60");

	}; // namespace BSPDefines

	struct BSPWorldData : NonCopyable
	{
		std::vector<std::unordered_map<std::string, std::string>> m_entitiesText;
		std::vector<BSPDefines::Plane> m_planes;
		std::vector<BSPDefines::MipTex> m_miptex;
		std::vector<BSPDefines::Vertex> m_vertices;
		std::vector<oxyU8> m_visibility;
		std::vector<BSPDefines::Node> m_nodes;
		std::vector<BSPDefines::TexInfo> m_texinfo;
		std::vector<BSPDefines::Face> m_faces;
		std::vector<BSPDefines::ClipNode> m_clipNodes;
		std::vector<BSPDefines::Leaf> m_leaves;
		std::vector<oxyU16> m_marksurfaces;
		std::vector<BSPDefines::Edge> m_edges;
		std::vector<oxyS32> m_surfedges;
		std::vector<BSPDefines::Model> m_models;

		auto Load(std::string_view mapname) -> oxyBool;
	};

}; // namespace oxygen