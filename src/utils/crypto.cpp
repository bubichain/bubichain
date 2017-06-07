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

#include <openssl/err.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>

#include "lrucache.hpp"
#include "thread.h"
#include "crypto.h"
#include "strings.h"

namespace utils {

	static const char* kBase58Dictionary = "123456789AbCDEFGHJKLMNPQRSTuVWXYZaBcdefghijkmnopqrstUvwxyz";
	static const int8_t kBase58digits[] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, -1, -1, -1, -1, -1, -1,
		-1, 9, 34, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1,
		22, 23, 24, 25, 26, 52, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1,
		-1, 33, 10, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46,
		47, 48, 49, 50, 51, 27, 53, 54, 55, 56, 57, -1, -1, -1, -1, -1
	};

	std::string Char2Hex(std::string &blob) {
		std::string str;
		for (size_t i = 0; i < blob.size(); i++) {
			str += String::Format("%02X", blob.at(i));
		}
		return str;
	}

	Sha256::Sha256() {
		SHA256_Init(&ctx_);
	}

	Sha256::~Sha256() {

	}

	void Sha256::Update(const std::string &input) {
		SHA256_Update(&ctx_, input.c_str(), input.size());
	}

	void Sha256::Update(const void *buffer, size_t len) {
		SHA256_Update(&ctx_, buffer, len);
	}

	std::string Sha256::Final() {
		std::string final_str;
		final_str.resize(32);
		SHA256_Final((unsigned char *)final_str.c_str(), &ctx_);
		return final_str;
	}

	std::string Sha256::Crypto(const std::string &input) {
		std::string str_out = "";
		str_out.resize(32);
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, input.c_str(), input.size());
		SHA256_Final((unsigned char*)str_out.c_str(), &sha256);
		return str_out;
	}


	void Sha256::Crypto(const std::string &input, std::string &output) {
		output.resize(32);

		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, input.c_str(), input.size());
		SHA256_Final((unsigned char*)output.c_str(), &sha256);
	}



	void Sha256::Crypto(unsigned char* str, int len, unsigned char *buf) {
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, str, len);
		SHA256_Final(buf, &sha256);
	}

	std::string Base58::Encode(const std::string &str_in) {
		std::string strOut;
		int zeros = 0;
		std::size_t first_no_zero = 0;
		for (std::size_t i = 0; i < str_in.size() && str_in.at(i) == 0; i++) {
			first_no_zero = i;
			zeros++;
		}

		std::vector< unsigned char > b58((str_in.size()) * 138 / 100 + 1);
		for (std::size_t i = first_no_zero; i < str_in.size(); i++) {
			int carry = (unsigned int)(str_in.at(i) & 0xFF);
			for (std::vector< unsigned char >::reverse_iterator it = b58.rbegin(); it != b58.rend(); it++) {
				carry += 256 * (*it);
				*it = carry % 58;
				carry /= 58;
			}
		}

		std::vector< unsigned char >::iterator it = b58.begin();
		while (it != b58.end() && *it == 0)
			it++;
		strOut = "";
		strOut.reserve(zeros + (b58.end() - it));
		strOut.assign(zeros, '1');
		while (it != b58.end()) {
			strOut += kBase58Dictionary[*(it++)];
		}
		return strOut;
	}

	std::string Base58::Decode(const std::string &strIn) {
		std::string out = "";
		Decode(strIn, out);
		return out;
	}

	int Base58::Decode(const std::string &strIn, std::string &strout) {
		std::size_t nZeros = 0;
		for (; nZeros < strIn.size() && strIn.at(nZeros) == kBase58Dictionary[0]; nZeros++);
		std::size_t left_size = strIn.size() - nZeros;
		std::size_t new_size = std::size_t(left_size * log2(58.0) / 8 + 2);
		std::string tmp_str(new_size, 0);
		int carry = 0;
		for (size_t i = nZeros; i < strIn.size(); i++) {
			carry = (unsigned char)kBase58digits[strIn[i]];
			for (int j = new_size - 1; j >= 0; j--) {
				int tmp = (unsigned char)tmp_str[j] * 58 + carry;
				tmp_str[j] = (unsigned char)(tmp % 256);
				carry = tmp / 256;
			}
		}
		strout.clear();
		for (size_t i = 0; i < nZeros; i++)
			strout.push_back((unsigned char)0);
		size_t k = 0;
		for (; k < tmp_str.size() && tmp_str[k] == 0; k++);
		for (; k < tmp_str.size(); k++)
			strout.push_back(tmp_str[k]);
		return nZeros + tmp_str.size() - k;
	}

	int Base58::Decode_old(const std::string &strIn, std::string &strOut) {
		strOut.clear();
		BIGNUM bn, bnchar;
		BIGNUM bn58, bn0;
		BN_CTX *ctx = BN_CTX_new();
		if (NULL == ctx)
			return 0;

		std::size_t nZeros = 0;
		for (nZeros = 0; nZeros < strIn.size(); nZeros++) {
			//58 base ，0
			if (strIn.at(nZeros) != kBase58Dictionary[0]) {
				break;
			}
		}
		std::string str_trim = strIn.substr(nZeros, strIn.size() - nZeros);


		BN_init(&bn58);
		BN_init(&bn0);
		BN_init(&bn);
		BN_init(&bnchar);

		BN_set_word(&bn58, 58);
		BN_zero(&bn0);
		for (std::size_t i = 0; i < str_trim.size(); i++) {
			//c = *p;
			unsigned char c = str_trim.at(i);
			if (c & 0x80) {
				if (NULL != ctx)
					BN_CTX_free(ctx);
				return 0;
			}

			if (-1 == kBase58digits[c]) {
				if (NULL != ctx)
					BN_CTX_free(ctx);
				return 0;
			}


			BN_set_word(&bnchar, kBase58digits[c]);

			if (!BN_mul(&bn, &bn, &bn58, ctx)) {
				if (NULL != ctx)
					BN_CTX_free(ctx);
				return 0;
			}

			BN_add(&bn, &bn, &bnchar);
		}
		int cb = BN_num_bytes(&bn);
		unsigned char * out = new unsigned char[cb];
		BN_bn2bin(&bn, out);
		BN_CTX_free(ctx);
		for (std::size_t i = 0; i < nZeros; i++) {
			strOut.push_back((char)0);
		}
		for (int i = 0; i < cb; i++)
			strOut.push_back(out[i]);
		delete[]out;

		BN_clear_free(&bn58);
		BN_clear_free(&bn0);
		BN_clear_free(&bn);
		BN_clear_free(&bnchar);
		return cb + nZeros;
	}

	//////////////////////////////////MD5//////////////////////////////////////

