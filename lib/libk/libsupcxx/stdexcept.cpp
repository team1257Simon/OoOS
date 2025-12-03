/*
 * Copyright 2010-2011 PathScale, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *		this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *		this list of conditions and the following disclaimer in the documentation
 *		and/or other materials provided with the distribution.
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
 * stdexcept.cc - provides stub implementations of the exceptions required by the runtime.
 */
#include <bits/stdexcept.h>
namespace std
{
	exception::exception() noexcept {}
	exception::~exception() noexcept {}
	exception::exception(exception const&) noexcept {}
	exception& exception::operator=(exception const&) noexcept { return *this; }
	const char*	exception::what() const noexcept { return "std::exception"; }
	bad_alloc::bad_alloc() noexcept {}
	bad_alloc::~bad_alloc() noexcept {}
	bad_alloc::bad_alloc(bad_alloc const&) noexcept {}
	bad_alloc& bad_alloc::operator=(bad_alloc const&) noexcept { return *this; }
	const char* bad_alloc::what() const noexcept { return "std::bad_alloc"; }
	bad_cast::bad_cast() noexcept {}
	bad_cast::~bad_cast() noexcept {}
	bad_cast::bad_cast(const bad_cast&) noexcept {}
	bad_cast& bad_cast::operator=(bad_cast const&) noexcept { return *this; }
	const char* bad_cast::what() const noexcept { return "std::bad_cast"; }
	bad_typeid::bad_typeid() noexcept {}
	bad_typeid::~bad_typeid() noexcept {}
	bad_typeid::bad_typeid(bad_typeid const&) noexcept {}
	bad_typeid& bad_typeid::operator=(bad_typeid const&) noexcept { return *this; }
	const char* bad_typeid::what() const noexcept { return "std::bad_typeid"; }
	bad_array_new_length::bad_array_new_length() noexcept {}
	bad_array_new_length::~bad_array_new_length() noexcept {}
	bad_array_new_length::bad_array_new_length(bad_array_new_length const&) noexcept {}
	bad_array_new_length& bad_array_new_length::operator=(bad_array_new_length const&) noexcept { return *this; }
	const char* bad_array_new_length::what() const noexcept { return "std::bad_array_new_length"; }
	logic_error::logic_error(string const& __msg) : exception(), __my_msg(__msg) {}
	logic_error::logic_error(const char* __msg) : exception(), __my_msg(__msg) {}
	logic_error::logic_error(logic_error const& that) : exception(that), __my_msg(that.__my_msg) {}
	logic_error::logic_error(logic_error&& that) : exception(that), __my_msg(move(that.__my_msg)) {}
	logic_error& logic_error::operator=(logic_error const& that) noexcept { this->__my_msg = that.__my_msg; return *this; }
	logic_error& logic_error::operator=(logic_error&&) noexcept = default;
	logic_error::~logic_error() noexcept {}
	const char* logic_error::what() const noexcept { return __my_msg.c_str(); }
	domain_error::domain_error(string const& __msg) : logic_error(__msg) {}
	domain_error::domain_error(const char* __msg) : logic_error(__msg) {}
	domain_error::~domain_error() noexcept {}
	invalid_argument::invalid_argument(string const& __msg) : logic_error(__msg) {}
	invalid_argument::invalid_argument(const char* __msg) : logic_error(__msg) {}
	invalid_argument::~invalid_argument() noexcept {}
	length_error::length_error(string const& __msg) : logic_error(__msg) {}
	length_error::length_error(const char* __msg) : logic_error(__msg) {}
	length_error::~length_error() noexcept {}
	out_of_range::out_of_range(string const& __msg) : logic_error(__msg) {}
	out_of_range::out_of_range(const char* __msg) : logic_error(__msg) {}
	out_of_range::~out_of_range() noexcept {}
	runtime_error::runtime_error(string const& __msg) : exception(), __my_msg(__msg) {}
	runtime_error::runtime_error(const char* __msg) : exception(), __my_msg(__msg) {}
	runtime_error::runtime_error(runtime_error&& that) : exception(that), __my_msg(move(that.__my_msg)) {}
	runtime_error::runtime_error(runtime_error const& that) : exception(that), __my_msg(that.__my_msg) {}
	runtime_error& runtime_error::operator=(runtime_error const& that) noexcept { this->__my_msg = that.__my_msg; return *this; }
	runtime_error& runtime_error::operator=(runtime_error&&) noexcept = default;
	runtime_error::~runtime_error() noexcept {}
	const char* runtime_error::what() const noexcept { return __my_msg.c_str(); }
	range_error::range_error(string const& __msg) : runtime_error(__msg) {}
	range_error::range_error(const char* __msg) : runtime_error(__msg) {}
	range_error::~range_error() noexcept {}
	overflow_error::overflow_error(string const& __msg) : runtime_error(__msg) {}
	overflow_error::overflow_error(const char* __msg) : runtime_error(__msg) {}
	overflow_error::~overflow_error() noexcept {}
	underflow_error::underflow_error(string const& __msg) : runtime_error(__msg) {}
	underflow_error::underflow_error(const char* __msg) : runtime_error(__msg) {}
	underflow_error::~underflow_error() noexcept {}
} // namespace std
extension void xpanic(std::string const& str) noexcept { panic(str.c_str()); }