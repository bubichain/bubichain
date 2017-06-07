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

#include <openssl/ripemd.h>
#include <utils/logger.h>
#include <utils/crypto.h>
#include <utils/strings.h>
#include "general.h"
#include "private_key.h"

namespace bubi {
	PublicKey::PublicKey() :valid_(false), type_(UNKNOWNSIG) {}

	PublicKey::~PublicKey() {}

	PublicKey::PublicKey(const std::string &base58_pub_key) {
		std::string buff = utils::Base58::Decode(base58_pub_key);
		if (buff.size() == 32) {
			type_ = ED25519SIG;
			memcpy(raw_pub_key_.ed25519_raw_pub_key_, buff.c_str(), 32);
			valid_ = true;
		}
		else {
			type_ = UNKNOWNSIG;
			valid_ = false;
		}
	}

	bool PublicKey::IsValid() const {
		return valid_;
	}

	SignatureType PublicKey::GetSignType() { 
		return type_; 
	}

	std::string PublicKey::GetBase58Address() const {
		std::string str_result;
		str_result.push_back((char)0XE6);
		str_result.push_back((char)0X9A);
		str_result.push_back((char)0X73);
		str_result.push_back((char)0XFF);

		str_result.push_back((int)type_);

		unsigned char md[20];
		if (type_ == ED25519SIG) {
			RIPEMD160(raw_pub_key_.ed25519_raw_pub_key_, 32, md);
		}
		
		str_result.append((const char *)md, 20);

		std::string hash1, hash2;
		if (type_ == ED25519SIG) {
			hash1 = utils::Sha256::Crypto(str_result);
			hash2 = utils::Sha256::Crypto(hash1);
		}
		else {
			hash1 = utils::Sm3::Crypto(str_result);
			hash2 = utils::Sm3::Crypto(hash1);
		}
		str_result.append(hash2.c_str(), 4);
		return utils::Base58::Encode(str_result);
	}

	std::string PublicKey::GetPublicKey() const {
		std::string key;
		if (type_ == ED25519SIG) {
			key.append((const char *)raw_pub_key_.ed25519_raw_pub_key_, 32);
		}
		else {
			LOG_ERROR("Signature type is not set");
		}
		return key;
	}

	std::string PublicKey::GetBase58PublicKey() const {
		std::string key;
		if (type_ == ED25519SIG) {
			key.append((const char *)raw_pub_key_.ed25519_raw_pub_key_, 32);
		}
		else {
			LOG_ERROR("Signature type is not set");
		}
		return utils::Base58::Encode(key);
	}

	bool PublicKey::Verify(const std::string &data, const std::string &signature, const std::string &public_key_base58) {
		std::string raw_pub_key = utils::Base58::Decode(public_key_base58);
		if (raw_pub_key.size() == 32) {
			return ed25519_sign_open((unsigned char *)data.c_str(), data.size(), (unsigned char *)raw_pub_key.c_str(), (unsigned char *)signature.c_str()) == 0;
		}
		else {
			LOG_ERROR("Unknown signature type");
			return false;
		}
	}
	bool PublicKey::IsAddressValid(const std::string &address) {
		std::string str = utils::Base58::Decode(address);
		if (str.length() != 29) {
			return false;
		}
		return true;
	}

	PrivateKey::PrivateKey(SignatureType type) {
		type_ = type;
		if (type_ == ED25519SIG) {
			utils::MutexGuard guard_(lock_);
			// ed25519;
			ed25519_randombytes_unsafe(raw_priv_key_, 32);
			ed25519_publickey(raw_priv_key_, pub_key_.raw_pub_key_.ed25519_raw_pub_key_);
			pub_key_.type_ = type;
		}
		else {
			LOG_ERROR("Unknown signature type");
			pub_key_.type_ = type_ = UNKNOWNSIG;
			return;
		}
		pub_key_.valid_ = true;
		valid_ = true;
	}

	PrivateKey::~PrivateKey() {}

	bool PrivateKey::IsValid() const {
		return valid_;
	}

	std::string PrivateKey::GetRawPrivateKey() {
		std::string s;
		for (int i = 0; i < 32; i++) {
			s.push_back(raw_priv_key_[i]);
		}
		return utils::String::BinToHexString(s);
	}

	SignatureType PrivateKey::GetSignType() {
		return type_;
	}

	PrivateKey::PrivateKey(const std::string &base58_private_key) {
		From(base58_private_key);
	}

	bool PrivateKey::From(const std::string &base58_private_key) {
		valid_ = false;
		do {
			std::string serialized_data;
			utils::Base58::Decode(base58_private_key, serialized_data);
			if (serialized_data.size() != 41) {
				LOG_ERROR("Make new key from %s failed!", base58_private_key.c_str());
				break;
			}
			type_ = (SignatureType)(unsigned char)serialized_data[3];
			pub_key_.type_ = type_;

			for (size_t i = 0; i < 32; i++) {
				raw_priv_key_[i] = serialized_data[i + 4];
			}

			if (type_ == ED25519SIG) {
				ed25519_publickey(raw_priv_key_, pub_key_.raw_pub_key_.ed25519_raw_pub_key_);
			}
			else {
				LOG_ERROR("Unknown signature type");
				pub_key_.type_ = type_ = UNKNOWNSIG;
				valid_ = false;
				return valid_;
			}
			pub_key_.valid_ = true;
			valid_ = true;

		} while (false);

		return valid_;
	}

	std::string PrivateKey::Sign(const std::string &input) const {
		unsigned char sig[10240];
		int sig_len;

		if (type_ == ED25519SIG) {
			/*	ed25519_signature sig;*/
			ed25519_sign((unsigned char *)input.c_str(), input.size(), raw_priv_key_, pub_key_.raw_pub_key_.ed25519_raw_pub_key_, sig);
			sig_len = 64;
		}
		else {
			return "";
		}
		std::string output;
		output.append((const char *)sig, sig_len);
		return output;
	}

	std::string PrivateKey::GetBase58PrivateKey() const {
		std::string nvec;
		nvec.push_back((char)0XDA);
		nvec.push_back((char)0X37);
		nvec.push_back((char)0X9F);

		nvec.push_back((int)type_);
		nvec.append((const char *)raw_priv_key_, sizeof(raw_priv_key_));
		nvec.push_back(0X00);
		std::string hash1, hash2;
		if (type_ == ED25519SIG) {
			hash1 = utils::Sha256::Crypto(nvec);
			hash2 = utils::Sha256::Crypto(hash1);
		}
		else {
			hash1 = utils::Sm3::Crypto(nvec);
			hash2 = utils::Sm3::Crypto(hash1);
		}

		nvec.push_back(hash2[0]);
		nvec.push_back(hash2[1]);
		nvec.push_back(hash2[2]);
		nvec.push_back(hash2[3]);

		return utils::Base58::Encode(nvec);
	}

	std::string PrivateKey::GetBase58Address() const {
		return pub_key_.GetBase58Address();
	}

	std::string PrivateKey::GetBase58PublicKey() const {
		return pub_key_.GetBase58PublicKey();
	}

	std::string PrivateKey::GetPublicKey() const {
		return pub_key_.GetPublicKey();
	}

	utils::Mutex PrivateKey::lock_;
}
