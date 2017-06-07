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

#ifndef PRIVATE_KEY_H_
#define PRIVATE_KEY_H_

#include "ecc_sm2.h"
#include <utils/headers.h>

namespace bubi {
	enum SignatureType {
		ED25519SIG = 1,
		ECCSM2SIG = 2,
		UNKNOWNSIG = 3
	};

	class PublicKey {
		DISALLOW_COPY_AND_ASSIGN(PublicKey);
		friend class PrivateKey;
	public:
		PublicKey();
		PublicKey(const std::string &base58_pub_key);
		~PublicKey();
		std::string GetBase58Address() const;
		std::string GetBase58PublicKey() const;
		std::string GetPublicKey() const;
		static bool Verify(const std::string &data, const std::string &signature, const std::string &public_key_base58);
		static bool IsAddressValid(const std::string &address);
		bool IsValid()const;
		SignatureType GetSignType();
	private:
		union {
			ed25519_public_key ed25519_raw_pub_key_;
			sm2_public_key sm2_raw_pub_key;
		} raw_pub_key_;
		bool valid_;
		SignatureType type_;
	};

	class PrivateKey {
		DISALLOW_COPY_AND_ASSIGN(PrivateKey);
	public:
		PrivateKey(SignatureType type);
		PrivateKey(const std::string &base58_private_key);
		~PrivateKey();
		bool From(const std::string &base58_private_key);
		std::string	Sign(const std::string &input) const;
		std::string GetBase58PrivateKey() const;
		std::string GetBase58Address() const;
		std::string GetBase58PublicKey() const;
		std::string GetPublicKey() const;
		bool IsValid() const;
		std::string GetRawPrivateKey();
		SignatureType GetSignType();
	private:
		ed25519_secret_key raw_priv_key_;
		bool valid_;
		SignatureType type_;
		PublicKey pub_key_;
		static utils::Mutex lock_;
	};
};

#endif
