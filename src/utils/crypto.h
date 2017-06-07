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

#ifndef UTILS_CRYPTO_H
#define UTILS_CRYPTO_H
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>

namespace utils {
	//typedef std::vector<unsigned char>  std::string;
	std::string Char2Hex(std::string &blob);
	class Base58 {
	public:
		Base58() {}
		~Base58() {}
		static std::string Encode(const std::string &buff);
		static int Decode(const std::string &strIn, std::string &out);
		static int Decode_old(const std::string &strIn, std::string &out);
		static std::string Decode(const std::string &strIn);
	};
	class Hash {
	public:
		Hash() {};
		~Hash() {};
		virtual void Update(const std::string &input) = 0;
		virtual void Update(const void *buffer, size_t len) = 0;
		virtual std::string Final() = 0;
	};
	class Sha256 : public Hash {	
	public:
		Sha256();
		~Sha256();
		void Update(const std::string &input);
		void Update(const void *buffer, size_t len);
		std::string Final();
		static std::string CryptoBase58(const std::string &input) {
			return utils::Base58::Encode(Crypto(input));
		}
		static std::string Crypto(const std::string &input);
		static void Crypto(unsigned char* str, int len, unsigned char *buf);
		static void Crypto(const std::string &input, std::string &str);
	public:
		static const int SIZE = 32;
	private:
		SHA256_CTX ctx_;
	};
	class MD5 {
	public:
		static std::string GenerateMd5File(const char* filename);
		static std::string GenerateMd5File(std::FILE* file);
		static std::string GenerateMD5(const void* dat, size_t len);
		static std::string GenerateMD5(std::string dat);
		static std::string GenerateMD5Sum6(const void* dat, size_t len);
		static std::string GenerateMD5Sum6(std::string dat);
	private:
		static char hb2hex(unsigned char hb);
		static void md5bin(const void* dat, size_t len, unsigned char out[16]);
	};

	class Sm3 : public Hash {
		typedef struct {
			unsigned long total[2];     /*!< number of bytes processed 8 */
			unsigned long state[8];     /*!< intermediate digest state  */
			unsigned char buffer[64];   /*!< data block being processed */
			unsigned char ipad[64];     /*!< HMAC: inner padding        */
			unsigned char opad[64];     /*!< HMAC: outer padding        */
		}
		sm3_context;
		sm3_context ctx_;
	public:
		Sm3();
		~Sm3();
		void Update(const std::string &input);
		void Update(const void *buffer, size_t len);
		std::string Final();
		static std::string CryptoBase58(const std::string &input) {
			return utils::Base58::Encode(Crypto(input));
		}
		static std::string Crypto(const std::string &input);
		static void Crypto(unsigned char* str, int len, unsigned char *buf);
		static void Crypto(const std::string &input, std::string &str);
	private:
		static void sm3_starts(sm3_context *ctx);
		/**
		* \brief          SM3 process buffer
		*
		* \param ctx      SM3 context
		* \param input    buffer holding the  data
		* \param ilen     length of the input data
		*/
		static void sm3_update(sm3_context *ctx, unsigned char *input, int ilen);
		/**
		* \brief          SM3 final digest
		*
		* \param ctx      SM3 context
		*/
		static void sm3_finish(sm3_context *ctx, unsigned char output[32]);
		/**
		* \brief          Output = SM3( input buffer )
		*
		* \param input    buffer holding the  data
		* \param ilen     length of the input data
		* \param output   SM3 checksum result
		*/
		static void sm3(unsigned char *input, int ilen,
			unsigned char output[32]);
		static void sm3_process(sm3_context *ctx, unsigned char data[64]);
		/**
		* \brief          Output = SM3( file contents )
		*
		* \param path     input file name
		* \param output   SM3 checksum result
		*
		* \return         0 if successful, 1 if fopen failed,
		*                 or 2 if fread failed
		*/
		static int sm3_file(char *path, unsigned char output[32]);
		/**
		* \brief          SM3 HMAC context setup
		*
		* \param ctx      HMAC context to be initialized
		* \param key      HMAC secret key
		* \param keylen   length of the HMAC key
		*/
		static void sm3_hmac_starts(sm3_context *ctx, unsigned char *key, int keylen);
		/**
		* \brief          SM3 HMAC process buffer
		*
		* \param ctx      HMAC context
		* \param input    buffer holding the  data
		* \param ilen     length of the input data
		*/
		static void sm3_hmac_update(sm3_context *ctx, unsigned char *input, int ilen);
		/**
		* \brief          SM3 HMAC final digest
		*
		* \param ctx      HMAC context
		* \param output   SM3 HMAC checksum result
		*/
		static void sm3_hmac_finish(sm3_context *ctx, unsigned char output[32]);
		/**
		* \brief          Output = HMAC-SM3( hmac key, input buffer )
		*
		* \param key      HMAC secret key
		* \param keylen   length of the HMAC key
		* \param input    buffer holding the  data
		* \param ilen     length of the input data
		* \param output   HMAC-SM3 result
		*/
		static void sm3_hmac(unsigned char *key, int keylen,
			unsigned char *input, int ilen,
			unsigned char output[32]);
	};
}
#endif //CRYPTO_H

