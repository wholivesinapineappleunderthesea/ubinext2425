#include "OxygenPCH.h"
#include "GfxRenderer.h"
#include "GfxSoftwareRasterize.inl"

#include "GameManager/GameManager.h"
#include "UI/UIManager.h"

#include "Platform/Platform.h"

namespace oxygen
{
	GfxRenderer::GfxRenderer()
	{
		m_errorTexture = LoadTexture(
			std::format("{}/textures/err.png", GetExecutableDirectory()));
		m_whiteSolidTexture = LoadTexture(std::format(
			"{}/textures/solidwhite.png", GetExecutableDirectory()));
		m_fontAtlasTexture = LoadTexture(
			std::format("{}/textures/glyphs.png", GetExecutableDirectory()));
	}
	auto GfxRenderer::LoadTexture(std::string_view texturePath)
		-> std::shared_ptr<const GfxTexture>
	{
		const auto hash = std::hash<std::string_view>{}(texturePath);
		if (const auto it = m_textures.find(hash); it != m_textures.end())
			if (const auto texture = it->second.lock(); texture)
				return texture;

		auto copy = std::string{texturePath};
		auto abstracttex = GraphicsAbstraction::LoadTexture(copy.c_str());
		if (!abstracttex)
			return {};
		auto texture = std::make_shared<GfxTexture>();
		texture->m_width = abstracttex->m_width;
		texture->m_height = abstracttex->m_height;
		texture->m_texturePath = std::move(copy);
		texture->m_texture = std::move(abstracttex);
		m_textures[hash] = texture;
		return texture;
	}

	auto GfxRenderer::OverlayText(std::string_view text, oxyF32 blxndc,
								  oxyF32 blyndc, const oxyVec3& colour,
								  oxyF32 spacing, oxyF32 size,
								  oxyBool center) -> void
	{
		const auto& fontAtlas = m_fontAtlasTexture;

		if (center)
		{
			const auto textwidth = text.size() * spacing;
			blxndc -= textwidth * 0.5f;
		}

		for (auto i = 0; i < text.size(); i++)
		{
			GfxTri a{}, b{};
			const auto ascii = text[i];
			if (ascii < k_fontAtlasBeginASCII || ascii > k_fontAtlasEndASCII)
				continue;
			const auto col =
				(ascii - k_fontAtlasBeginASCII) % k_fontAtlasColumns;
			const auto row =
				(ascii - k_fontAtlasBeginASCII) / k_fontAtlasColumns;
			const auto uvx = col / static_cast<oxyF32>(k_fontAtlasColumns);
			const auto uvy = row / static_cast<oxyF32>(k_fontAtlasRows);
			const auto uvw = 1.f / static_cast<oxyF32>(k_fontAtlasColumns);
			const auto uvh = 1.f / static_cast<oxyF32>(k_fontAtlasRows);
			const auto x0 = blxndc + i * spacing;
			const auto x1 = x0 + size;
			const auto y0 = blyndc;
			const auto y1 = y0 + size;
			a.m_vertices[0].m_position = {x0, y0, 0.f, 1.f};
			a.m_vertices[1].m_position = {x1, y0, 0.f, 1.f};
			a.m_vertices[2].m_position = {x0, y1, 0.f, 1.f};
			a.m_vertices[0].m_uv = {uvx, uvy};
			a.m_vertices[1].m_uv = {uvx + uvw, uvy};
			a.m_vertices[2].m_uv = {uvx, uvy + uvh};
			a.m_colour = colour;
			a.m_texture = fontAtlas.get();
			b.m_vertices[0].m_position = {x1, y0, 0.f, 1.f};
			b.m_vertices[1].m_position = {x1, y1, 0.f, 1.f};
			b.m_vertices[2].m_position = {x0, y1, 0.f, 1.f};
			b.m_vertices[0].m_uv = {uvx + uvw, uvy};
			b.m_vertices[1].m_uv = {uvx + uvw, uvy + uvh};
			b.m_vertices[2].m_uv = {uvx, uvy + uvh};
			b.m_colour = colour;
			b.m_texture = fontAtlas.get();

			SubmitTriToQueue(a, GfxRenderStrategy_DirectToGPU);
			SubmitTriToQueue(b, GfxRenderStrategy_DirectToGPU);
		}
	}

