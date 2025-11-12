#ifndef __SCALABLE_MULTIARRAY
#define __SCALABLE_MULTIARRAY
#include <kernel_api.hpp>
namespace ooos
{
	struct vec2
	{
		ssize_t x;
		ssize_t y;
		constexpr vec2 operator+(vec2 const& that) const noexcept { return vec2(this->x + that.x, this->y + that.y); }
		constexpr vec2 operator-(vec2 const& that) const noexcept { return vec2(this->x - that.x, this->y - that.y); }
		constexpr vec2 operator*(ssize_t scale) const noexcept { return vec2(x * scale, y * scale); }
		constexpr vec2 operator/(ssize_t divisor) const noexcept { return vec2(x / divisor, y / divisor); }
		constexpr vec2& operator+=(vec2 const& that) noexcept { return *this = *this + that; }
		constexpr vec2& operator-=(vec2 const& that) noexcept { return *this = *this - that; }
		constexpr vec2& operator*=(ssize_t scale) noexcept { return *this = *this * scale; }
		constexpr vec2& operator/=(ssize_t divisor) noexcept { return *this = *this / divisor; }
		constexpr ssize_t dot(vec2 const& that) const noexcept { return this->x * that.x + this->y * that.y; }
	};
	struct vec3
	{
		ssize_t x;
		ssize_t y;
		ssize_t z;
		constexpr vec3 operator+(vec3 const& that) const noexcept { return vec3(this->x + that.x, this->y + that.y, this->z + that.z); }
		constexpr vec3 operator-(vec3 const& that) const noexcept { return vec3(this->x + that.x, this->y + that.y, this->z + that.z); }
		constexpr vec3 operator*(ssize_t scale) const noexcept { return vec3(x * scale, y * scale, z * scale); }
		constexpr vec3 operator/(ssize_t divisor) const noexcept { return vec3(x / divisor, y / divisor, z / divisor); }
		constexpr vec3& operator+=(vec3 const& that) noexcept { return *this = *this + that; }
		constexpr vec3& operator-=(vec3 const& that) noexcept { return *this = *this - that; }
		constexpr vec3& operator*=(ssize_t scale) noexcept { return *this = *this * scale; }
		constexpr vec3& operator*=(vec3 const& that) noexcept { return *this = this->cross(that); }
		constexpr vec3& operator/=(ssize_t divisor) noexcept { return *this = *this / divisor; }
		constexpr ssize_t dot(vec3 const& that) const noexcept { return this->x * that.x + this->y * that.y + this->z * that.z; }
		constexpr vec3 cross(vec3 const& that) const noexcept { return vec3(this->y * that.z - this->z * that.y, this->z * that.x - this->x * that.z, this->x * that.y - this->y * that.x); }
	};
	template<typename T>
	struct scalable_array
	{
		T* ptr;
		size_t scale;
		size_t size;
		constexpr T& operator[](size_t idx) noexcept { return ptr[(idx % size) * scale]; }
		constexpr T* operator+(size_t idx) noexcept { return ptr + (idx % size) * scale; }
		constexpr T const& operator[](size_t idx) const noexcept { return ptr[(idx % size) * scale]; }
		constexpr T const* operator+(size_t idx) const noexcept { return ptr + (idx % size) * scale; }
	};
	template<typename T>
	struct scalable_2d_array
	{
		T* ptr;
		vec2 size;
		vec2 scale;
		constexpr scalable_array<T> operator[](size_t i) noexcept { return scalable_array<T>(ptr + scale.x * (i % size.x), scale.y * scale.x * size.x, size.y); }
		constexpr scalable_array<T const> operator[](size_t i) const noexcept { return scalable_array<T const>(ptr + scale.x * (i % size.x), scale.y * scale.x * size.x, size.y); }
		constexpr T& operator[](size_t i, size_t j) noexcept { return ptr[scale.x * (i % size.x) + size.x * scale.y * scale.x * (j % size.y)]; }
		constexpr T const& operator[](size_t i, size_t j) const noexcept { return ptr[scale.x * (i % size.x) + size.x * scale.y * scale.x  * (j % size.y)]; }
	};
	template<typename T>
	struct scalable_3d_array
	{
		T* ptr;
		vec3 size;
		vec3 scale;
	private:
		constexpr size_t __offset(size_t i, size_t j) const noexcept { return static_cast<size_t>(scale.x * (i % size.x) + size.x * scale.y * scale.x * (j % size.y)); }
		constexpr size_t __offset(size_t i, size_t j, size_t k) const noexcept { return __offset(i, j) + scale.z * scale.y * scale.x * size.y * size.x * (k % size.z); }
		constexpr size_t __yz_scale() const noexcept { return static_cast<size_t>(scale.z * size.y * size.x); }
		typedef scalable_2d_array<T> __2d;
		typedef scalable_2d_array<T const> __c2d;
		typedef scalable_array<T> __1d;
		typedef scalable_array<T const> __c1d;
	public:
		constexpr scalable_2d_array<T> operator[](size_t i) noexcept { return __2d(ptr + scale.x * (i % size.x), vec2(scale.y * size.x, scale.z * size.x), vec2(size.y, size.z)); }
		constexpr scalable_array<T> operator[](size_t i, size_t j) noexcept { return __1d(ptr + __offset(i, j), __yz_scale(), static_cast<size_t>(size.z)); }
		constexpr T& operator[](size_t i, size_t j, size_t k) noexcept { return ptr[__offset(i, j, k)]; }
		constexpr scalable_2d_array<T const> operator[](size_t i) const noexcept { return scalable_2d_array<T const>(ptr + scale.x * (i % size.x), vec2(scale.y * size.x, scale.z * size.x), vec2(size.y, size.z)); }
		constexpr scalable_array<T const> operator[](size_t i, size_t j) const noexcept { return __c2d(ptr + __offset(i, j), __yz_scale(), static_cast<size_t>(size.z)); }
		constexpr T const& operator[](size_t i, size_t j, size_t k) const noexcept { return ptr[__offset(i, j, k)]; }
	};
}
#endif