#ifndef HAVE_OPENSSL

#define F(x, y, z)   ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)   ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)   ((x) ^ (y) ^ (z))
#define I(x, y, z)   ((y) ^ ((x) | ~(z)))
#define STEP(f, a, b, c, d, x, t, s) \
		(a) += f((b), (c), (d)) + (x) + (t); \
		(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
		(a) += (b);

#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
			(*(MD5_u32 *)&ptr[(n) * 4])
#define GET(n) \
			SET(n)
#else
#define SET(n) \
			(ctx->block[(n)] = \
			(MD5_u32)ptr[(n) * 4] | \
			((MD5_u32)ptr[(n) * 4 + 1] << 8) | \
			((MD5_u32)ptr[(n) * 4 + 2] << 16) | \
			((MD5_u32)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
			(ctx->block[(n)])
#endif

	typedef unsigned int MD5_u32;

	typedef struct {
		MD5_u32 lo, hi;
		MD5_u32 a, b, c, d;
		unsigned char buffer[64];
		MD5_u32 block[16];
	} MD5_CTX;

	static void MD5_Init(MD5_CTX *ctx);
	static void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
	static void MD5_Final(unsigned char *result, MD5_CTX *ctx);

	static const void *body(MD5_CTX *ctx, const void *data, unsigned long size) {
		const unsigned char *ptr;
		MD5_u32 a, b, c, d;
		MD5_u32 saved_a, saved_b, saved_c, saved_d;

		ptr = (const unsigned char*)data;

		a = ctx->a;
		b = ctx->b;
		c = ctx->c;
		d = ctx->d;

		do {
			saved_a = a;
			saved_b = b;
			saved_c = c;
			saved_d = d;

			STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
				STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
				STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
				STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
				STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
				STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
				STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
				STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
				STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
				STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
				STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
				STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
				STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
				STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
				STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
				STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)
				STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
				STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
				STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
				STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
				STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
				STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
				STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
				STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
				STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
				STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
				STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
				STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
				STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
				STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
				STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
				STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)
				STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
				STEP(H, d, a, b, c, GET(8), 0x8771f681, 11)
				STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
				STEP(H, b, c, d, a, GET(14), 0xfde5380c, 23)
				STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
				STEP(H, d, a, b, c, GET(4), 0x4bdecfa9, 11)
				STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
				STEP(H, b, c, d, a, GET(10), 0xbebfbc70, 23)
				STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
				STEP(H, d, a, b, c, GET(0), 0xeaa127fa, 11)
				STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
				STEP(H, b, c, d, a, GET(6), 0x04881d05, 23)
				STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
				STEP(H, d, a, b, c, GET(12), 0xe6db99e5, 11)
				STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
				STEP(H, b, c, d, a, GET(2), 0xc4ac5665, 23)
				STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
				STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
				STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
				STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
				STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
				STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
				STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
				STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
				STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
				STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
				STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
				STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
				STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
				STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
				STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
				STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

				a += saved_a;
			b += saved_b;
			c += saved_c;
			d += saved_d;

			ptr += 64;
		} while (size -= 64);

		ctx->a = a;
		ctx->b = b;
		ctx->c = c;
		ctx->d = d;

		return ptr;
	}

	void MD5_Init(MD5_CTX *ctx) {
		ctx->a = 0x67452301;
		ctx->b = 0xefcdab89;
		ctx->c = 0x98badcfe;
		ctx->d = 0x10325476;

		ctx->lo = 0;
		ctx->hi = 0;
	}

	void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size) {
		MD5_u32 saved_lo;
		unsigned long used, free;

		saved_lo = ctx->lo;
		if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
			ctx->hi++;
		ctx->hi += size >> 29;
		used = saved_lo & 0x3f;

		if (used) {
			free = 64 - used;
			if (size < free) {
				memcpy(&ctx->buffer[used], data, size);
				return;
			}

			memcpy(&ctx->buffer[used], data, free);
			data = (unsigned char *)data + free;
			size -= free;
			body(ctx, ctx->buffer, 64);
		}

		if (size >= 64) {
			data = body(ctx, data, size & ~(unsigned long)0x3f);
			size &= 0x3f;
		}

		memcpy(ctx->buffer, data, size);
	}

	void MD5_Final(unsigned char *result, MD5_CTX *ctx) {
		unsigned long used, free;
		used = ctx->lo & 0x3f;
		ctx->buffer[used++] = 0x80;
		free = 64 - used;

		if (free < 8) {
			memset(&ctx->buffer[used], 0, free);
			body(ctx, ctx->buffer, 64);
			used = 0;
			free = 64;
		}

		memset(&ctx->buffer[used], 0, free - 8);

		ctx->lo <<= 3;
		ctx->buffer[56] = ctx->lo;
		ctx->buffer[57] = ctx->lo >> 8;
		ctx->buffer[58] = ctx->lo >> 16;
		ctx->buffer[59] = ctx->lo >> 24;
		ctx->buffer[60] = ctx->hi;
		ctx->buffer[61] = ctx->hi >> 8;
		ctx->buffer[62] = ctx->hi >> 16;
		ctx->buffer[63] = ctx->hi >> 24;
		body(ctx, ctx->buffer, 64);
		result[0] = ctx->a;
		result[1] = ctx->a >> 8;
		result[2] = ctx->a >> 16;
		result[3] = ctx->a >> 24;
		result[4] = ctx->b;
		result[5] = ctx->b >> 8;
		result[6] = ctx->b >> 16;
		result[7] = ctx->b >> 24;
		result[8] = ctx->c;
		result[9] = ctx->c >> 8;
		result[10] = ctx->c >> 16;
		result[11] = ctx->c >> 24;
		result[12] = ctx->d;
		result[13] = ctx->d >> 8;
		result[14] = ctx->d >> 16;
		result[15] = ctx->d >> 24;
		memset(ctx, 0, sizeof(*ctx));
	}