	auto GfxRenderer::OverlayRect(const oxyVec3& col, const oxyVec2& minndc,
								  const oxyVec2& maxndc) -> void
	{
		GfxTri a{}, b{};
		a.m_vertices[0].m_position = {minndc.x, minndc.y, 0.f, 1.f};
		a.m_vertices[1].m_position = {maxndc.x, minndc.y, 0.f, 1.f};
		a.m_vertices[2].m_position = {minndc.x, maxndc.y, 0.f, 1.f};
		a.m_colour = col;
		a.m_texture = m_whiteSolidTexture.get();
		b.m_vertices[0].m_position = {maxndc.x, minndc.y, 0.f, 1.f};
		b.m_vertices[1].m_position = {maxndc.x, maxndc.y, 0.f, 1.f};
		b.m_vertices[2].m_position = {minndc.x, maxndc.y, 0.f, 1.f};
		b.m_colour = col;
		b.m_texture = m_whiteSolidTexture.get();
		SubmitTriToQueue(a, GfxRenderStrategy_DirectToGPU);
		SubmitTriToQueue(b, GfxRenderStrategy_DirectToGPU);
	}

	auto GfxRenderer::EndFrame() -> void
	{
		oxySize numtrisortedraster{};
		oxySize numtrirastered{};
		oxyU32 rasterWidth = m_softwareWidth;
		oxyU32 rasterHeight = m_softwareHeight;
		std::future<void> rasterFuture = std::async([&]() {
			std::vector<BBox> unsortedBBoxes;
			unsortedBBoxes.reserve(m_triQueueSoftwareDepthRasterize.size());
			for (const auto& tri : m_triQueueSoftwareDepthRasterize)
			{
				const auto tribbox = NDCTriToBBox(tri);
				oxyBool inserted = false;
				for (auto& bbox : unsortedBBoxes)
				{
					if (bbox.Expand(tribbox))
					{
						inserted = true;
						break;
					}
				}
				if (!inserted)
					unsortedBBoxes.push_back(tribbox);
			}
		
			// TODO: par_unseq for each bbox, raster divide by bbox (will need
			// to switch to ndc division/bbox)
		
			// TODO: bsp culling of dynamic meshes
			#if 1
			for (oxySize i = 0;
				 i < m_triQueueSoftwareDepthRasterizePreSorted.size(); ++i)
			{
				const auto bbox =
					NDCTriToBBox(m_triQueueSoftwareDepthRasterizePreSorted[i]);
				const auto minDepth =
					std::min({m_triQueueSoftwareDepthRasterizePreSorted[i]
								  .m_vertices[0]
								  .m_position.z,
							  m_triQueueSoftwareDepthRasterizePreSorted[i]
								  .m_vertices[1]
								  .m_position.z,
							  m_triQueueSoftwareDepthRasterizePreSorted[i]
								  .m_vertices[2]
								  .m_position.z});
				for (oxySize j = 0; j < unsortedBBoxes.size(); ++j)
				{
					// If the triangle overlaps the bbox and the min depth
					// is less than the max depth of the bbox
					if (unsortedBBoxes[j].Overlaps(bbox) &&
						minDepth < unsortedBBoxes[j].m_maxDepth)
					{
						GfxSoftwareRasterizer::RasterTriNoDepthCompare(
							m_triQueueSoftwareDepthRasterizePreSorted[i],
							rasterWidth, rasterHeight, m_zbuffer.get(), 0, 0,
							rasterWidth - 1, rasterHeight);
						numtrisortedraster++;
						break;
					}
				}
			}
		
			for (oxyS16 dynamicTriID = -1;
				 const auto& tri : m_triQueueSoftwareDepthRasterize)
			{
				numtrirastered += GfxSoftwareRasterizer::RasterTriDepthTest(
					tri, ++dynamicTriID, rasterWidth, rasterHeight,
					m_zbuffer.get(), m_tribuffer.get(), 0, 0, rasterWidth - 1,
					rasterHeight);
			}
			#endif
		});

		if (m_triQueueSoftwareDepthRasterizePreSortedOverlay.size())
		{
			const auto cnt =
				m_triQueueSoftwareDepthRasterizePreSortedOverlay.size();
			OXYCHECK(cnt == m_triQueueSoftwareDepthRasterizePreSorted.size());
			for (oxySize i = 0; i < cnt; ++i)
			{
				DrawPreSortedTri(m_triQueueSoftwareDepthRasterizePreSorted[i]);
				DrawPreSortedTri(
					m_triQueueSoftwareDepthRasterizePreSortedOverlay[i]);
			}
		}
		else
		{
			for (const auto& tri : m_triQueueSoftwareDepthRasterizePreSorted)
				DrawPreSortedTri(tri);
		}

		

		rasterFuture.wait();

		if (numtrirastered)
			DrawSpans(rasterWidth, rasterHeight);
		#if 0
		const auto numtrisortrasttxt =
			std::format("Num tris sorted raster: {}", numtrisortedraster);
		const auto numtrirasttxt =
			std::format("Num tris rastered: {}", numtrirastered);
		const auto numsortedtxt =
			std::format("Num sorted tris: {}",
						m_triQueueSoftwareDepthRasterizePreSorted.size());
		const auto numunsortedtxt = std::format(
			"Num unsorted tris: {}", m_triQueueSoftwareDepthRasterize.size());
		OverlayText(numtrisortrasttxt, 0.f, .9f, {1.f, 1.f, 1.f}, 0.025f,
					0.05f, true);
		OverlayText(numtrirasttxt, 0.f, .85f, {1.f, 1.f, 1.f}, 0.025f, 0.05f, true);
		OverlayText(numsortedtxt, 0.f, .8f, {1.f, 1.f, 1.f}, 0.025f, 0.05f, true);
		OverlayText(numunsortedtxt, 0.f, .75f, {1.f, 1.f, 1.f}, 0.025f, 0.05f, true);

		// this kinda is a lie...
		const auto numoverlaytxt =
			std::format("Overlay: {}", m_triQueueDirectToGPU.size());
		OverlayText(numoverlaytxt, 0.f, .7f, {1.f, 1.f, 1.f}, 0.025f, 0.05f,
					true);
		#endif

		for (auto& tri : m_triQueueDirectToGPU)
		{
			GraphicsAbstraction::TexturedQuad quad;
			quad.m_vertices[0] = tri.m_vertices[0].m_position;
			quad.m_vertices[1] = tri.m_vertices[1].m_position;
			quad.m_vertices[2] = tri.m_vertices[2].m_position;
			quad.m_vertices[3] = tri.m_vertices[2].m_position;
			quad.m_textureCoords[0] = tri.m_vertices[0].m_uv;
			quad.m_textureCoords[1] = tri.m_vertices[1].m_uv;
			quad.m_textureCoords[2] = tri.m_vertices[2].m_uv;
			quad.m_textureCoords[3] = tri.m_vertices[2].m_uv;
			quad.m_colour = tri.m_colour;
			auto tex = tri.m_texture;
			if (!tex)
				tex = m_errorTexture.get();
			quad.m_texture = tex->m_texture.get();
			GraphicsAbstraction::DrawTexturedQuad(quad);
		}
	}

