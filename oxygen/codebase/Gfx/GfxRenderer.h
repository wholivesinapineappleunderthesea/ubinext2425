#pragma once

#include "Singleton/Singleton.h"

namespace oxygen
{
	namespace GraphicsAbstraction
	{
		struct Texture;
	}; // namespace GraphicsAbstraction

	struct GfxTexture
	{
		oxyU32 m_width;
		oxyU32 m_height;
		std::string m_texturePath;
		std::shared_ptr<const GraphicsAbstraction::Texture> m_texture;
	};

	struct GfxVertex
	{
		oxyVec4 m_position;
		oxyVec2 m_uv;
	};
	enum GfxRenderStrategy : oxyU8
	{
		// Submit straight to the GPU, 2D only, no depth writes or tests, no
		// clipping, drawn after all 3d geometry, in the order submitted
		// Usage: 2D sprites, UI elements
		GfxRenderStrategy_DirectToGPU = 0,
		// Submit to GPU, write depth buffer w/o test,
		// clips against all six planes
		// Usage: BSP sorted 3D geometry
		GfxRenderStrategy_SoftwareDepthRasterizePreSorted,

		// Submit to GPU immediately after paired PreSorted
		// clips against all six planes
		// Usage: BSP sorted 3D geometry
		GfxRenderStrategy_SoftwareDepthRasterizePreSortedOverlay,

		// Write depth buffer w/ test,
		// submit to gpu in spans at the end of the frame,
		// clips against near and far planes
		// Usage: dynamic 3D geometry
		GfxRenderStrategy_SoftwareDepthRasterize,
	};
	enum GfxCullType
	{
		GfxCullType_None = 0,
		GfxCullType_Backface = 1,
		GfxCullType_Frontface = 2,
	};
	struct GfxTri
	{
		GfxVertex m_vertices[3];
		oxyVec3 m_colour;
		const GfxTexture* m_texture{};
		GfxCullType m_cullType;
	};

	inline auto CullBackfaceTri(const GfxTri& tri) -> bool
	{
		return (tri.m_vertices[1].m_position - tri.m_vertices[0].m_position)
				   .CrossProduct(tri.m_vertices[2].m_position -
								 tri.m_vertices[0].m_position)
				   .z < 0;
	}
	inline auto CullFrontfaceTri(const GfxTri& tri) -> bool
	{
		return (tri.m_vertices[1].m_position - tri.m_vertices[0].m_position)
				   .CrossProduct(tri.m_vertices[2].m_position -
								 tri.m_vertices[0].m_position)
				   .z > 0;
	}

	struct GfxRenderer : SingletonBase<GfxRenderer>
	{
		GfxRenderer();
		auto SetViewProjectionMatrix(const oxyMat4x4& viewProjectionMatrix)
			-> void
		{
			m_viewProjectionMatrix = viewProjectionMatrix;
		}
		auto GetViewProjectionMatrix() const -> const oxyMat4x4&
		{
			return m_viewProjectionMatrix;
		}

		auto GetWidth() const -> oxyS32
		{
			return m_width;
		}
		auto GetHeight() const -> oxyS32
		{
			return m_height;
		}

		auto LoadTexture(std::string_view texturePath)
			-> std::shared_ptr<const GfxTexture>;

		auto OverlayText(std::string_view text, oxyF32 blxndc, oxyF32 blyndc,
						 const oxyVec3& colour, oxyF32 spacing, oxyF32 size, oxyBool center) -> void;
		auto OverlayRect(const oxyVec3& col, const oxyVec2& minndc,
						 const oxyVec2& maxndc) -> void;

		auto BeginFrame(oxyS32 w, oxyS32 h) -> void;
		auto EndFrame() -> void;

		auto SubmitTriToQueue(const GfxTri& tri, GfxRenderStrategy mode, oxyF32 zmult = 1.0f)
			-> void;
	  private:
		enum ClipCode
		{
			ClipCode_None = 0,
			ClipCode_Near = 1,
			ClipCode_Far = 2,
			ClipCode_Left = 4,
			ClipCode_Right = 8,
			ClipCode_Top = 16,
			ClipCode_Bottom = 32,
		};
		template <typename Fun>
		static auto ClipTri(const GfxTri& tri, ClipCode clipcode, Fun&& cb)
			-> void;

		auto CullClipSpaceTri(const GfxTri& tri) -> bool;

		auto ConvertTriToNDCAndCull(GfxTri& tri) -> bool;

		auto DrawPreSortedTri(const GfxTri& tri) -> void;

		auto DrawSpans(oxyU16 width, oxyU16 height) -> void;
		auto GetTriFromID(oxyS16 id) -> const GfxTri*;
		auto DrawSpan(const GfxTri& tri, oxyS32 y, oxyS32 x0, oxyS32 x1, oxyU16 fbwidth, oxyU16 fbheight)
			-> void;

		auto HandleResize(oxyS32 w, oxyS32 h) -> void;
		oxyS32 m_width;
		oxyS32 m_height;

		oxyS32 m_softwareWidth;
		oxyS32 m_softwareHeight;

		oxyU64 m_frameCounter{};

		std::unique_ptr<oxyF32[]> m_zbuffer;
		std::unique_ptr<oxyS16[]> m_tribuffer;

		oxyMat4x4 m_viewProjectionMatrix;

		std::unordered_map<std::size_t, std::weak_ptr<const GfxTexture>>
			m_textures;

		std::shared_ptr<const GfxTexture> m_errorTexture;
		std::shared_ptr<const GfxTexture> m_whiteSolidTexture;
		std::shared_ptr<const GfxTexture> m_fontAtlasTexture;
		static inline constexpr auto k_fontAtlasBeginASCII = 32;
		static inline constexpr auto k_fontAtlasEndASCII = 127;
		static inline constexpr auto k_fontAtlasColumns = 16;
		static inline constexpr auto k_fontAtlasRows = 6;


		std::vector<GfxTri> m_triQueueSoftwareDepthRasterizePreSorted;
		std::vector<GfxTri> m_triQueueSoftwareDepthRasterizePreSortedOverlay;
		std::vector<GfxTri> m_triQueueSoftwareDepthRasterize;
		std::vector<GfxTri> m_triQueueDirectToGPU;

		struct BBox
		{
			oxyS32 m_x0;
			oxyS32 m_y0;
			oxyS32 m_x1;
			oxyS32 m_y1;
			oxyF32 m_maxDepth;

			auto Overlaps(const BBox& other) const -> oxyBool;
			auto Expand(const BBox& other) -> oxyBool;
		};

		auto NDCTriToBBox(const GfxTri& tri) -> BBox;
	};
}; // namespace oxygen