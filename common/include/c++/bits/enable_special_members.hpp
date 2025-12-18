// <bits/enable_special_members.h> -*- C++ -*-
// Copyright (C) 2013-2025 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.
// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.
// NOTE: I have changed the formatting and such, because underscores and PascalCase do not mix. I will die on this hill.
#ifndef __SPECIAL_MEMBERS
#define __SPECIAL_MEMBERS
namespace std
{
	struct __enable_default_constructor_tag { explicit constexpr __enable_default_constructor_tag() = default; };
	/**
	* @brief A mixin helper to conditionally enable or disable the default
	* constructor.
	* @sa __enable_special_members
	*/
	template<bool SB, typename T = void>
	struct __enable_default_constructor
	{
		constexpr __enable_default_constructor() noexcept = default;
		constexpr __enable_default_constructor(__enable_default_constructor const&) noexcept = default;
		constexpr __enable_default_constructor(__enable_default_constructor&&) noexcept = default;
		__enable_default_constructor& operator=(__enable_default_constructor const&) noexcept = default;
		__enable_default_constructor& operator=(__enable_default_constructor&&) noexcept = default;
		// Can be used in other ctors.
		constexpr explicit __enable_default_constructor(__enable_default_constructor_tag) {}
	};
	/**
	* @brief A mixin helper to conditionally enable or disable the default
	* destructor.
	* @sa __enable_special_members
	*/
	template<bool SB, typename T = void> struct __enable_destructor {};
	/**
	* @brief A mixin helper to conditionally enable or disable the copy/move
	* special members.
	* @sa __enable_special_members
	*/
	template<bool CC, bool CA, bool MC, bool MA, typename T = void> struct __enable_copy_move {};
	/**
	* @brief A mixin helper to conditionally enable or disable the special
	* members.
	*
	* The @c T type parameter is to make mixin bases unique and thus avoid
	* ambiguities.
	*/
	template<bool DV, bool D, bool CC, bool CA, bool MC, bool MA, typename T = void>
	struct __enable_special_members :
		private __enable_default_constructor<DV, T>,
		private __enable_destructor<D, T>,
		private __enable_copy_move<CC, CA, MC, MA, T> {};
	// Boilerplate follows.
	template<typename T>
	struct __enable_default_constructor<false, T>
	{
		constexpr __enable_default_constructor() noexcept = delete;
		constexpr __enable_default_constructor(__enable_default_constructor const&) noexcept = default;
		constexpr __enable_default_constructor(__enable_default_constructor&&) noexcept = default;
		__enable_default_constructor& operator=(__enable_default_constructor const&) noexcept = default;
		__enable_default_constructor& operator=(__enable_default_constructor&&) noexcept = default;
		// Can be used in other ctors.
		constexpr explicit __enable_default_constructor(__enable_default_constructor_tag) {}
	};
	template<typename T> struct __enable_destructor<false, T> { ~__enable_destructor() noexcept = delete; };
	template<typename T>
	struct __enable_copy_move<false, true, true, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};
	template<typename T>
	struct __enable_copy_move<true, false, true, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};
	template<typename T>
	struct __enable_copy_move<false, false, true, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};
	template<typename T>
	struct __enable_copy_move<true, true, false, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};
	template<typename T>
	struct __enable_copy_move<false, true, false, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};

	template<typename T>
	struct __enable_copy_move<true, false, false, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};
	template<typename T>
	struct __enable_copy_move<false, false, false, true, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = default;
	};
	template<typename T>
	struct __enable_copy_move<true, true, true, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<false, true, true, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<true, false, true, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<false, false, true, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<true, true, false, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<false, true, false, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = default;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<true, false, false, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
	template<typename T>
	struct __enable_copy_move<false, false, false, false, T>
	{
		constexpr __enable_copy_move() noexcept = default;
		constexpr __enable_copy_move(__enable_copy_move const&) noexcept = delete;
		constexpr __enable_copy_move(__enable_copy_move&&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move const&) noexcept = delete;
		__enable_copy_move& operator=(__enable_copy_move&&) noexcept = delete;
	};
}
#endif