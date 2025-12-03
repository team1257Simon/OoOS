/*
 * Copyright 2010-2011 PathScale, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * stdexcept.h - provides a stub version of <stdexcept>, which defines enough
 * of the exceptions for the runtime to use.
 */
#ifndef __STDEXCEPT_H
#define __STDEXCEPT_H
#include <bits/cxxabi.h>
#include <bits/exception.h>
#include <string>
namespace std
{
	class bad_alloc: public exception
	{
	public:
		bad_alloc() noexcept;
		bad_alloc(bad_alloc const&) noexcept;
		bad_alloc& operator=(bad_alloc const&) noexcept;
		~bad_alloc() noexcept;
		virtual const char* what() const noexcept;
	};
	class bad_cast: public exception
	{
	public:
		bad_cast() noexcept;
		bad_cast(const bad_cast&) noexcept;
		bad_cast& operator=(const bad_cast&) noexcept;
		virtual ~bad_cast() noexcept;
		virtual const char* what() const noexcept;
	};
	class bad_typeid: public exception
	{
	public:
		bad_typeid() noexcept;
		bad_typeid(bad_typeid const&) noexcept;
		bad_typeid& operator=(bad_typeid const&) noexcept;
		virtual ~bad_typeid() noexcept;
		virtual const char* what() const noexcept;
	};
	class bad_array_new_length: public bad_alloc
	{
	public:
		bad_array_new_length() noexcept;
		bad_array_new_length(bad_array_new_length const&) noexcept;
		bad_array_new_length& operator=(bad_array_new_length const&) noexcept;
		virtual ~bad_array_new_length() noexcept;
		virtual const char *what() const noexcept;
	};
	class logic_error : public exception
	{
		string __my_msg;
	public:
		explicit logic_error(string const& __msg);
		explicit logic_error(const char*);
		logic_error(logic_error&&);
		logic_error& operator=(logic_error&&) noexcept;
		logic_error(logic_error const&);
		logic_error& operator=(logic_error const&) noexcept;
		virtual ~logic_error() noexcept;
		virtual const char* what() const noexcept;
	};
	class domain_error : public logic_error
	{
	public:
		explicit domain_error(string const&);
		explicit domain_error(const char*);
		domain_error(domain_error const&) = default;
		domain_error& operator=(domain_error const&) = default;
		domain_error(domain_error&&) = default;
		domain_error& operator=(domain_error&&) = default;
		virtual ~domain_error() noexcept;
	};
	class invalid_argument : public logic_error
	{
	public:
		explicit invalid_argument(string const&);
		explicit invalid_argument(const char*);
		invalid_argument(invalid_argument const&) = default;
		invalid_argument& operator=(invalid_argument const&) = default;
		invalid_argument(invalid_argument&&) = default;
		invalid_argument& operator=(invalid_argument&&) = default;
		virtual ~invalid_argument() noexcept;
	};
	class length_error : public logic_error
	{
	public:
		explicit length_error(string const&);
		explicit length_error(const char*);
		length_error(length_error const&) = default;
		length_error& operator=(length_error const&) = default;
		length_error(length_error&&) = default;
		length_error& operator=(length_error&&) = default;
		virtual ~length_error() noexcept;
	};
	class out_of_range : public logic_error
	{
	public:
		explicit out_of_range(const string&);
		explicit out_of_range(const char*);
		out_of_range(out_of_range const&) = default;
		out_of_range& operator=(out_of_range const&) = default;
		out_of_range(out_of_range&&) = default;
		out_of_range& operator=(out_of_range&&) = default;
		virtual ~out_of_range() noexcept;
	};
	class runtime_error : public exception
	{
		string __my_msg;
	public:
		explicit runtime_error(const string&);
		explicit runtime_error(const char*);
		runtime_error(runtime_error&&);
		runtime_error& operator=(runtime_error&&) noexcept;
		runtime_error(runtime_error const&);
		runtime_error& operator=(runtime_error const&) noexcept;
		virtual ~runtime_error() noexcept;
		virtual const char* what() const noexcept;
	};
	class range_error : public runtime_error
	{
	public:
		explicit range_error(const string&);
		explicit range_error(const char*);
		range_error(const range_error&) = default;
		range_error& operator=(const range_error&) = default;
		range_error(range_error&&) = default;
		range_error& operator=(range_error&&) = default;
		virtual ~range_error() noexcept;
	};
	class overflow_error : public runtime_error
	{
	public:
		explicit overflow_error(const string&);
		explicit overflow_error(const char*);
		overflow_error(const overflow_error&) = default;
		overflow_error& operator=(const overflow_error&) = default;
		overflow_error(overflow_error&&) = default;
		overflow_error& operator=(overflow_error&&) = default;
		virtual ~overflow_error() noexcept;
	};
	class underflow_error : public runtime_error
	{
	public:
		explicit underflow_error(const string&);
		explicit underflow_error(const char*);
		underflow_error(const underflow_error&) = default;
		underflow_error& operator=(const underflow_error&) = default;
		underflow_error(underflow_error&&) = default;
		underflow_error& operator=(underflow_error&&) = default;
		virtual ~underflow_error() noexcept;
	};
}
#endif