/*
Copyright © Bubi Technologies Co., Ltd. 2017 All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
		 http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
base_int.h
An unsigned 128 bit integer type for C++
Copyright (c) 2014 Jason Lee @ calccrypto at gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

With much help from Auston Sterling

Thanks to Stefan Deigmller for finding
a bug in operator*.

Thanks to Franois Dessenne for convincing me
to do a general rewrite of this class.
*/

#ifndef __UINT128_T__
#define __UINT128_T__

#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <utility>
#include <string>
#include "crypto.h"
#include "strings.h"
#include "basen.h"

class uint128_t {
private:
	uint64_t UPPER, LOWER;
public:
	// Constructors
	uint128_t();
	uint128_t(const uint128_t & rhs);

	template <typename T> explicit uint128_t(const T & rhs) {
		UPPER = 0;
		LOWER = rhs;
	}

	template <typename S, typename T> uint128_t(const S & upper_rhs, const T & lower_rhs) {
		UPPER = upper_rhs;
		LOWER = lower_rhs;
	}
	//  RHS input args only
	// Assignment Operator
	uint128_t operator=(const uint128_t & rhs);

	template <typename T> uint128_t operator=(const T & rhs) {
		UPPER = 0;
		LOWER = rhs;
		return *this;
	}
	// Typecast Operators
	operator bool() const;
	operator char() const;
	operator int() const;
	operator uint8_t() const;
	operator uint16_t() const;
	operator uint32_t() const;
	operator uint64_t() const;
	// Bitwise Operators
	uint128_t operator&(const uint128_t & rhs) const;
	uint128_t operator|(const uint128_t & rhs) const;
	uint128_t operator^(const uint128_t & rhs) const;
	uint128_t operator&=(const uint128_t & rhs);
	uint128_t operator|=(const uint128_t & rhs);
	uint128_t operator^=(const uint128_t & rhs);
	uint128_t operator~() const;

	template <typename T> uint128_t operator&(const T & rhs) const {
		return uint128_t(0, LOWER & (uint64_t)rhs);
	}

	template <typename T> uint128_t operator|(const T & rhs) const {
		return uint128_t(UPPER, LOWER | (uint64_t)rhs);
	}

	template <typename T> uint128_t operator^(const T & rhs) const {
		return uint128_t(UPPER, LOWER ^ (uint64_t)rhs);
	}

	template <typename T> uint128_t operator&=(const T & rhs) {
		UPPER = 0;
		LOWER &= rhs;
		return *this;
	}

	template <typename T> uint128_t operator|=(const T & rhs) {
		LOWER |= (uint64_t)rhs;
		return *this;
	}

	template <typename T> uint128_t operator^=(const T & rhs) {
		LOWER ^= (uint64_t)rhs;
		return *this;
	}
	// Bit Shift Operators
	uint128_t operator<<(const uint128_t & rhs) const;
	uint128_t operator>>(const uint128_t & rhs) const;
	uint128_t operator<<=(const uint128_t & rhs);
	uint128_t operator>>=(const uint128_t & rhs);

	template <typename T>uint128_t operator<<(const T & rhs) const {
		return *this << uint128_t(rhs);
	}

	template <typename T>uint128_t operator>>(const T & rhs) const {
		return *this >> uint128_t(rhs);
	}

	template <typename T>uint128_t operator<<=(const T & rhs) {
		*this = *this << uint128_t(rhs);
		return *this;
	}

	template <typename T>uint128_t operator>>=(const T & rhs) {
		*this = *this >> uint128_t(rhs);
		return *this;
	}
	// Logical Operators
	bool operator!() const;
	bool operator&&(const uint128_t & rhs) const;
	bool operator||(const uint128_t & rhs) const;

	template <typename T> bool operator&&(const T & rhs) {
		return ((bool)*this && rhs);
	}

	template <typename T> bool operator||(const T & rhs) {
		return ((bool)*this || rhs);
	}
	// Comparison Operators
	bool operator==(const uint128_t & rhs) const;
	bool operator!=(const uint128_t & rhs) const;
	bool operator>(const uint128_t & rhs) const;
	bool operator<(const uint128_t & rhs) const;
	bool operator>=(const uint128_t & rhs) const;
	bool operator<=(const uint128_t & rhs) const;

	template <typename T> bool operator==(const T & rhs) const {
		return (!UPPER && (LOWER == (uint64_t)rhs));
	}

	template <typename T> bool operator!=(const T & rhs) const {
		return (UPPER | (LOWER != (uint64_t)rhs));
	}

	template <typename T> bool operator>(const T & rhs) const {
		return (UPPER || (LOWER > (uint64_t)rhs));
	}