#else
#include <openssl/md5.h>
#endif

	void MD5::md5bin(const void* dat, size_t len, unsigned char out[16]) {
		MD5_CTX c;
		MD5_Init(&c);
		MD5_Update(&c, dat, len);
		MD5_Final(out, &c);
	}

	char MD5::hb2hex(unsigned char hb) {
		hb = hb & 0xF;
		return hb < 10 ? '0' + hb : hb - 10 + 'a';
	}

	std::string MD5::GenerateMd5File(const char* filename) {
		std::FILE* file = std::fopen(filename, "rb");
		std::string res = "";
		if (NULL == file) {
			res = "";
		}
		else {
			res = GenerateMd5File(file);
			std::fclose(file);
		}
		return res;
	}

	std::string MD5::GenerateMd5File(std::FILE* file) {
		MD5_CTX c;
		MD5_Init(&c);

		char buff[BUFSIZ];
		unsigned char out[16];
		size_t len = 0;
		while ((len = std::fread(buff, sizeof(char), BUFSIZ, file)) > 0) {
			MD5_Update(&c, buff, len);
		}
		MD5_Final(out, &c);

		std::string res = "";
		for (size_t i = 0; i < 16; ++i) {
			res.push_back(hb2hex(out[i] >> 4));
			res.push_back(hb2hex(out[i]));
		}
		return res;
	}

	std::string MD5::GenerateMD5(const void* dat, size_t len) {
		unsigned char out[16];
		md5bin(dat, len, out);
		std::string res = "";
		for (size_t i = 0; i < 16; ++i) {
			res.push_back(hb2hex(out[i] >> 4));
			res.push_back(hb2hex(out[i]));
		}
		return res;
	}

	std::string MD5::GenerateMD5(std::string dat) {
		return GenerateMD5(dat.c_str(), dat.length());
	}

	std::string MD5::GenerateMD5Sum6(const void* dat, size_t len) {
		static const char* tbl = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		unsigned char out[16];
		md5bin(dat, len, out);
		std::string res = "";
		for (size_t i = 0; i < 6; ++i) {
			res.push_back(tbl[out[i] % 62]);
		}
		return res;
	}

	std::string MD5::GenerateMD5Sum6(std::string dat) {
		return GenerateMD5Sum6(dat.c_str(), dat.length());
	}

	/*
	* 32-bit integer manipulation macros (big endian)
	*/
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
	{                                                       \
    (n) = ( (uint32_t) (b)[(i)    ] << 24 )        \
        | ( (uint32_t) (b)[(i) + 1] << 16 )        \
        | ( (uint32_t) (b)[(i) + 2] <<  8 )        \
        | ( (uint32_t) (b)[(i) + 3]       );       \
	}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
		{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
		}
