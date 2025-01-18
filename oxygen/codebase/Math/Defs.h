#pragma once

struct oxyVec2;
struct oxyVec3;
struct oxyVec4;
struct oxyQuat;
struct oxyMat4x4;

namespace oxygen
{
	namespace Math
	{
		static inline constexpr auto k_pi = 3.14159265358979323846f;
		static inline constexpr auto k_twoPi = 2.0f * k_pi;
		static inline constexpr auto k_halfPi = 0.5f * k_pi;

		static inline constexpr auto k_radToDeg = 180.0f / k_pi;
		static inline constexpr auto k_degToRad = k_pi / 180.0f;
	}; // namespace Math
}; // namespace oxygen

inline constexpr auto operator-(const oxyVec2& a) -> oxyVec2;
inline constexpr auto operator+(const oxyVec2& a, const oxyVec2& b) -> oxyVec2;
inline constexpr auto operator-(const oxyVec2& a, const oxyVec2& b) -> oxyVec2;
inline constexpr auto operator*(const oxyVec2& a, const oxyVec2& b) -> oxyVec2;
inline constexpr auto operator/(const oxyVec2& a, const oxyVec2& b) -> oxyVec2;
inline constexpr auto operator*(const oxyVec2& a, oxyF32 b) -> oxyVec2;
inline constexpr auto operator/(const oxyVec2& a, oxyF32 b) -> oxyVec2;

inline constexpr auto operator-(const oxyVec3& a) -> oxyVec3;
inline constexpr auto operator+(const oxyVec3& a, const oxyVec3& b) -> oxyVec3;
inline constexpr auto operator-(const oxyVec3& a, const oxyVec3& b) -> oxyVec3;
inline constexpr auto operator*(const oxyVec3& a, const oxyVec3& b) -> oxyVec3;
inline constexpr auto operator/(const oxyVec3& a, const oxyVec3& b) -> oxyVec3;
inline constexpr auto operator*(const oxyVec3& a, oxyF32 b) -> oxyVec3;
inline constexpr auto operator/(const oxyVec3& a, oxyF32 b) -> oxyVec3;

inline constexpr auto operator-(const oxyVec4& a) -> oxyVec4;
inline constexpr auto operator+(const oxyVec4& a, const oxyVec4& b) -> oxyVec4;
inline constexpr auto operator-(const oxyVec4& a, const oxyVec4& b) -> oxyVec4;
inline constexpr auto operator*(const oxyVec4& a, const oxyVec4& b) -> oxyVec4;
inline constexpr auto operator/(const oxyVec4& a, const oxyVec4& b) -> oxyVec4;
inline constexpr auto operator*(const oxyVec4& a, oxyF32 b) -> oxyVec4;
inline constexpr auto operator/(const oxyVec4& a, oxyF32 b) -> oxyVec4;
inline constexpr auto operator*(const oxyVec4& a,
								const oxyMat4x4& b) -> oxyVec4;

inline constexpr auto operator*(const oxyMat4x4& a,
								const oxyMat4x4& b) -> oxyMat4x4;
inline constexpr auto operator*(const oxyMat4x4& a, oxyF32 b) -> oxyMat4x4;
inline constexpr auto operator*(const oxyMat4x4& a,
								const oxyVec4& b) -> oxyVec4;

inline constexpr auto operator*(const oxyQuat& a, const oxyQuat& b) -> oxyQuat;
inline constexpr auto operator*(const oxyQuat& a, const oxyVec3& b) -> oxyVec3;
inline constexpr auto operator*(const oxyQuat& a, oxyF32 b) -> oxyQuat;
inline constexpr auto operator-(const oxyQuat& a) -> oxyQuat;
inline constexpr auto operator*(const oxyQuat& a,
								const oxyMat4x4& b) -> oxyMat4x4;

struct oxyVec2
{
	constexpr oxyVec2() : x(0), y(0)
	{
	}
	constexpr oxyVec2(oxyF32 x, oxyF32 y) : x(x), y(y)
	{
	}

	oxyF32 x, y;

	constexpr auto MagnitudeSquared() const -> oxyF32
	{
		return x * x + y * y;
	}
	auto Magnitude() const -> oxyF32
	{
		return std::sqrtf(x * x + y * y);
	}

	auto Normalized() const -> oxyVec2
	{
		oxyF32 mag = Magnitude();
		return {x / mag, y / mag};
	}
	auto Normalize() -> oxyVec2&
	{
		oxyF32 mag = Magnitude();
		x /= mag;
		y /= mag;
		return *this;
	}