	template <typename T> bool operator<(const T & rhs) const {
		return (!UPPER) ? (LOWER < (uint64_t)rhs) : false;
	}

	template <typename T> bool operator>=(const T & rhs) const {
		return ((*this > rhs) | (*this == rhs));
	}

	template <typename T> bool operator<=(const T & rhs) const {
		return ((*this < rhs) | (*this == rhs));
	}
	// Arithmetic Operators
	uint128_t operator+(const uint128_t & rhs) const;
	uint128_t operator+=(const uint128_t & rhs);
	uint128_t operator-(const uint128_t & rhs) const;
	uint128_t operator-=(const uint128_t & rhs);
	uint128_t operator*(const uint128_t & rhs) const;
	uint128_t operator*=(const uint128_t & rhs);
private:
	std::pair <uint128_t, uint128_t> divmod(const uint128_t & lhs, const uint128_t & rhs) const;
public:
	uint128_t operator/(const uint128_t & rhs) const;
	uint128_t operator/=(const uint128_t & rhs);
	uint128_t operator%(const uint128_t & rhs) const;
	uint128_t operator%=(const uint128_t & rhs);

	template <typename T> uint128_t operator+(const T & rhs) const {
		return uint128_t(UPPER + ((LOWER + (uint64_t)rhs) < LOWER), LOWER + (uint64_t)rhs);
	}

	template <typename T> uint128_t operator+=(const T & rhs) {
		UPPER = UPPER + ((LOWER + rhs) < LOWER);
		LOWER = LOWER + rhs;
		return *this;
	}

	template <typename T> uint128_t operator-(const T & rhs) const {
		return uint128_t((uint64_t)(UPPER - ((LOWER - rhs) > LOWER)), (uint64_t)(LOWER - rhs));
	}

	template <typename T> uint128_t operator-=(const T & rhs) {
		*this = *this - rhs;
		return *this;
	}

	template <typename T> uint128_t operator*(const T & rhs) const {
		return (*this) * (uint128_t(rhs));
	}

	template <typename T> uint128_t operator*=(const T & rhs) {
		*this = *this * uint128_t(rhs);
		return *this;
	}

	template <typename T> uint128_t operator/(const T & rhs) const {
		return *this / uint128_t(rhs);
	}

	template <typename T> uint128_t operator/=(const T & rhs) {
		*this = *this / uint128_t(rhs);
		return *this;
	}

	template <typename T> uint128_t operator%(const T & rhs) const {
		return *this - (rhs * (*this / rhs));
	}

	template <typename T> uint128_t operator%=(const T & rhs) {
		*this = *this % uint128_t(rhs);
		return *this;
	}
	// Increment Operator
	uint128_t operator++();
	uint128_t operator++(int);
	// Decrement Operator
	uint128_t operator--();
	uint128_t operator--(int);
	// Get private values
	uint64_t upper() const;
	uint64_t lower() const;
	// Get bitsize of value
	uint8_t bits() const;
	// Get string representation of value
	std::string str(uint8_t base = 10, const unsigned int & len = 0) const;
};
// Useful values
extern const uint128_t uint128_0;
extern const uint128_t uint128_1;
extern const uint128_t uint128_64;
extern const uint128_t uint128_128;
// lhs type T as first arguemnt
// If the output is not a bool, casts to type T
// Bitwise Operators
template <typename T> T operator&(const T & lhs, const uint128_t & rhs) {
	return (T)(lhs & (T)rhs.lower());
}

template <typename T> T operator|(const T & lhs, const uint128_t & rhs) {
	return (T)(lhs | (T)rhs.lower());
}

template <typename T> T operator^(const T & lhs, const uint128_t & rhs) {
	return (T)(lhs ^ (T)rhs.lower());
}

template <typename T> T operator&=(T & lhs, const uint128_t & rhs) {
	lhs &= (T)rhs.lower(); return lhs;
}

template <typename T> T operator|=(T & lhs, const uint128_t & rhs) {
	lhs |= (T)rhs.lower(); return lhs;
}

template <typename T> T operator^=(T & lhs, const uint128_t & rhs) {
	lhs ^= (T)rhs.lower(); return lhs;
}
// Comparison Operators
template <typename T> bool operator==(const T & lhs, const uint128_t & rhs) {
	return (!rhs.upper() && ((uint64_t)lhs == rhs.lower()));
}

template <typename T> bool operator!=(const T & lhs, const uint128_t & rhs) {
	return (rhs.upper() | ((uint64_t)lhs != rhs.lower()));
}