	auto GfxRenderer::SubmitTriToQueue(const GfxTri& tri,
									   GfxRenderStrategy mode,
									   oxyF32 zmult) -> void
	{
		if (CullClipSpaceTri(tri))
			return;

		if (mode == GfxRenderStrategy::GfxRenderStrategy_DirectToGPU)
		{
			m_triQueueDirectToGPU.push_back(tri);
		}
		else if (mode == GfxRenderStrategy::
							 GfxRenderStrategy_SoftwareDepthRasterizePreSorted)
		{
			std::underlying_type_t<ClipCode> clipOr = ClipCode_None;
			std::underlying_type_t<ClipCode> clipAnd = ClipCode_None;
			for (const auto& vtx : tri.m_vertices)
			{
				std::underlying_type_t<ClipCode> clip = ClipCode_None;
				if (vtx.m_position.z < -vtx.m_position.w)
					clip |= ClipCode_Near;
				if (vtx.m_position.z > vtx.m_position.w)
					clip |= ClipCode_Far;
				clipOr |= clip;
				clipAnd &= clip;
			}
			const auto SubmitSortedTri = [&](GfxTri tri) {
				if (ConvertTriToNDCAndCull(tri))
					return;
				tri.m_vertices[0].m_position.z *= zmult;
				tri.m_vertices[1].m_position.z *= zmult;
				tri.m_vertices[2].m_position.z *= zmult;
				m_triQueueSoftwareDepthRasterizePreSorted.push_back(tri);
			};
			if (clipAnd == ClipCode_None)
			{
				if (clipOr == ClipCode_None)
					SubmitSortedTri(tri);
				else
					ClipTri(tri, static_cast<ClipCode>(clipOr),
							SubmitSortedTri);
			}
		}
		else if (mode == GfxRenderStrategy_SoftwareDepthRasterizePreSortedOverlay)
		{
			std::underlying_type_t<ClipCode> clipOr = ClipCode_None;
			std::underlying_type_t<ClipCode> clipAnd = ClipCode_None;
			for (const auto& vtx : tri.m_vertices)
			{
				std::underlying_type_t<ClipCode> clip = ClipCode_None;
				if (vtx.m_position.z < -vtx.m_position.w)
					clip |= ClipCode_Near;
				if (vtx.m_position.z > vtx.m_position.w)
					clip |= ClipCode_Far;
				clipOr |= clip;
				clipAnd &= clip;
			}
			const auto SubmitSortedTri = [&](GfxTri tri) {
				if (ConvertTriToNDCAndCull(tri))
					return;
				tri.m_vertices[0].m_position.z *= zmult;
				tri.m_vertices[1].m_position.z *= zmult;
				tri.m_vertices[2].m_position.z *= zmult;
				m_triQueueSoftwareDepthRasterizePreSortedOverlay.push_back(tri);
			};
			if (clipAnd == ClipCode_None)
			{
				if (clipOr == ClipCode_None)
					SubmitSortedTri(tri);
				else
					ClipTri(tri, static_cast<ClipCode>(clipOr),
							SubmitSortedTri);
			}
		}
		else if (mode ==
				 GfxRenderStrategy::GfxRenderStrategy_SoftwareDepthRasterize)
		{
			std::underlying_type_t<ClipCode> clipOr = ClipCode_None;
			std::underlying_type_t<ClipCode> clipAnd = ClipCode_None;
			for (const auto& vtx : tri.m_vertices)
			{
				std::underlying_type_t<ClipCode> clip = ClipCode_None;
				if (vtx.m_position.z < -vtx.m_position.w)
					clip |= ClipCode_Near;
				if (vtx.m_position.z > vtx.m_position.w)
					clip |= ClipCode_Far;
				clipOr |= clip;
				clipAnd &= clip;
			}
			const auto SubmitTri = [&](GfxTri tri) {
				if (ConvertTriToNDCAndCull(tri))
					return;
				tri.m_vertices[0].m_position.z *= zmult;
				tri.m_vertices[1].m_position.z *= zmult;
				tri.m_vertices[2].m_position.z *= zmult;
				m_triQueueSoftwareDepthRasterize.push_back(tri);
			};
			if (clipAnd == ClipCode_None)
			{
				if (clipOr == ClipCode_None)
					SubmitTri(tri);
				else
					ClipTri(tri, static_cast<ClipCode>(clipOr), SubmitTri);
			}
		}
	}

