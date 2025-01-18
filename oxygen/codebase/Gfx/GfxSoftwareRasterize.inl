namespace oxygen
{
	namespace GfxSoftwareRasterizer
	{
		template <typename T> struct CountingIterator
		{
			using iterator_category = std::random_access_iterator_tag;
			using value_type = T;
			using difference_type = T;
			using pointer = T*;
			using reference = T&;
			const T& operator*() const
			{
				return m_value;
			}
			CountingIterator& operator++()
			{
				++m_value;
				return *this;
			}
			CountingIterator operator++(int)
			{
				auto temp = *this;
				++*this;
				return temp;
			}
			CountingIterator& operator--()
			{
				--m_value;
				return *this;
			}
			CountingIterator operator--(int)
			{
				auto temp = *this;
				--*this;
				return temp;
			}
			CountingIterator& operator+=(const T& lhs)
			{
				m_value += lhs;
				return *this;
			}
			CountingIterator& operator-=(const T& lhs)
			{
				m_value -= lhs;
				return *this;
			}
			CountingIterator operator+(const T& lhs) const
			{
				auto temp = *this;
				temp += lhs;
				return temp;
			}
			CountingIterator operator-(const T& lhs) const
			{
				auto temp = *this;
				temp -= lhs;
				return temp;
			}
			bool operator==(const CountingIterator& lhs) const
			{
				return m_value == lhs.m_value;
			}
			bool operator!=(const CountingIterator& lhs) const
			{
				return m_value != lhs.m_value;
			}
			const T operator+(const CountingIterator& lhs) const
			{
				return m_value + lhs.m_value;
			}
			T operator-(const CountingIterator& lhs) const
			{
				return m_value - lhs.m_value;
			}
			T m_value;
		};

		inline auto RasterTriDepthTest(const GfxTri& tri, oxyS16 triID,
									   oxyU32 width, oxyU32 height,
									   oxyF32* zbuffer, oxyS16* tribuffer, oxyU32 divminx, oxyU32 divminy, oxyU32 divmaxx, oxyU32 divmaxy)
			-> oxyBool
		{
			oxyBool rasteredany{};
			oxyVec2 screenSpaceVerts[3];
			for (auto i = 0; i < 3; ++i)
			{
				const auto& vert = tri.m_vertices[i];
				const auto x = (vert.m_position.x + 1.f) * 0.5f * width;
				const auto y = (1.f - vert.m_position.y) * 0.5f * height;
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
				width - 1);
			auto miny = std::max<oxyS16>(
				std::min<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].y),
								  static_cast<oxyS16>(screenSpaceVerts[1].y),
								  static_cast<oxyS16>(screenSpaceVerts[2].y)}),
				0);
			auto maxy = std::min<oxyS16>(
				std::max<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].y),
								  static_cast<oxyS16>(screenSpaceVerts[1].y),
								  static_cast<oxyS16>(screenSpaceVerts[2].y)}),
				height);

			if ((maxx - minx) <= 0 || (maxy - miny) <= 0)
				return false;

			// Clamp by divide region
			
			if (minx >= static_cast<oxyS16>(divmaxx))
				return false;
			if (miny >= static_cast<oxyS16>(divmaxy))
				return false;
			if (maxx <= static_cast<oxyS16>(divminx))
				return false;
			if (maxy <= static_cast<oxyS16>(divminy))
				return false;
			minx = std::max<oxyS16>(minx, divminx);
			miny = std::max<oxyS16>(miny, divminy);
			maxx = std::min<oxyS16>(maxx, divmaxx);
			maxy = std::min<oxyS16>(maxy, divmaxy);
			

			const auto x10 = screenSpaceVerts[1].x - screenSpaceVerts[0].x;
			const auto x21 = screenSpaceVerts[2].x - screenSpaceVerts[1].x;
			const auto x02 = screenSpaceVerts[0].x - screenSpaceVerts[2].x;
			const auto y10 = screenSpaceVerts[1].y - screenSpaceVerts[0].y;
			const auto y21 = screenSpaceVerts[2].y - screenSpaceVerts[1].y;
			const auto y02 = screenSpaceVerts[0].y - screenSpaceVerts[2].y;

			const auto area = x21 * y02 - x02 * y21;
			const auto invArea = 1.f / area;

			std::for_each(
				std::execution::par_unseq, CountingIterator<int>{miny},
				CountingIterator<int>{maxy}, [&](auto y) {
					for (auto x = minx; x <= maxx; ++x)
					{
						const auto bw0cross =
							x21 * (y - screenSpaceVerts[2].y) -
							(x - screenSpaceVerts[2].x) * y21;
						const auto bw1cross =
							x02 * (y - screenSpaceVerts[0].y) -
							(x - screenSpaceVerts[0].x) * y02;
						const auto bw2cross =
							x10 * (y - screenSpaceVerts[1].y) -
							(x - screenSpaceVerts[1].x) * y10;

						const auto bw0crossbits =
							std::bit_cast<oxyU32>(bw0cross);
						const auto bw1crossbits =
							std::bit_cast<oxyU32>(bw1cross);
						const auto bw2crossbits =
							std::bit_cast<oxyU32>(bw2cross);

						// If all sign bits are equal
						if ((bw0crossbits & 0x80000000) ==
								(bw1crossbits & 0x80000000) &&
							(bw1crossbits & 0x80000000) ==
								(bw2crossbits & 0x80000000))
						{
							const auto w0 = bw0cross * invArea;
							const auto w1 = bw1cross * invArea;
							const auto w2 = bw2cross * invArea;

							const auto z = w0 * tri.m_vertices[0].m_position.z +
										   w1 * tri.m_vertices[1].m_position.z +
										   w2 * tri.m_vertices[2].m_position.z;
							const auto index = y * width + x;
							if (zbuffer[index] > z)
							{
								zbuffer[index] = z;
								tribuffer[index] = triID;
								rasteredany = true;
							}
						}
					}
				});
			return rasteredany;
		}
		inline auto RasterTriNoDepthCompare(const GfxTri& tri, oxyU32 width,
											oxyU32 height, oxyF32* zbuffer,
											oxyU32 divminx, oxyU32 divminy,
											oxyU32 divmaxx, oxyU32 divmaxy)
			-> void
		{
			oxyVec2 screenSpaceVerts[3];
			for (auto i = 0; i < 3; ++i)
			{
				const auto& vert = tri.m_vertices[i];
				const auto x = (vert.m_position.x + 1.f) * 0.5f * width;
				const auto y = (1.f - vert.m_position.y) * 0.5f * height;
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
				width - 1);
			auto miny = std::max<oxyS16>(
				std::min<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].y),
								  static_cast<oxyS16>(screenSpaceVerts[1].y),
								  static_cast<oxyS16>(screenSpaceVerts[2].y)}),
				0);
			auto maxy = std::min<oxyS16>(
				std::max<oxyS16>({static_cast<oxyS16>(screenSpaceVerts[0].y),
								  static_cast<oxyS16>(screenSpaceVerts[1].y),
								  static_cast<oxyS16>(screenSpaceVerts[2].y)}),
				height);

			if ((maxx - minx) <= 0 || (maxy - miny) <= 0)
				return;

			// Clamp by divide region

			if (minx >= static_cast<oxyS16>(divmaxx))
				return;
			if (miny >= static_cast<oxyS16>(divmaxy))
				return;
			if (maxx <= static_cast<oxyS16>(divminx))
				return;
			if (maxy <= static_cast<oxyS16>(divminy))
				return;
			minx = std::max<oxyS16>(minx, divminx);
			miny = std::max<oxyS16>(miny, divminy);
			maxx = std::min<oxyS16>(maxx, divmaxx);
			maxy = std::min<oxyS16>(maxy, divmaxy);

			const auto x10 = screenSpaceVerts[1].x - screenSpaceVerts[0].x;
			const auto x21 = screenSpaceVerts[2].x - screenSpaceVerts[1].x;
			const auto x02 = screenSpaceVerts[0].x - screenSpaceVerts[2].x;
			const auto y10 = screenSpaceVerts[1].y - screenSpaceVerts[0].y;
			const auto y21 = screenSpaceVerts[2].y - screenSpaceVerts[1].y;
			const auto y02 = screenSpaceVerts[0].y - screenSpaceVerts[2].y;

			const auto area = x21 * y02 - x02 * y21;
			const auto invArea = 1.f / area;

			std::for_each(
				std::execution::par_unseq, CountingIterator<int>{miny},
				CountingIterator<int>{maxy}, [&](auto y) {
					for (auto x = minx; x <= maxx; ++x)
					{
						const auto bw0cross =
							x21 * (y - screenSpaceVerts[2].y) -
							(x - screenSpaceVerts[2].x) * y21;
						const auto bw1cross =
							x02 * (y - screenSpaceVerts[0].y) -
							(x - screenSpaceVerts[0].x) * y02;
						const auto bw2cross =
							x10 * (y - screenSpaceVerts[1].y) -
							(x - screenSpaceVerts[1].x) * y10;

						const auto bw0crossbits =
							std::bit_cast<oxyU32>(bw0cross);
						const auto bw1crossbits =
							std::bit_cast<oxyU32>(bw1cross);
						const auto bw2crossbits =
							std::bit_cast<oxyU32>(bw2cross);

						// If all sign bits are equal
						if ((bw0crossbits & 0x80000000) ==
								(bw1crossbits & 0x80000000) &&
							(bw1crossbits & 0x80000000) ==
								(bw2crossbits & 0x80000000))
						{
							const auto w0 = bw0cross * invArea;
							const auto w1 = bw1cross * invArea;
							const auto w2 = bw2cross * invArea;

							const auto z = w0 * tri.m_vertices[0].m_position.z +
										   w1 * tri.m_vertices[1].m_position.z +
										   w2 * tri.m_vertices[2].m_position.z;
							const auto index = y * width + x;
							zbuffer[index] = z;
						}
					}
				});
		}
	}; // namespace GfxSoftwareRasterizer
};	   // namespace oxygen