	constexpr auto DotProduct(const oxyVec2& other) const -> oxyF32
	{
		return x * other.x + y * other.y;
	}

	constexpr auto CrossProduct(const oxyVec2& other) const -> oxyF32
	{
		return x * other.y - y * other.x;
	}

	constexpr auto Conjugate() const -> oxyVec2
	{
		return {-x, -y};
	}

	constexpr auto Inversed() const -> oxyVec2
	{
		return Conjugate() / (x * x + y * y);
	}
	constexpr auto Inverse() -> oxyVec2&
	{
		*this = Inversed();
		return *this;
	}

	constexpr auto operator+=(const oxyVec2& other) -> oxyVec2&
	{
		*this = *this + other;
		return *this;
	}
	constexpr auto operator-=(const oxyVec2& other) -> oxyVec2&
	{
		*this = *this - other;
		return *this;
	}
	constexpr auto operator*=(const oxyVec2& other) -> oxyVec2&
	{
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(const oxyVec2& other) -> oxyVec2&
	{
		*this = *this / other;
		return *this;
	}
	constexpr auto operator*=(oxyF32 other) -> oxyVec2&
	{
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(oxyF32 other) -> oxyVec2&
	{
		*this = *this / other;
		return *this;
	}
};

struct oxyVec3
{
	constexpr oxyVec3() : x(0), y(0), z(0)
	{
	}
	constexpr oxyVec3(oxyF32 x, oxyF32 y, oxyF32 z) : x(x), y(y), z(z)
	{
	}
	constexpr oxyVec3(const oxyVec2& v, oxyF32 z) : x(v.x), y(v.y), z(z)
	{
	}
	oxyF32 x, y, z;

	operator oxyVec2() const
	{
		return {x, y};
	}

	constexpr auto MagnitudeSquared() const -> oxyF32
	{
		return x * x + y * y + z * z;
	}
	auto Magnitude() const -> oxyF32
	{
		return std::sqrtf(x * x + y * y + z * z);
	}

	auto Normalized() const -> oxyVec3
	{
		const auto mag = Magnitude();
		return {x / mag, y / mag, z / mag};
	}
	auto Normalize() -> oxyVec3&
	{
		const auto mag = Magnitude();
		x /= mag;
		y /= mag;
		z /= mag;
		return *this;
	}

	constexpr auto DotProduct(const oxyVec3& other) const -> oxyF32
	{
		return x * other.x + y * other.y + z * other.z;
	}

	constexpr auto CrossProduct(const oxyVec3& other) const -> oxyVec3
	{
		return {y * other.z - z * other.y, z * other.x - x * other.z,
				x * other.y - y * other.x};
	}

	constexpr auto Conjugate() const -> oxyVec3
	{
		return {-x, -y, z};
	}

	constexpr auto Inversed() const -> oxyVec3
	{
		return Conjugate() / (x * x + y * y + z * z);
	}
	constexpr auto Inverse() -> oxyVec3&
	{
		*this = Inversed();
		return *this;
	}

	constexpr auto operator+=(const oxyVec3& other) -> oxyVec3&
	{
		*this = *this + other;
		return *this;
	}
	constexpr auto operator-=(const oxyVec3& other) -> oxyVec3&
	{
		*this = *this - other;
		return *this;
	}
	constexpr auto operator*=(const oxyVec3& other) -> oxyVec3&
	{
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(const oxyVec3& other) -> oxyVec3&
	{
		*this = *this / other;
		return *this;
	}
	constexpr auto operator*=(oxyF32 other) -> oxyVec3&
	{
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(oxyF32 other) -> oxyVec3&
	{
		*this = *this / other;
		return *this;
	}
};

struct oxyVec4
{
	constexpr oxyVec4() : x(0), y(0), z(0), w(0)
	{
	}
	constexpr oxyVec4(oxyF32 x, oxyF32 y, oxyF32 z, oxyF32 w)
		: x(x), y(y), z(z), w(w)
	{
	}
	constexpr oxyVec4(const oxyVec3& v, oxyF32 w) : x(v.x), y(v.y), z(v.z), w(w)
	{
	}
	constexpr oxyVec4(const oxyVec2& v, oxyF32 z, oxyF32 w)
		: x(v.x), y(v.y), z(z), w(w)
	{
	}
	oxyF32 x, y, z, w;

	operator oxyVec3() const
	{
		return {x, y, z};
	}
	operator oxyVec2() const
	{
		return {x, y};
	}

	constexpr auto MagnitudeSquared() const -> oxyF32
	{
		return x * x + y * y + z * z + w * w;
	}
	auto Magnitude() const -> oxyF32
	{
		return std::sqrtf(x * x + y * y + z * z + w * w);
	}

	auto Normalized() const -> oxyVec4
	{
		const auto mag = Magnitude();
		return {x / mag, y / mag, z / mag, w / mag};
	}
	auto Normalize() -> oxyVec4&
	{
		const auto mag = Magnitude();
		x /= mag;
		y /= mag;
		z /= mag;
		w /= mag;
		return *this;
	}

	constexpr auto DotProduct(const oxyVec4& other) const -> oxyF32
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	constexpr auto Conjugate() const -> oxyVec4
	{
		return {-x, -y, -z, w};
	}

	constexpr auto Inversed() const -> oxyVec4
	{
		return Conjugate() / (x * x + y * y + z * z + w * w);
	}
	constexpr auto Inverse() -> oxyVec4&
	{
		*this = Inversed();
		return *this;
	}
	constexpr auto CrossProduct(const oxyVec4& other) const -> oxyVec4
	{
		return {y * other.z - z * other.y, z * other.x - x * other.z,
				x * other.y - y * other.x, 0};
	}

	constexpr auto operator+=(const oxyVec4& other) -> oxyVec4&
	{
		*this = *this + other;
		return *this;
	}
	constexpr auto operator-=(const oxyVec4& other) -> oxyVec4&
	{
		*this = *this - other;
		return *this;
	}
	constexpr auto operator*=(const oxyVec4& other) -> oxyVec4&
	{
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(const oxyVec4& other) -> oxyVec4&
	{
		*this = *this / other;
		return *this;
	}
	constexpr auto operator*=(oxyF32 other) -> oxyVec4&
	{
		*this = *this * other;
		return *this;
	}
	constexpr auto operator/=(oxyF32 other) -> oxyVec4&
	{
		*this = *this / other;
		return *this;
	}
};

struct oxyMat4x4
{
	oxyF32 m[4][4];

	constexpr auto Determinant() const -> oxyF32
	{
		return m[0][0] * m[1][1] * m[2][2] * m[3][3] +
			   m[0][0] * m[1][2] * m[2][3] * m[3][1] +
			   m[0][0] * m[1][3] * m[2][1] * m[3][2] +
			   m[0][1] * m[1][0] * m[2][3] * m[3][2] +
			   m[0][1] * m[1][2] * m[2][0] * m[3][3] +
			   m[0][1] * m[1][3] * m[2][2] * m[3][0] +
			   m[0][2] * m[1][0] * m[2][1] * m[3][3] +
			   m[0][2] * m[1][1] * m[2][3] * m[3][0] +
			   m[0][2] * m[1][3] * m[2][0] * m[3][1] +
			   m[0][3] * m[1][0] * m[2][2] * m[3][1] +
			   m[0][3] * m[1][1] * m[2][0] * m[3][2] +
			   m[0][3] * m[1][2] * m[2][1] * m[3][0] -
			   m[0][0] * m[1][1] * m[2][3] * m[3][2] -
			   m[0][0] * m[1][2] * m[2][1] * m[3][3] -
			   m[0][0] * m[1][3] * m[2][2] * m[3][1] -
			   m[0][1] * m[1][0] * m[2][2] * m[3][3] -
			   m[0][1] * m[1][2] * m[2][3] * m[3][0] -
			   m[0][1] * m[1][3] * m[2][0] * m[3][2] -
			   m[0][2] * m[1][0] * m[2][3] * m[3][1] -
			   m[0][2] * m[1][1] * m[2][0] * m[3][3] -
			   m[0][2] * m[1][3] * m[2][1] * m[3][0] -
			   m[0][3] * m[1][0] * m[2][1] * m[3][2] -
			   m[0][3] * m[1][1] * m[2][2] * m[3][0] -
			   m[0][3] * m[1][2] * m[2][0] * m[3][1];
	}

	constexpr auto Transposed() const -> oxyMat4x4
	{
		oxyMat4x4 result;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				result.m[i][j] = m[j][i];
			}
		}
		return result;
	}
	constexpr auto Transpose() -> oxyMat4x4&
	{
		*this = Transposed();
		return *this;
	}

	constexpr auto operator[](int i) -> oxyF32*
	{
		return m[i];
	}
	constexpr auto operator[](int i) const -> const oxyF32*
	{
		return m[i];
	}

	constexpr auto operator*=(const oxyMat4x4& other) -> oxyMat4x4&
	{
		*this = *this * other;
		return *this;
	}

	static inline constexpr auto Identity() -> oxyMat4x4
	{
		oxyMat4x4 result{};
		result.m[0][0] = 1;
		result.m[1][1] = 1;
		result.m[2][2] = 1;
		result.m[3][3] = 1;
		return result;
	}
};

struct oxyQuat
{
	oxyF32 x{0.0f}, y{0.0f}, z{0.0f}, w{1.0f};

	constexpr auto MagnitudeSquared() const -> oxyF32
	{
		return x * x + y * y + z * z + w * w;
	}
	auto Magnitude() const -> oxyF32
	{
		return std::sqrtf(x * x + y * y + z * z + w * w);
	}

	auto Normalized() const -> oxyQuat
	{
		const auto mag = Magnitude();
		return {x / mag, y / mag, z / mag, w / mag};
	}
	auto Normalize() -> oxyQuat&
	{
		const auto mag = Magnitude();
		x /= mag;
		y /= mag;
		z /= mag;
		w /= mag;
		return *this;
	}

	constexpr auto DotProduct(const oxyQuat& other) const -> oxyF32
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	constexpr auto Conjugate() const -> oxyQuat
	{
		return {-x, -y, -z, w};
	}

	auto Inversed() const -> oxyQuat
	{
		const auto conj = Conjugate();
		const auto magsq = MagnitudeSquared();
		return {conj.x / magsq, conj.y / magsq, conj.z / magsq, conj.w / magsq};
	}
	auto Inverse() -> oxyQuat&
	{
		*this = Inversed();
		return *this;
	}

	constexpr auto operator*=(const oxyQuat& other) -> oxyQuat&
	{
		*this = *this * other;
		return *this;
	}
};

inline constexpr auto operator-(const oxyVec2& a) -> oxyVec2
{
	return {-a.x, -a.y};
}
inline constexpr auto operator+(const oxyVec2& a, const oxyVec2& b) -> oxyVec2
{
	return {a.x + b.x, a.y + b.y};
}
inline constexpr auto operator-(const oxyVec2& a, const oxyVec2& b) -> oxyVec2
{
	return {a.x - b.x, a.y - b.y};
}
inline constexpr auto operator*(const oxyVec2& a, const oxyVec2& b) -> oxyVec2
{
	return {a.x * b.x, a.y * b.y};
}
inline constexpr auto operator/(const oxyVec2& a, const oxyVec2& b) -> oxyVec2
{
	return {a.x / b.x, a.y / b.y};
}
inline constexpr auto operator*(const oxyVec2& a, oxyF32 b) -> oxyVec2
{
	return {a.x * b, a.y * b};
}
inline constexpr auto operator/(const oxyVec2& a, oxyF32 b) -> oxyVec2
{
	return {a.x / b, a.y / b};
}

inline constexpr auto operator-(const oxyVec3& a) -> oxyVec3
{
	return {-a.x, -a.y, -a.z};
}
inline constexpr auto operator+(const oxyVec3& a, const oxyVec3& b) -> oxyVec3
{
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline constexpr auto operator-(const oxyVec3& a, const oxyVec3& b) -> oxyVec3
{
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline constexpr auto operator*(const oxyVec3& a, const oxyVec3& b) -> oxyVec3
{
	return {a.x * b.x, a.y * b.y, a.z * b.z};
}
inline constexpr auto operator/(const oxyVec3& a, const oxyVec3& b) -> oxyVec3
{
	return {a.x / b.x, a.y / b.y, a.z / b.z};
}
inline constexpr auto operator*(const oxyVec3& a, oxyF32 b) -> oxyVec3
{
	return {a.x * b, a.y * b, a.z * b};
}
inline constexpr auto operator/(const oxyVec3& a, oxyF32 b) -> oxyVec3
{
	return {a.x / b, a.y / b, a.z / b};
}

inline constexpr auto operator-(const oxyVec4& a) -> oxyVec4
{
	return {-a.x, -a.y, -a.z, -a.w};
}
inline constexpr auto operator+(const oxyVec4& a, const oxyVec4& b) -> oxyVec4
{
	return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
inline constexpr auto operator-(const oxyVec4& a, const oxyVec4& b) -> oxyVec4
{
	return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
inline constexpr auto operator*(const oxyVec4& a, const oxyVec4& b) -> oxyVec4
{
	return {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}
inline constexpr auto operator/(const oxyVec4& a, const oxyVec4& b) -> oxyVec4
{
	return {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}
inline constexpr auto operator*(const oxyVec4& a, oxyF32 b) -> oxyVec4
{
	return {a.x * b, a.y * b, a.z * b, a.w * b};
}
inline constexpr auto operator/(const oxyVec4& a, oxyF32 b) -> oxyVec4
{
	return {a.x / b, a.y / b, a.z / b, a.w / b};
}
inline constexpr auto operator*(const oxyVec4& a, const oxyMat4x4& b) -> oxyVec4
{
	return {
		a.x * b.m[0][0] + a.y * b.m[1][0] + a.z * b.m[2][0] + a.w * b.m[3][0],
		a.x * b.m[0][1] + a.y * b.m[1][1] + a.z * b.m[2][1] + a.w * b.m[3][1],
		a.x * b.m[0][2] + a.y * b.m[1][2] + a.z * b.m[2][2] + a.w * b.m[3][2],
		a.x * b.m[0][3] + a.y * b.m[1][3] + a.z * b.m[2][3] + a.w * b.m[3][3]};
}

inline constexpr auto operator*(const oxyMat4x4& a,
								const oxyMat4x4& b) -> oxyMat4x4
{
	oxyMat4x4 result;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] +
							 a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
		}
	}
	return result;
}
inline constexpr auto operator*(const oxyMat4x4& a, oxyF32 b) -> oxyMat4x4
{
	oxyMat4x4 result = a;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] *= b;
		}
	}
	return result;
}
inline constexpr auto operator*(const oxyMat4x4& a, const oxyVec4& b) -> oxyVec4
{
	oxyVec4 result;
	for (int i = 0; i < 4; i++)
	{
		result.x += a.m[i][0] * b.x;
		result.y += a.m[i][1] * b.y;
		result.z += a.m[i][2] * b.z;
		result.w += a.m[i][3] * b.w;
	}
	return result;
}

inline constexpr auto operator*(const oxyQuat& a, const oxyQuat& b) -> oxyQuat
{
	return {a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
			a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
			a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
			a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
}
inline constexpr auto operator*(const oxyQuat& a, const oxyVec3& b) -> oxyVec3
{
	oxyQuat v = {b.x, b.y, b.z, 0};
	oxyQuat result = a * v * a.Conjugate();
	return {result.x, result.y, result.z};
}
inline constexpr auto operator*(const oxyQuat& a, oxyF32 b) -> oxyQuat
{
	return {a.x * b, a.y * b, a.z * b, a.w * b};
}
inline constexpr auto operator-(const oxyQuat& a) -> oxyQuat
{
	return {-a.x, -a.y, -a.z, -a.w};
}
inline constexpr auto operator*(const oxyQuat& a,
								const oxyMat4x4& b) -> oxyMat4x4
{
	oxyMat4x4 result;
	result.m[0][0] = 1 - 2 * a.y * a.y - 2 * a.z * a.z;
	result.m[0][1] = 2 * a.x * a.y - 2 * a.z * a.w;
	result.m[0][2] = 2 * a.x * a.z + 2 * a.y * a.w;
	result.m[0][3] = 0;
	result.m[1][0] = 2 * a.x * a.y + 2 * a.z * a.w;
	result.m[1][1] = 1 - 2 * a.x * a.x - 2 * a.z * a.z;
	result.m[1][2] = 2 * a.y * a.z - 2 * a.x * a.w;
	result.m[1][3] = 0;
	result.m[2][0] = 2 * a.x * a.z - 2 * a.y * a.w;
	result.m[2][1] = 2 * a.y * a.z + 2 * a.x * a.w;
	result.m[2][2] = 1 - 2 * a.x * a.x - 2 * a.y * a.y;
	result.m[2][3] = 0;
	result.m[3][0] = 0;
	result.m[3][1] = 0;
	result.m[3][2] = 0;
	result.m[3][3] = 1;
	return result * b;
}

namespace oxygen
{
	namespace Math
	{
		inline constexpr auto Translate(const oxyMat4x4& m,
										const oxyVec3& v) -> oxyMat4x4
		{
			oxyMat4x4 result = m;
			result.m[3][0] += v.x;
			result.m[3][1] += v.y;
			result.m[3][2] += v.z;
			return result;
		}
		inline constexpr auto Rotate(const oxyMat4x4& m,
									 const oxyQuat& q) -> oxyMat4x4
		{
			oxyMat4x4 result;
			result.m[0][0] = 1 - 2 * q.y * q.y - 2 * q.z * q.z;
			result.m[0][1] = 2 * q.x * q.y - 2 * q.z * q.w;
			result.m[0][2] = 2 * q.x * q.z + 2 * q.y * q.w;
			result.m[0][3] = 0;
			result.m[1][0] = 2 * q.x * q.y + 2 * q.z * q.w;
			result.m[1][1] = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
			result.m[1][2] = 2 * q.y * q.z - 2 * q.x * q.w;
			result.m[1][3] = 0;
			result.m[2][0] = 2 * q.x * q.z - 2 * q.y * q.w;
			result.m[2][1] = 2 * q.y * q.z + 2 * q.x * q.w;
			result.m[2][2] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
			result.m[2][3] = 0;
			result.m[3][0] = 0;
			result.m[3][1] = 0;
			result.m[3][2] = 0;
			result.m[3][3] = 1;
			return result * m;
		}
		inline auto Rotate(const oxyMat4x4& m, oxyF32 angle,
						   const oxyVec3& axis) -> oxyMat4x4
		{
			const auto halfAngle = angle / 2;
			const auto s = std::sinf(halfAngle);
			const auto q = oxyQuat{axis.x * s, axis.y * s, axis.z * s,
								   std::cosf(halfAngle)};
			return Rotate(m, q);
		}
		inline constexpr auto Scale(const oxyMat4x4& m,
									const oxyVec3& v) -> oxyMat4x4
		{
			oxyMat4x4 result = m;
			result.m[0][0] *= v.x;
			result.m[1][1] *= v.y;
			result.m[2][2] *= v.z;
			return result;
		}
		inline auto LookAt(const oxyVec3& eye, const oxyVec3& center,
						   const oxyVec3& up) -> oxyMat4x4
		{
			const auto f = (center - eye).Normalized();
			const auto s = up.CrossProduct(f).Normalized();
			const auto u = f.CrossProduct(s);
			oxyMat4x4 result{};
			result[0][0] = s.x;
			result[1][0] = s.y;
			result[2][0] = s.z;
			result[0][1] = u.x;
			result[1][1] = u.y;
			result[2][1] = u.z;
			result[0][2] = f.x;
			result[1][2] = f.y;
			result[2][2] = f.z;
			result[3][0] = -s.DotProduct(eye);
			result[3][1] = -u.DotProduct(eye);
			result[3][2] = -f.DotProduct(eye);
			result[3][3] = 1;
			return result;
		}
		inline auto Perspective(oxyF32 fovy, oxyF32 aspect, oxyF32 near,
								oxyF32 far) -> oxyMat4x4
		{
			const auto tanHalfFovy = std::tanf(fovy / 2.f);
			oxyMat4x4 result{};
			result[0][0] = 1.f / (aspect * tanHalfFovy);
			result[1][1] = 1.f / tanHalfFovy;
			result[2][2] = far / (far - near);
			result[2][3] = 1.f;
			result[3][2] = -(far * near) / (far - near);
			return result;
		}
		inline auto InverseMatrix(const oxyMat4x4& m) -> oxyMat4x4
		{
			oxyMat4x4 result;
			const auto det = m.Determinant();
			if (det == 0)
				return result;
			const auto invDet = 1 / det;
			result[0][0] =
				invDet *
				(m[1][1] * m[2][2] * m[3][3] + m[1][2] * m[2][3] * m[3][1] +
				 m[1][3] * m[2][1] * m[3][2] - m[1][1] * m[2][3] * m[3][2] -
				 m[1][2] * m[2][1] * m[3][3] - m[1][3] * m[2][2] * m[3][1]);
			result[0][1] =
				invDet *
				(m[0][1] * m[2][3] * m[3][2] + m[0][2] * m[2][1] * m[3][3] +
				 m[0][3] * m[2][2] * m[3][1] - m[0][1] * m[2][2] * m[3][3] -
				 m[0][2] * m[2][3] * m[3][1] - m[0][3] * m[2][1] * m[3][2]);
			result[0][2] =
				invDet *
				(m[0][1] * m[1][2] * m[3][3] + m[0][2] * m[1][3] * m[3][1] +
				 m[0][3] * m[1][1] * m[3][2] - m[0][1] * m[1][3] * m[3][2] -
				 m[0][2] * m[1][1] * m[3][3] - m[0][3] * m[1][2] * m[3][1]);
			result[0][3] =
				invDet *
				(m[0][1] * m[1][3] * m[2][2] + m[0][2] * m[1][1] * m[2][3] +
				 m[0][3] * m[1][2] * m[2][1] - m[0][1] * m[1][2] * m[2][3] -
				 m[0][2] * m[1][3] * m[2][1] - m[0][3] * m[1][1] * m[2][2]);

			result[1][0] =
				invDet *
				(m[1][0] * m[2][3] * m[3][2] + m[1][2] * m[2][0] * m[3][3] +
				 m[1][3] * m[2][2] * m[3][0] - m[1][0] * m[2][2] * m[3][3] -
				 m[1][2] * m[2][3] * m[3][0] - m[1][3] * m[2][0] * m[3][2]);
			result[1][1] =
				invDet *
				(m[0][0] * m[2][2] * m[3][3] + m[0][2] * m[2][3] * m[3][0] +
				 m[0][3] * m[2][0] * m[3][2] - m[0][0] * m[2][3] * m[3][2] -
				 m[0][2] * m[2][0] * m[3][3] - m[0][3] * m[2][2] * m[3][0]);
			result[1][2] =
				invDet *
				(m[0][0] * m[1][3] * m[3][2] + m[0][2] * m[1][0] * m[3][3] +
				 m[0][3] * m[1][2] * m[3][0] - m[0][0] * m[1][2] * m[3][3] -
				 m[0][2] * m[1][3] * m[3][0] - m[0][3] * m[1][0] * m[3][2]);
			result[1][3] =
				invDet *
				(m[0][0] * m[1][2] * m[2][3] + m[0][1] * m[1][3] * m[2][0] +
				 m[0][3] * m[1][0] * m[2][2] - m[0][0] * m[1][3] * m[2][2] -
				 m[0][1] * m[1][0] * m[2][3] - m[0][3] * m[1][2] * m[2][0]);

			result[2][0] =
				invDet *
				(m[1][0] * m[2][3] * m[3][1] + m[1][1] * m[2][0] * m[3][3] +
				 m[1][3] * m[2][1] * m[3][0] - m[1][0] * m[2][1] * m[3][3] -
				 m[1][1] * m[2][3] * m[3][0] - m[1][3] * m[2][0] * m[3][1]);
			result[2][1] =
				invDet *
				(m[0][0] * m[2][1] * m[3][3] + m[0][1] * m[2][3] * m[3][0] +
				 m[0][3] * m[2][0] * m[3][1] - m[0][0] * m[2][3] * m[3][1] -
				 m[0][1] * m[2][0] * m[3][3] - m[0][3] * m[2][1] * m[3][0]);
			result[2][2] =
				invDet *
				(m[0][0] * m[1][1] * m[3][3] + m[0][1] * m[1][3] * m[3][0] +
				 m[0][3] * m[1][0] * m[3][1] - m[0][0] * m[1][3] * m[3][1] -
				 m[0][1] * m[1][0] * m[3][3] - m[0][3] * m[1][1] * m[3][0]);
			result[2][3] =
				invDet *
				(m[0][0] * m[1][1] * m[2][3] + m[0][1] * m[1][3] * m[2][0] +
				 m[0][2] * m[1][0] * m[2][1] - m[0][0] * m[1][3] * m[2][1] -
				 m[0][1] * m[1][0] * m[2][3] - m[0][2] * m[1][1] * m[2][0]);

			result[3][0] =
				invDet *
				(m[1][0] * m[2][1] * m[3][2] + m[1][1] * m[2][2] * m[3][0] +
				 m[1][2] * m[2][0] * m[3][1] - m[1][0] * m[2][2] * m[3][1] -
				 m[1][1] * m[2][0] * m[3][2] - m[1][2] * m[2][1] * m[3][0]);
			result[3][1] =
				invDet *
				(m[0][0] * m[2][2] * m[3][1] + m[0][1] * m[2][0] * m[3][2] +
				 m[0][2] * m[2][1] * m[3][0] - m[0][0] * m[2][1] * m[3][2] -
				 m[0][1] * m[2][2] * m[3][0] - m[0][2] * m[2][0] * m[3][1]);
			result[3][2] =
				invDet *
				(m[0][0] * m[1][1] * m[3][0] + m[0][1] * m[1][0] * m[3][2] +
				 m[0][2] * m[1][2] * m[3][1] - m[0][0] * m[1][2] * m[3][1] -
				 m[0][1] * m[1][2] * m[3][0] - m[0][2] * m[1][0] * m[3][1]);
			result[3][3] =
				invDet *
				(m[0][0] * m[1][1] * m[2][0] + m[0][1] * m[1][0] * m[2][1] +
				 m[0][2] * m[1][1] * m[2][2] - m[0][0] * m[1][1] * m[2][1] -
				 m[0][1] * m[1][2] * m[2][0] - m[0][2] * m[1][0] * m[2][1]);
			return result;
		}
		inline auto RotationMatrixToEuler(const oxyMat4x4& m) -> oxyVec3
		{
			oxyVec3 result;
			result.y = std::asin(-m[2][0]);
			if (std::cos(result.y) != 0)
			{
				result.x = std::atan2(m[2][1], m[2][2]);
				result.z = std::atan2(m[1][0], m[0][0]);
			}
			else
			{
				result.x = 0;
				result.z = std::atan2(-m[0][1], m[1][1]);
			}
			return result;
		}
		inline auto Slerp(const oxyQuat& a, const oxyQuat& b,
						  oxyF32 t) -> oxyQuat
		{
			const auto dot = a.DotProduct(b);
			const auto angle = std::acosf(dot);
			const auto denom = std::sinf(angle);
			if (denom == 0)
				return a;
			const auto s0 = std::sinf((1 - t) * angle) / denom;
			const auto s1 = std::sinf(t * angle) / denom;
			const auto sa = a * s0;
			const auto sb = b * s1;
			return oxyQuat{sa.x + sb.x, sa.y + sb.y, sa.z + sb.z, sa.w + sb.w};
		}
		inline auto AngleAxisToQuat(const oxyF32 angle,
									const oxyVec3& axis) -> oxyQuat
		{
			const auto halfAngle = angle / 2;
			const auto s = std::sinf(halfAngle);
			return {axis.x * s, axis.y * s, axis.z * s, std::cos(halfAngle)};
		}
		inline auto QuatToEulerAngles(const oxyQuat& q) -> oxyVec3
		{
			oxyVec3 result;
			result.x = std::atan2(2 * (q.w * q.x + q.y * q.z),
								  1 - 2 * (q.x * q.x + q.y * q.y));
			result.y = std::asin(2 * (q.w * q.y - q.z * q.x));
			result.z = std::atan2(2 * (q.w * q.z + q.x * q.y),
								  1 - 2 * (q.y * q.y + q.z * q.z));
			return result;
		}
		inline auto EulerAnglesToQuat(const oxyVec3& v) -> oxyQuat
		{
			const auto pitch = AngleAxisToQuat(v.x, {1, 0, 0});
			const auto yaw = AngleAxisToQuat(v.y, {0, 1, 0});
			const auto roll = AngleAxisToQuat(v.z, {0, 0, 1});
			return pitch * yaw * roll;
		}
		inline auto QuatLookAt(const oxyVec3& position,
							   const oxyVec3& where) -> oxyQuat
		{
			const auto direction = (where - position).Normalized();
			const auto forward = oxyVec3{0, 0, 1};
			const auto axis = forward.CrossProduct(direction);
			const auto angle = std::acosf(forward.DotProduct(direction));
			return AngleAxisToQuat(angle, axis);
		}
		inline auto EulerForward(const oxyVec3& euler) -> oxyVec3
		{
			return oxyVec3{std::cosf(euler.z) * std::cosf(euler.x),
						   std::sinf(euler.z) * std::cosf(euler.x),
						   std::sinf(euler.x)};
		}
		inline auto ToHalfFloat(oxyF32 x) -> oxyU16
		{
			uint32_t i = std::bit_cast<uint32_t>(x);
			uint16_t bits = (i >> 16) & 0x8000;
			uint16_t m = (i >> 12) & 0x7ff;
			uint16_t e = ((i >> 23) & 0xff) - 127;
			if (e > 30)
			{
				bits |= 0x7c00;
			}
			else if (e > 15)
			{
				m |= 0x800;
				bits |= (e << 10) | (m >> 1);
			}
			else if (e > -15)
			{
				bits |= ((e + 15) << 10) | (m >> 1);
			}
			else if (e > -25)
			{
				m |= 0x800;
				bits |= ((e + 15) << 10) | (m >> 1);
			}
			else
			{
				bits |= 0;
			}
			return bits;
		}
		inline auto FromHalfFloat(oxyU16 x) -> oxyF32
		{
			uint32_t mantissa = x & 0x3ff;
			uint32_t exponent = x & 0x7c00;
			if (exponent == 0x7c00)
			{
				exponent = 0x8f << 23;
			}
			else if (exponent == 0)
			{
				if (mantissa != 0)
				{
					exponent = 0x71 << 23;
					while ((mantissa & 0x400) == 0)
					{
						mantissa <<= 1;
						exponent -= 0x800000;
					}
					mantissa &= 0x3ff;
				}
			}
			else
			{
				exponent = (exponent >> 10) + 0x70;
			}
			uint32_t result = (x & 0x8000) << 16;
			result |= (exponent << 23);
			result |= (mantissa << 13);
			return std::bit_cast<float>(result);
		}

	}; // namespace Math
}; // namespace oxygen