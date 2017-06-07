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

#ifndef ECC_SM2_H_
#define ECC_SM2_H_

#include <memory>
#include <limits.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <ed25519-donna/ed25519.h>

#ifdef WIN32
#include <winbase.h>
#endif

namespace bubi{
	typedef unsigned char sm2_public_key[65];
	class ecc_sm2 {
		public:
			~ecc_sm2 (){
				if (group_ != NULL)
					EC_GROUP_free(group_);
			}
			typedef std::shared_ptr<ecc_sm2> pointer;			
			static ecc_sm2::pointer Instance ();
			int generate_keys (ed25519_secret_key pri_key, ed25519_public_key pub_key);
			int get_publickey_from_private_key (ed25519_secret_key pri_key, ed25519_public_key pub_key);
			int sign (const unsigned char *m, int mlen, ed25519_secret_key pri_key, ed25519_public_key pub_key, unsigned char *sig, int *siglen);
			ECDSA_SIG *do_sign (const unsigned char* m, int mlen, EC_KEY *eckey);
			// true means verify OK
			bool verify (const unsigned char *m, int mlen, const unsigned char *sig, int sig_len, unsigned char *raw_pub_key);
			int do_verify (const unsigned char *m, int m_len, ECDSA_SIG *sig, unsigned char *raw_pub_key);
		private:
			ecc_sm2 ();
			void Initialize ();
	    private:
			static ecc_sm2::pointer ecc_;
			EC_GROUP* group_;
	};

};

#endif