	auto GfxRenderer::BeginFrame(oxyS32 w, oxyS32 h) -> void
	{
		if (w < 0 || h < 0)
			return;
		if (w != m_width || h != m_height)
			HandleResize(w, h);

		m_frameCounter++;
		m_triQueueSoftwareDepthRasterizePreSorted.clear();
		m_triQueueSoftwareDepthRasterizePreSortedOverlay.clear();
		m_triQueueSoftwareDepthRasterize.clear();
		m_triQueueDirectToGPU.clear();
		std::fill_n(m_zbuffer.get(), m_softwareWidth * m_softwareHeight, 1.0f);
		std::fill_n(m_tribuffer.get(), m_softwareWidth * m_softwareHeight, -1);

		GameManager::GetInstance().Render();
		UIManager::GetInstance().Render();
	}

	//
	// TODO:
	// Potential optimization:
	// There's CPU downtime between Update and Render,
	// Once Update is done, another thread could be started to iterate tris
	// and raster depth and such. Then Render would just wait for that to be
	// done (if it isn't already) and then draw the spans.
	//

	template <typename Fun>
	auto GfxRenderer::ClipTri(const GfxTri& tri, ClipCode clipcode,
							  Fun&& cb) -> void
	{
		GfxVertex inVerts[12];
		GfxVertex outVerts[12];
		size_t inCount{};
		size_t outCount{};
		for (size_t i{}; i < std::size(tri.m_vertices); i++)
			inVerts[inCount++] = tri.m_vertices[i];

		auto clipint = std::underlying_type_t<ClipCode>{clipcode};

		constexpr oxyVec4 clipPlanes[] = {
			{0.0f, 0.0f, 1.0f, 1.0f},  // Near
			{0.0f, 0.0f, -1.0f, 1.0f}, // Far
			{0.0f, 1.0f, 0.0f, 1.0f},  // Top
			{0.0f, -1.0f, 0.0f, 1.0f}, // Bottom
			{1.0f, 0.0f, 0.0f, 1.0f},  // Right
			{-1.0f, 0.0f, 0.0f, 1.0f}  // Left
		};

		unsigned long planeIdx;
		while (_BitScanForward(&planeIdx, clipint))
		{
			clipint ^= 1U << planeIdx;
			const auto& planeEq = clipPlanes[planeIdx];
			for (size_t i{}; i < inCount; i++)
			{
				const auto& current = inVerts[i];
				const auto& next = inVerts[i == (inCount - 1) ? 0 : i + 1];
				const auto d1 = planeEq.DotProduct(current.m_position);
				const auto d2 = planeEq.DotProduct(next.m_position);
				const auto currIn = d1 >= 0.0f;
				const auto nextIn = d2 >= 0.0f;
				if (currIn)
					outVerts[outCount++] = current;
				if (currIn ^ nextIn)
				{
					const auto lerpVertices = [](const GfxVertex& a,
												 const GfxVertex& b, float t) {
						GfxVertex v;
						v.m_position =
							a.m_position +
							oxyVec4{t, t, t, t} * (b.m_position - a.m_position);
						v.m_uv = a.m_uv + oxyVec2{t, t} * (b.m_uv - a.m_uv);
						return v;
					};
					if (currIn)
						outVerts[outCount++] =
							lerpVertices(current, next, d1 / (d1 - d2));
					else
						outVerts[outCount++] =
							lerpVertices(next, current, d2 / (d2 - d1));
				}
			}
			if (outCount < 3)
				return;

			inCount = 0;
			for (size_t i{}; i < outCount; i++)
				inVerts[inCount++] = outVerts[i];
			outCount = 0;
		}
		for (size_t i = 1; i < inCount - 1; i++)
		{
			GfxTri newtri;
			newtri.m_vertices[0] = inVerts[0];
			newtri.m_vertices[1] = inVerts[i];
			newtri.m_vertices[2] = inVerts[i + 1];
			newtri.m_colour = tri.m_colour;
			newtri.m_texture = tri.m_texture;
			newtri.m_cullType = tri.m_cullType;
			cb(newtri);
		}
	}