#endif

	Sm3::Sm3() {
		sm3_starts(&ctx_);
	}

	Sm3::~Sm3() {}

	void Sm3::Update(const std::string &input) {
		sm3_update(&ctx_, (unsigned char*)input.c_str(), input.size());
	}

	void Sm3::Update(const void *buffer, size_t len) {
		sm3_update(&ctx_, (unsigned char*)buffer, len);
	}

	std::string Sm3::Final() {
		std::string final_str;
		final_str.resize(32);
		sm3_finish(&ctx_, (unsigned char *)final_str.c_str());
		return final_str;
	}

	std::string Sm3::Crypto(const std::string &input) {
		std::string str_out = "";
		str_out.resize(32);

		sm3_context sm3;
		sm3_starts(&sm3);
		sm3_update(&sm3, (unsigned char*)input.c_str(), input.size());
		sm3_finish(&sm3, (unsigned char*)str_out.c_str());

		return str_out;
	}


	void Sm3::Crypto(const std::string &input, std::string &output) {
		output.resize(32);

		sm3_context sm3;
		sm3_starts(&sm3);
		sm3_update(&sm3, (unsigned char*)input.c_str(), input.size());
		sm3_finish(&sm3, (unsigned char*)output.c_str());
	}



	void Sm3::Crypto(unsigned char* str, int len, unsigned char *buf) {
		sm3_context sm3;
		sm3_starts(&sm3);
		sm3_update(&sm3, str, len);
		sm3_finish(&sm3, buf);
	}

	/*
	* SM3 context setup
	*/
	void Sm3::sm3_starts(sm3_context *ctx) {
		ctx->total[0] = 0;
		ctx->total[1] = 0;

		ctx->state[0] = 0x7380166F;
		ctx->state[1] = 0x4914B2B9;
		ctx->state[2] = 0x172442D7;
		ctx->state[3] = 0xDA8A0600;
		ctx->state[4] = 0xA96F30BC;
		ctx->state[5] = 0x163138AA;
		ctx->state[6] = 0xE38DEE4D;
		ctx->state[7] = 0xB0FB0E4E;

	}

	void Sm3::sm3_process(sm3_context *ctx, unsigned char data[64]) {
		uint32_t SS1, SS2, TT1, TT2, W[68], W1[64];
		uint32_t A, B, C, D, E, F, G, H;
		uint32_t T[64];
		uint32_t Temp1, Temp2, Temp3, Temp4, Temp5;
		int j;
	
		for (j = 0; j < 16; j++)
			T[j] = 0x79CC4519;
		for (j = 16; j < 64; j++)
			T[j] = 0x7A879D8A;

		GET_ULONG_BE(W[0], data, 0);  //W[0]=data[0] data[1] data[2] data[3]
		GET_ULONG_BE(W[1], data, 4);
		GET_ULONG_BE(W[2], data, 8);
		GET_ULONG_BE(W[3], data, 12);
		GET_ULONG_BE(W[4], data, 16);
		GET_ULONG_BE(W[5], data, 20);
		GET_ULONG_BE(W[6], data, 24);
		GET_ULONG_BE(W[7], data, 28);
		GET_ULONG_BE(W[8], data, 32);
		GET_ULONG_BE(W[9], data, 36);
		GET_ULONG_BE(W[10], data, 40);
		GET_ULONG_BE(W[11], data, 44);
		GET_ULONG_BE(W[12], data, 48);
		GET_ULONG_BE(W[13], data, 52);
		GET_ULONG_BE(W[14], data, 56);
		GET_ULONG_BE(W[15], data, 60);
		
#define FF0(x,y,z) ( (x) ^ (y) ^ (z)) 
#define FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define GG0(x,y,z) ( (x) ^ (y) ^ (z)) 
#define GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )


#define  SHL(x,n) (((x) & 0xFFFFFFFF) << n)
#define ROTL(x,n) (SHL((x),n) | ((x) >> (32 - n)))

#define P0(x) ((x) ^  ROTL((x),9) ^ ROTL((x),17)) 
#define P1(x) ((x) ^  ROTL((x),15) ^ ROTL((x),23)) 

		for (j = 16; j < 68; j++) {
			//W[j] = P1( W[j-16] ^ W[j-9] ^ ROTL(W[j-3],15)) ^ ROTL(W[j - 13],7 ) ^ W[j-6];
			//Why thd release's result is different with the debug's ?
			//Below is okay. Interesting, Perhaps VC6 has a bug of Optimizaiton.

			Temp1 = W[j - 16] ^ W[j - 9];
			Temp2 = ROTL(W[j - 3], 15);
			Temp3 = Temp1 ^ Temp2;
			Temp4 = P1(Temp3);
			Temp5 = ROTL(W[j - 13], 7) ^ W[j - 6];
			W[j] = Temp4 ^ Temp5;
		}
		for (j = 0; j < 64; j++) {
			W1[j] = W[j] ^ W[j + 4];
		}
		A = ctx->state[0];
		B = ctx->state[1];
		C = ctx->state[2];
		D = ctx->state[3];
		E = ctx->state[4];
		F = ctx->state[5];
		G = ctx->state[6];
		H = ctx->state[7];
		
		for (j = 0; j < 16; j++) {
			SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)), 7);
			SS2 = SS1 ^ ROTL(A, 12);
			TT1 = FF0(A, B, C) + D + SS2 + W1[j];
			TT2 = GG0(E, F, G) + H + SS1 + W[j];
			D = C;
			C = ROTL(B, 9);
			B = A;
			A = TT1;
			H = G;
			G = ROTL(F, 19);
			F = E;
			E = P0(TT2);
		}

		for (j = 16; j < 64; j++) {
			SS1 = ROTL((ROTL(A, 12) + E + ROTL(T[j], j)), 7);
			SS2 = SS1 ^ ROTL(A, 12);
			TT1 = FF1(A, B, C) + D + SS2 + W1[j];
			TT2 = GG1(E, F, G) + H + SS1 + W[j];
			D = C;
			C = ROTL(B, 9);
			B = A;
			A = TT1;
			H = G;
			G = ROTL(F, 19);
			F = E;
			E = P0(TT2);  
		}

		ctx->state[0] ^= A;
		ctx->state[1] ^= B;
		ctx->state[2] ^= C;
		ctx->state[3] ^= D;
		ctx->state[4] ^= E;
		ctx->state[5] ^= F;
		ctx->state[6] ^= G;
		ctx->state[7] ^= H;
	}

	/*
	* SM3 process buffer
	*/
	void Sm3::sm3_update(sm3_context *ctx, unsigned char *input, int ilen) {
		int fill;
		uint32_t left;

		if (ilen <= 0)
			return;

		left = ctx->total[0] & 0x3F;
		fill = 64 - left;

		ctx->total[0] += ilen;
		ctx->total[0] &= 0xFFFFFFFF;

		if (ctx->total[0] < (uint32_t)ilen)
			ctx->total[1]++;

		if (left && ilen >= fill) {
			memcpy((void *)(ctx->buffer + left),
				(void *)input, fill);
			sm3_process(ctx, ctx->buffer);
			input += fill;
			ilen -= fill;
			left = 0;
		}

		while (ilen >= 64) {
			sm3_process(ctx, input);
			input += 64;
			ilen -= 64;
		}

		if (ilen > 0) {
			memcpy((void *)(ctx->buffer + left),
				(void *)input, ilen);
		}
	}

	static const unsigned char sm3_padding[64] =
	{
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	/*
	* SM3 final digest
	*/
	void Sm3::sm3_finish(sm3_context *ctx, unsigned char output[32]) {
		uint32_t last, padn;
		uint32_t high, low;
		unsigned char msglen[8];

		high = (ctx->total[0] >> 29)
			| (ctx->total[1] << 3);
		low = (ctx->total[0] << 3);

		PUT_ULONG_BE(high, msglen, 0);
		PUT_ULONG_BE(low, msglen, 4);

		last = ctx->total[0] & 0x3F;
		padn = (last < 56) ? (56 - last) : (120 - last);

		sm3_update(ctx, (unsigned char *)sm3_padding, padn);
		sm3_update(ctx, msglen, 8);

		PUT_ULONG_BE(ctx->state[0], output, 0);
		PUT_ULONG_BE(ctx->state[1], output, 4);
		PUT_ULONG_BE(ctx->state[2], output, 8);
		PUT_ULONG_BE(ctx->state[3], output, 12);
		PUT_ULONG_BE(ctx->state[4], output, 16);
		PUT_ULONG_BE(ctx->state[5], output, 20);
		PUT_ULONG_BE(ctx->state[6], output, 24);
		PUT_ULONG_BE(ctx->state[7], output, 28);
	}

	void Sm3::sm3(unsigned char *input, int ilen,
		unsigned char output[32]) {
		sm3_context ctx;

		sm3_starts(&ctx);
		sm3_update(&ctx, input, ilen);
		sm3_finish(&ctx, output);

		memset(&ctx, 0, sizeof(sm3_context));
	}

	/*
	* output = SM3( file contents )
	*/
	int Sm3::sm3_file(char *path, unsigned char output[32]) {
		FILE *f;
		size_t n;
		sm3_context ctx;
		unsigned char buf[1024];

		if ((f = fopen(path, "rb")) == NULL)
			return(1);

		sm3_starts(&ctx);

		while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
			sm3_update(&ctx, buf, (int)n);

		sm3_finish(&ctx, output);

		memset(&ctx, 0, sizeof(sm3_context));

		if (ferror(f) != 0) {
			fclose(f);
			return(2);
		}

		fclose(f);
		return(0);
	}

	/*
	* SM3 HMAC context setup
	*/
	void Sm3::sm3_hmac_starts(sm3_context *ctx, unsigned char *key, int keylen) {
		int i;
		unsigned char sum[32];

		if (keylen > 64) {
			sm3(key, keylen, sum);
			keylen = 32;
			//keylen = ( is224 ) ? 28 : 32;
			key = sum;
		}

		memset(ctx->ipad, 0x36, 64);
		memset(ctx->opad, 0x5C, 64);

		for (i = 0; i < keylen; i++) {
			ctx->ipad[i] = (unsigned char)(ctx->ipad[i] ^ key[i]);
			ctx->opad[i] = (unsigned char)(ctx->opad[i] ^ key[i]);
		}

		sm3_starts(ctx);
		sm3_update(ctx, ctx->ipad, 64);

		memset(sum, 0, sizeof(sum));
	}

	/*
	* SM3 HMAC process buffer
	*/
	void Sm3::sm3_hmac_update(sm3_context *ctx, unsigned char *input, int ilen) {
		sm3_update(ctx, input, ilen);
	}

	/*
	* SM3 HMAC final digest
	*/
	void Sm3::sm3_hmac_finish(sm3_context *ctx, unsigned char output[32]) {
		int hlen;
		unsigned char tmpbuf[32];

		//is224 = ctx->is224;
		hlen = 32;

		sm3_finish(ctx, tmpbuf);
		sm3_starts(ctx);
		sm3_update(ctx, ctx->opad, 64);
		sm3_update(ctx, tmpbuf, hlen);
		sm3_finish(ctx, output);

		memset(tmpbuf, 0, sizeof(tmpbuf));
	}

	/*
	* output = HMAC-SM#( hmac key, input buffer )
	*/
	void Sm3::sm3_hmac(unsigned char *key, int keylen,
		unsigned char *input, int ilen,
		unsigned char output[32]) {
		sm3_context ctx;

		sm3_hmac_starts(&ctx, key, keylen);
		sm3_hmac_update(&ctx, input, ilen);
		sm3_hmac_finish(&ctx, output);

		memset(&ctx, 0, sizeof(sm3_context));
	}
}