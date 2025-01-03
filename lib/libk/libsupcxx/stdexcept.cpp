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
 * stdexcept.cc - provides stub implementations of the exceptions required by the runtime.
 */
#include "bits/stdexcept.h"
namespace std {
exception::exception() noexcept {}
exception::~exception() noexcept {}
exception::exception(const exception&) noexcept {}
exception& exception::operator=(const exception&) noexcept
{
	return *this;
}
const char* exception::what() const noexcept
{
	return "std::exception";
}
bad_alloc::bad_alloc() noexcept {}
bad_alloc::~bad_alloc() noexcept {}
bad_alloc::bad_alloc(const bad_alloc&) noexcept {}
bad_alloc& bad_alloc::operator=(const bad_alloc&) noexcept
{
	return *this;
}
const char* bad_alloc::what() const noexcept
{
	return "cxxrt::bad_alloc";
}
bad_cast::bad_cast() noexcept {}
bad_cast::~bad_cast() noexcept {}
bad_cast::bad_cast(const bad_cast&) noexcept {}
bad_cast& bad_cast::operator=(const bad_cast&) noexcept
{
	return *this;
}
const char* bad_cast::what() const noexcept
{
	return "std::bad_cast";
}
bad_typeid::bad_typeid() noexcept {}
bad_typeid::~bad_typeid() noexcept {}
bad_typeid::bad_typeid(const bad_typeid &__rhs) noexcept {}
bad_typeid& bad_typeid::operator=(const bad_typeid &__rhs) noexcept
{
	return *this;
}
const char* bad_typeid::what() const noexcept
{
	return "std::bad_typeid";
}
bad_array_new_length::bad_array_new_length() noexcept {}
bad_array_new_length::~bad_array_new_length() noexcept {}
bad_array_new_length::bad_array_new_length(const bad_array_new_length&) noexcept {}
bad_array_new_length& bad_array_new_length::operator=(const bad_array_new_length&) noexcept
{
	return *this;
}
const char* bad_array_new_length::what() const noexcept
{
	return "std::bad_array_new_length";
}
} // namespace std