	auto GfxRenderer::CullClipSpaceTri(const GfxTri& tri) -> bool
	{
		// checks if the triangle is completely outside the view frustum
		// if it is, it should be culled

		// Near plane
		if (tri.m_vertices[0].m_position.z < -tri.m_vertices[0].m_position.w &&
			tri.m_vertices[1].m_position.z < -tri.m_vertices[1].m_position.w &&
			tri.m_vertices[2].m_position.z < -tri.m_vertices[2].m_position.w)
			return true;
		// Far plane
		if (tri.m_vertices[0].m_position.z > tri.m_vertices[0].m_position.w &&
			tri.m_vertices[1].m_position.z > tri.m_vertices[1].m_position.w &&
			tri.m_vertices[2].m_position.z > tri.m_vertices[2].m_position.w)
			return true;
		// Top plane
		if (tri.m_vertices[0].m_position.y > tri.m_vertices[0].m_position.w &&
			tri.m_vertices[1].m_position.y > tri.m_vertices[1].m_position.w &&
			tri.m_vertices[2].m_position.y > tri.m_vertices[2].m_position.w)
			return true;
		// Bottom plane
		if (tri.m_vertices[0].m_position.y < -tri.m_vertices[0].m_position.w &&
			tri.m_vertices[1].m_position.y < -tri.m_vertices[1].m_position.w &&
			tri.m_vertices[2].m_position.y < -tri.m_vertices[2].m_position.w)
			return true;
		// Right plane
		if (tri.m_vertices[0].m_position.x > tri.m_vertices[0].m_position.w &&
			tri.m_vertices[1].m_position.x > tri.m_vertices[1].m_position.w &&
			tri.m_vertices[2].m_position.x > tri.m_vertices[2].m_position.w)
			return true;
		// Left plane
		if (tri.m_vertices[0].m_position.x < -tri.m_vertices[0].m_position.w &&
			tri.m_vertices[1].m_position.x < -tri.m_vertices[1].m_position.w &&
			tri.m_vertices[2].m_position.x < -tri.m_vertices[2].m_position.w)
			return true;

		return false;
	}