template <typename T> bool operator>(const T & lhs, const uint128_t & rhs) {
	return (!rhs.upper()) && ((uint64_t)lhs > rhs.lower());
}

template <typename T> bool operator<(const T & lhs, const uint128_t & rhs) {
	if (rhs.upper()) {
		return true;
	}
	return ((uint64_t)lhs < rhs.lower());
}

template <typename T> bool operator>=(const T & lhs, const uint128_t & rhs) {
	if (rhs.upper()) {
		return false;
	}
	return ((uint64_t)lhs >= rhs.lower());
}

template <typename T> bool operator<=(const T & lhs, const uint128_t & rhs) {
	if (rhs.upper()) {
		return true;
	}
	return ((uint64_t)lhs <= rhs.lower());
}
// Arithmetic Operators
template <typename T> T operator+(const T & lhs, const uint128_t & rhs) {
	return (T)(rhs + lhs);
}

template <typename T> T & operator+=(T & lhs, const uint128_t & rhs) {
	lhs = (T)(rhs + lhs);
	return lhs;
}

template <typename T> T operator-(const T & lhs, const uint128_t & rhs) {
	return (T)(uint128_t(lhs) - rhs);
}

template <typename T> T & operator-=(T & lhs, const uint128_t & rhs) {
	lhs = (T)(uint128_t(lhs) - rhs);
	return lhs;
}

template <typename T> T operator*(const T & lhs, const uint128_t & rhs) {
	return lhs * (T)rhs.lower();
}

template <typename T> T & operator*=(T & lhs, const uint128_t & rhs) {
	lhs *= (T)rhs.lower();
	return lhs;
}

template <typename T> T operator/(const T & lhs, const uint128_t & rhs) {
	return (T)(uint128_t(lhs) / rhs);
}

template <typename T> T & operator/=(T & lhs, const uint128_t & rhs) {
	lhs = (T)(uint128_t(lhs) / rhs);
	return lhs;
}

template <typename T> T operator%(const T & lhs, const uint128_t & rhs) {
	return (T)(uint128_t(lhs) % rhs);
}

template <typename T> T & operator%=(T & lhs, const uint128_t & rhs) {
	lhs = (T)(uint128_t(lhs) % rhs);
	return lhs;
}
// IO Operator
std::ostream & operator<<(std::ostream & stream, const uint128_t & rhs);
namespace utils {
	template <std::size_t bits>
	class base_uint {
		static_assert ((bits % 32) == 0,
			"The length of the base_uint must be a multiple of 32.");
		typedef char*	pointer;
	public:
		int get_bytes() {
			return WIDTH;
		}
		void zero() {
			data_.resize(WIDTH);
			for (int i = 0; i < WIDTH; i++) {
				data_[i] = 0;
			}
		}
		void init(const std::string &input) {
			assert(input.size() == WIDTH);
			data_ = input;
		}
		bool FromB16(const std::string &str) {
			std::string strDecode("");
			utils::decode_b16(str, strDecode);
			if (strDecode.size() != WIDTH) {
				return false;
			}
			init(strDecode);
			return true;
		}
		std::string ToB16() {
			return utils::encode_b16(data_);
		}
		std::string ToHex() {
			std::string str;
			for (std::size_t i = 0; i < WIDTH; i++) {
				str = utils::String::AppendFormat(str, "%02X", data_[i]);
			}
			return str;
		}
		char At(int32_t i) const {
			return data_.at(i);
		}
		std::string GetData() const {
			return data_;
		}
	private:
		enum {
			WIDTH = bits / 8
		};
		std::string data_;
	};

	template <std::size_t Bits>
	inline bool operator == (const base_uint<Bits> &a, const base_uint<Bits> &b) {
		return a.GetData() == b.GetData();
	}

	template <std::size_t Bits>
	inline bool operator != (const base_uint<Bits> &a, const base_uint<Bits> &b) {
		return !(a == b);
	};

	template <std::size_t Bits>
	inline bool operator < (const base_uint<Bits> &a, const base_uint<Bits> &b) {
		if (a.GetData().size() < b.GetData().size()) {
			return true;
		}
		else if (a.GetData().size() > b.GetData().size()) {
			return false;
		}
		else {
			return a.GetData() < b.GetData();
		}
	};
	typedef base_uint<256>  uint256;
	uint256 CryptoUint256(const std::string &input);
	int hex_to_decimal(char a);
	bool bigDivide(int64_t& result, int64_t A, int64_t B, int64_t C);
	bool bigDivide(uint64_t& result, uint64_t A, uint64_t B, uint64_t C);
	int64_t  bigDivide(int64_t A, int64_t B, int64_t C);
}

#endif