	auto GfxRenderer::ConvertTriToNDCAndCull(GfxTri& tri) -> bool
	{
		tri.m_vertices[0].m_position.x /= tri.m_vertices[0].m_position.w;
		tri.m_vertices[0].m_position.y /= tri.m_vertices[0].m_position.w;
		tri.m_vertices[0].m_position.z /= tri.m_vertices[0].m_position.w;
		tri.m_vertices[0].m_position.w = tri.m_vertices[0].m_position.w;
		tri.m_vertices[1].m_position.x /= tri.m_vertices[1].m_position.w;
		tri.m_vertices[1].m_position.y /= tri.m_vertices[1].m_position.w;
		tri.m_vertices[1].m_position.z /= tri.m_vertices[1].m_position.w;
		tri.m_vertices[1].m_position.w = tri.m_vertices[1].m_position.w;
		tri.m_vertices[2].m_position.x /= tri.m_vertices[2].m_position.w;
		tri.m_vertices[2].m_position.y /= tri.m_vertices[2].m_position.w;
		tri.m_vertices[2].m_position.z /= tri.m_vertices[2].m_position.w;
		tri.m_vertices[2].m_position.w = tri.m_vertices[2].m_position.w;

		if (tri.m_cullType == GfxCullType::GfxCullType_Backface)
			if (CullBackfaceTri(tri))
				return true;

		if (tri.m_cullType == GfxCullType::GfxCullType_Frontface)
			if (CullFrontfaceTri(tri))
				return true;

		return false;
	}

	auto GfxRenderer::DrawPreSortedTri(const GfxTri& tri) -> void
	{
		GraphicsAbstraction::TexturedQuad quad;
		quad.m_vertices[0] = {-1.f * tri.m_vertices[0].m_position.x,
							  tri.m_vertices[0].m_position.y};
		quad.m_vertices[1] = {-1.f * tri.m_vertices[1].m_position.x,
							  tri.m_vertices[1].m_position.y};
		quad.m_vertices[2] = {-1.f * tri.m_vertices[2].m_position.x,
							  tri.m_vertices[2].m_position.y};
		quad.m_vertices[3] = {-1.f * tri.m_vertices[2].m_position.x,
							  tri.m_vertices[2].m_position.y};
		quad.m_textureCoords[0] = tri.m_vertices[0].m_uv;
		quad.m_textureCoords[1] = tri.m_vertices[1].m_uv;
		quad.m_textureCoords[2] = tri.m_vertices[2].m_uv;
		quad.m_textureCoords[3] = tri.m_vertices[2].m_uv;
		quad.m_colour = tri.m_colour;
		auto tex = tri.m_texture;
		if (!tex)
			tex = m_errorTexture.get();
		quad.m_texture = tex->m_texture.get();
		GraphicsAbstraction::DrawTexturedQuad(quad);
	}

	auto GfxRenderer::DrawSpans(oxyU16 width, oxyU16 height) -> void
	{
		for (oxyU16 y = 0; y < height; ++y)
		{
			oxyU16 x0 = -1;
			oxyU16 x1 = -1;
			oxyS16 triID = -1;
			for (oxyU16 x = 0; x < width; ++x)
			{
				const auto curID = m_tribuffer[y * width + x];
				if (curID != triID)
				{
					if (triID != -1 && x1 > x0)
					{
						// Draw span y x0 x1
						DrawSpan(*GetTriFromID(triID), y, x0, x1, width,
								 height);
					}
					x0 = x;
					x1 = x;
					triID = curID;
				}
				else
				{
					x1 = x;
				}
			}
			if (triID != -1 && x1 > x0)
			{
				// Draw span y x0 x1
				DrawSpan(*GetTriFromID(triID), y, x0, x1, width, height);
			}
		}
	}

	auto GfxRenderer::GetTriFromID(oxyS16 id) -> const GfxTri*
	{
		return &m_triQueueSoftwareDepthRasterize[id];
	}

	auto GfxRenderer::DrawSpan(const GfxTri& tri, oxyS32 y, oxyS32 x0,
							   oxyS32 x1, oxyU16 fbwidth,
							   oxyU16 fbheight) -> void
	{
		const auto& v0 = tri.m_vertices[0];
		const auto& v1 = tri.m_vertices[1];
		const auto& v2 = tri.m_vertices[2];

		// Barycentric coordinates
		const auto x10 = v1.m_position.x - v0.m_position.x;
		const auto x21 = v2.m_position.x - v1.m_position.x;
		const auto x02 = v0.m_position.x - v2.m_position.x;
		const auto y10 = v1.m_position.y - v0.m_position.y;
		const auto y21 = v2.m_position.y - v1.m_position.y;
		const auto y02 = v0.m_position.y - v2.m_position.y;

		const auto area = x21 * y02 - x02 * y21;
		const auto invArea = 1.f / area;

		const auto CalcBary = [&](oxyF32 px,
								  oxyF32 py) -> std::array<oxyF32, 3> {
			const auto bw0cross =
				x21 * (py - v2.m_position.y) - (px - v2.m_position.x) * y21;
			const auto bw1cross =
				x02 * (py - v0.m_position.y) - (px - v0.m_position.x) * y02;
			const auto bw2cross =
				x10 * (py - v1.m_position.y) - (px - v1.m_position.x) * y10;

			const auto w0 = bw0cross * invArea;
			const auto w1 = bw1cross * invArea;
			const auto w2 = bw2cross * invArea;
			return {w0, w1, w2};
		};

		// Top left NDC:
		const auto x0ndc = (2.0f * (x0 - 1) / fbwidth) - 1.0f;
		const auto y0ndc = 1.0f - (2.0f * (y - 1) / fbheight);
		// Bottom right NDC:
		const auto x1ndc = (2.0f * (x1 + 1) / fbwidth) - 1.0f;
		const auto y1ndc = 1.0f - (2.0f * (y + 1) / fbheight);
		// Quad vertices:
		const auto quadx0 = x0ndc; // top left
		const auto quady0 = y0ndc;
		const auto quadx1 = x1ndc; // top right
		const auto quady1 = y0ndc;
		const auto quadx2 = x1ndc; // bottom right
		const auto quady2 = y1ndc;
		const auto quadx3 = x0ndc; // bottom left
		const auto quady3 = y1ndc;
		// Quad barycentric coordinates
		const auto bary0 = CalcBary(quadx0, quady0);
		const auto bary1 = CalcBary(quadx1, quady1);
		const auto bary2 = CalcBary(quadx2, quady2);
		const auto bary3 = CalcBary(quadx3, quady3);
		GraphicsAbstraction::TexturedQuad quad;
		quad.m_vertices[0] = {-1.f * quadx0, quady0};
		quad.m_vertices[1] = {-1.f * quadx1, quady1};
		quad.m_vertices[2] = {-1.f * quadx2, quady2};
		quad.m_vertices[3] = {-1.f * quadx3, quady3};
		quad.m_textureCoords[0] = {
			bary0[0] * v0.m_uv.x + bary0[1] * v1.m_uv.x + bary0[2] * v2.m_uv.x,
			bary0[0] * v0.m_uv.y + bary0[1] * v1.m_uv.y + bary0[2] * v2.m_uv.y};
		quad.m_textureCoords[1] = {
			bary1[0] * v0.m_uv.x + bary1[1] * v1.m_uv.x + bary1[2] * v2.m_uv.x,
			bary1[0] * v0.m_uv.y + bary1[1] * v1.m_uv.y + bary1[2] * v2.m_uv.y};
		quad.m_textureCoords[2] = {
			bary2[0] * v0.m_uv.x + bary2[1] * v1.m_uv.x + bary2[2] * v2.m_uv.x,
			bary2[0] * v0.m_uv.y + bary2[1] * v1.m_uv.y + bary2[2] * v2.m_uv.y};
		quad.m_textureCoords[3] = {
			bary3[0] * v0.m_uv.x + bary3[1] * v1.m_uv.x + bary3[2] * v2.m_uv.x,
			bary3[0] * v0.m_uv.y + bary3[1] * v1.m_uv.y + bary3[2] * v2.m_uv.y};
		quad.m_colour = tri.m_colour;
		auto tex = tri.m_texture;
		if (!tex)
			tex = m_errorTexture.get();
		quad.m_texture = tex->m_texture.get();
		GraphicsAbstraction::DrawTexturedQuad(quad);
	}

	auto GfxRenderer::HandleResize(oxyS32 w, oxyS32 h) -> void
	{
		m_width = w;
		m_height = h;
#ifdef OXYBUILDDEBUG
		m_softwareWidth = 400;
		m_softwareHeight = 300;
		#else
		m_softwareWidth = 1000;
		m_softwareHeight = 750;
		#endif
		m_zbuffer =
			std::make_unique<oxyF32[]>(m_softwareWidth * m_softwareHeight);
		m_tribuffer =
			std::make_unique<oxyS16[]>(m_softwareWidth * m_softwareHeight);
	}

	auto GfxRenderer::NDCTriToBBox(const GfxTri& tri) -> BBox
	{
		BBox ret;
		oxyVec2 screenSpaceVerts[3];
		for (auto i = 0; i < 3; ++i)
		{
			const auto& vert = tri.m_vertices[i];
			const auto x = (vert.m_position.x + 1.f) * 0.5f * m_softwareWidth;
			const auto y = (1.f - vert.m_position.y) * 0.5f * m_softwareHeight;
			screenSpaceVerts[i] = {std::ceilf(x), std::ceilf(y)};
		}

		auto minx = std::max<oxyS16>(
			std::min<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].x),
							  static_cast<oxyS16>(screenSpaceVerts[1].x),
							  static_cast<oxyS16>(screenSpaceVerts[2].x)}),
			0);
		auto maxx = std::min<oxyS16>(
			std::max<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].x),
							  static_cast<oxyS16>(screenSpaceVerts[1].x),
							  static_cast<oxyS16>(screenSpaceVerts[2].x)}),
			m_softwareWidth - 1);
		auto miny = std::max<oxyS16>(
			std::min<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].y),
							  static_cast<oxyS16>(screenSpaceVerts[1].y),
							  static_cast<oxyS16>(screenSpaceVerts[2].y)}),
			0);
		auto maxy = std::min<oxyS16>(
			std::max<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].y),
							  static_cast<oxyS16>(screenSpaceVerts[1].y),
							  static_cast<oxyS16>(screenSpaceVerts[2].y)}),
			m_softwareHeight);

		ret.m_x0 = minx;
		ret.m_y0 = miny;
		ret.m_x1 = maxx;
		ret.m_y1 = maxy;
		ret.m_maxDepth = std::max({tri.m_vertices[0].m_position.z,
								   tri.m_vertices[1].m_position.z,
								   tri.m_vertices[2].m_position.z});

		return ret;
	}

	auto GfxRenderer::BBox::Overlaps(const BBox& other) const -> oxyBool
	{
		return m_x0 <= other.m_x1 && m_x1 >= other.m_x0 && m_y0 <= other.m_y1 &&
			   m_y1 >= other.m_y0;
	}

	auto GfxRenderer::BBox::Expand(const BBox& other) -> oxyBool
	{
		if (!Overlaps(other))
			return false;
		m_x0 = std::min(m_x0, other.m_x0);
		m_y0 = std::min(m_y0, other.m_y0);
		m_x1 = std::max(m_x1, other.m_x1);
		m_y1 = std::max(m_y1, other.m_y1);
		m_maxDepth = std::max(m_maxDepth, other.m_maxDepth);
		return true;
	}

} // namespace oxygen
