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

#include "ecc_sm2.h"

#include <assert.h>
#include <utils/logger.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/ecdsa.h>
#include <openssl/ecdh.h>
#include <openssl/evp.h>

namespace bubi {

#define ABORT do { \
	fflush(stdout); \
	fprintf(stderr, "%s:%d: ABORT\n", __FILE__, __LINE__); \
	exit (1);	\
} while (0)

#define free_bn(x) do{ 	\
	if (x != NULL)		\
		BN_free (x);	\
}while (0)

#define free_ec_point(x) do{	\
	if (x != NULL)				\
		EC_POINT_free (x);		\
}while (0)	

	static const char *rnd_seed = "string to make the random number generator think it has entropy";

	ecc_sm2::pointer ecc_sm2::ecc_ = nullptr;

	ecc_sm2::pointer ecc_sm2::Instance() {
		if (ecc_ != nullptr)
			return ecc_;
		ecc_sm2* test = new ecc_sm2();
		ecc_.reset(test);
		return ecc_;
	}

	ecc_sm2::ecc_sm2() {
		CRYPTO_set_mem_debug_functions(0, 0, 0, 0, 0);
		CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);
		ERR_load_crypto_strings();
		RAND_seed(rnd_seed, sizeof rnd_seed);

		BN_CTX *ctx = NULL;
		BIGNUM *p = NULL, *a = NULL, *b = NULL;
		EC_POINT *P = NULL, *Q = NULL, *R = NULL;
		BIGNUM *x = NULL, *y = NULL, *z = NULL;
		group_ = NULL;

		ctx = BN_CTX_new();
		if (!ctx) ABORT;

		p = BN_new();
		a = BN_new();
		b = BN_new();
		if (!p || !a || !b) ABORT;
		group_ = EC_GROUP_new(EC_GFp_mont_method());
		if (!group_) ABORT;

		// sm2 ec parameters
		// p：8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3
		// a：787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498
		// b：63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A
		// xG 421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D
		// yG 0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2
		// n: 8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7

		if (!BN_hex2bn(&p, "8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3")) ABORT;
		if (1 != BN_is_prime_ex(p, BN_prime_checks, ctx, NULL)) ABORT;
		if (!BN_hex2bn(&a, "787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498")) ABORT;
		if (!BN_hex2bn(&b, "63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A")) ABORT;
		if (!EC_GROUP_set_curve_GFp(group_, p, a, b, ctx)) ABORT;

		P = EC_POINT_new(group_);
		Q = EC_POINT_new(group_);
		R = EC_POINT_new(group_);
		if (!P || !Q || !R) ABORT;

		x = BN_new();
		y = BN_new();
		z = BN_new();
		if (!x || !y || !z) ABORT;

		if (!BN_hex2bn(&x, "421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D")) ABORT;
		if (!EC_POINT_set_compressed_coordinates_GFp(group_, P, x, 0, ctx)) ABORT;
		if (!EC_POINT_is_on_curve(group_, P, ctx)) ABORT;
		if (!BN_hex2bn(&z, "8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7")) ABORT;
		if (!EC_GROUP_set_generator(group_, P, z, BN_value_one())) ABORT;

		if (!EC_POINT_get_affine_coordinates_GFp(group_, P, x, y, ctx)) ABORT;

		if (!BN_hex2bn(&z, "0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2")) ABORT;
		if (0 != BN_cmp(y, z)) ABORT;

		if (EC_GROUP_get_degree(group_) != 256) ABORT;

		if (!EC_GROUP_get_order(group_, z, ctx)) ABORT;

		if (!EC_GROUP_precompute_mult(group_, ctx)) ABORT;
		if (!EC_POINT_mul(group_, Q, z, NULL, NULL, ctx)) ABORT;
		if (!EC_POINT_is_at_infinity(group_, Q)) ABORT;

		//OK

	err:
		free_bn(p);
		free_bn(a);
		free_bn(b);
		free_bn(x);
		free_bn(y);
		free_bn(z);
		free_ec_point(P);
		free_ec_point(Q);
		free_ec_point(R);
		if (ctx != NULL)	BN_CTX_free(ctx);
	}


	int ecc_sm2::generate_keys(ed25519_secret_key pri_key, sm2_public_key pub_key) {
		assert(group_ != NULL);

		EC_KEY *ec_key = NULL;
		if ((ec_key = EC_KEY_new()) == NULL) {
			LOG_ERROR("Allocate ec key failed");
			return 1;
		}
		if (EC_KEY_set_group(ec_key, group_) == 0) {
			LOG_ERROR("Ec key set group failed");
			EC_KEY_free(ec_key);
			return 1;
		}
		/* create key */
		if (!EC_KEY_generate_key(ec_key)) {
			LOG_ERROR("Ec key generate key failed");
			EC_KEY_free(ec_key);
			return 1;
		}
		/* check key */
		if (!EC_KEY_check_key(ec_key)) {
			LOG_ERROR("Ec key check key failed");
			EC_KEY_free(ec_key);
			return 1;
		}

		const BIGNUM *bg_priv_key = EC_KEY_get0_private_key(ec_key);
		BN_bn2bin(bg_priv_key, pri_key);

		unsigned char *pbegin = pub_key;
		int pk_size = i2o_ECPublicKey(ec_key, &pbegin);
		assert(pk_size == 65);
		return 0;
	}

	int ecc_sm2::get_publickey_from_private_key(ed25519_secret_key pri_key, sm2_public_key pub_key) {
		assert(group_ != NULL);

		BIGNUM *private_key = NULL;
		EC_KEY *new_key = NULL;
		EC_POINT *public_key = NULL;
		BN_CTX *ctx = NULL;

		private_key = BN_new();
		new_key = EC_KEY_new();
		public_key = EC_POINT_new(group_);
		ctx = BN_CTX_new();

		BN_bin2bn(pri_key, 32, private_key);
		EC_POINT_mul(group_, public_key, private_key, NULL, NULL, ctx);

		EC_KEY_set_group(new_key, group_);
		EC_KEY_set_private_key(new_key, private_key);
		EC_KEY_set_public_key(new_key, public_key);

		unsigned char *pbegin = pub_key;
		int pk_size = i2o_ECPublicKey(new_key, &pbegin);
		assert(pk_size == 65);

		BN_free(private_key);
		EC_KEY_free(new_key);
		EC_POINT_free(public_key);
		BN_CTX_free(ctx);

		return 0;
	}

	int ecc_sm2::sign(const unsigned char *m, int mlen, ed25519_secret_key pri_key, sm2_public_key pub_key, unsigned char *sig, int *siglen) {
		assert(group_ != NULL);

		BIGNUM *private_key = NULL;
		EC_KEY *new_key = NULL;
		EC_POINT *public_key = NULL;
		BN_CTX *ctx = NULL;

		private_key = BN_new();
		new_key = EC_KEY_new();
		public_key = EC_POINT_new(group_);
		ctx = BN_CTX_new();

		BN_bin2bn(pri_key, 32, private_key);
		EC_POINT_mul(group_, public_key, private_key, NULL, NULL, ctx);

		EC_KEY_set_group(new_key, group_);
		EC_KEY_set_private_key(new_key, private_key);
		EC_KEY_set_public_key(new_key, public_key);

		ECDSA_SIG *s;
		s = do_sign(m, mlen, new_key);
		if (s == NULL) {
			*siglen = 0;
		}
		else {
			*siglen = i2d_ECDSA_SIG(s, &sig);
			ECDSA_SIG_free(s);
		}
		BN_free(private_key);
		EC_KEY_free(new_key);
		EC_POINT_free(public_key);
		BN_CTX_free(ctx);
		return 0;
	}


	ECDSA_SIG* ecc_sm2::do_sign(const unsigned char* dgst, int dgst_len, EC_KEY *eckey) {
		const EC_GROUP* group = NULL;
		const BIGNUM* priv_key = NULL;
		ECDSA_SIG  *ret = NULL;
		BN_CTX *ctx = NULL;
		BIGNUM *order = NULL, *tmp = NULL, *m = NULL, *x = NULL, *a = NULL;

		EC_POINT* tmp_point = NULL;
		BIGNUM *K = NULL, *X = NULL, *X_R = NULL, *R = NULL, *S = NULL;

		int i;

		group = EC_KEY_get0_group(eckey);
		priv_key = EC_KEY_get0_private_key(eckey);

		if (group == NULL || priv_key == NULL /*|| ecdsa == NULL*/) {
			ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_PASSED_NULL_PARAMETER);
			return NULL;
		}

		ret = ECDSA_SIG_new();
		R = ret->r;
		S = ret->s;

		if (!ret) {
			ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_MALLOC_FAILURE);
			return NULL;
		}

		if ((ctx = BN_CTX_new()) == NULL || (order = BN_new()) == NULL ||
			(tmp = BN_new()) == NULL || (m = BN_new()) == NULL ||
			(x = BN_new()) == NULL || (a = BN_new()) == NULL) {
			ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_MALLOC_FAILURE);
			goto err;
		}

		if (!EC_GROUP_get_order(group, order, ctx)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_EC_LIB);
			goto err;
		}
		i = BN_num_bits(order);
		if (8 * dgst_len > i)
			dgst_len = (i + 7) / 8;
		if (!BN_bin2bn(dgst, dgst_len, m)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
			goto err;
		}
		if ((8 * dgst_len > i) && !BN_rshift(m, m, 8 - (i & 0x7))) {
			ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
			goto err;
		}


		//sign starting...
		do {
			do {
				//step 1: generate random K
				K = BN_new();
				do
					if (!BN_rand_range(K, order)) {
						ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ECDSA_R_RANDOM_NUMBER_GENERATION_FAILED);
						goto err;
					}
				while (BN_is_zero(K));

				// step2: calculate X, while (X, Y) = K * G
				X = BN_new();
				X_R = BN_new();
				if ((tmp_point = EC_POINT_new(group)) == NULL) {
					ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ERR_R_EC_LIB);
					goto err;
				}
				if (!EC_POINT_mul(group, tmp_point, K, NULL, NULL, ctx)) {
					ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ERR_R_EC_LIB);
					goto err;
				}
				if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) == NID_X9_62_prime_field) {
					if (!EC_POINT_get_affine_coordinates_GFp(group_,
						tmp_point, X, NULL, ctx)) {
						ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ERR_R_EC_LIB);
						goto err;
					}
				}
				else /* NID_X9_62_characteristic_two_field */ {
					if (!EC_POINT_get_affine_coordinates_GF2m(group_,
						tmp_point, X, NULL, ctx)) {
						ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ERR_R_EC_LIB);
						goto err;
					}
				}

				if (!BN_nnmod(X_R, X, order, ctx)) {
					ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ERR_R_BN_LIB);
					goto err;
				}

			} while (BN_is_zero(X_R));

			// step 3: calculate R = (m + X) % n
			if (!BN_mod_add_quick(R, m, X_R, order)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
				goto err;
			}
			if (BN_is_zero(R))
				continue;
			BN_add(tmp, R, K);
			if (BN_ucmp(tmp, order) == 0)
				continue;

			// step 4: calculate S = (1 + dA)^(-1)*(K - R*dA) % n;
			if (!BN_mod_mul(tmp, priv_key, R, order, ctx)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
				goto err;
			}
			if (!BN_mod_sub_quick(S, K, tmp, order)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
				goto err;
			}
			BN_one(a);
			if (!BN_mod_add_quick(tmp, priv_key, a, order)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
				goto err;
			}
			if (!BN_mod_inverse(tmp, tmp, order, ctx)) {
				ECDSAerr(ECDSA_F_ECDSA_SIGN_SETUP, ERR_R_BN_LIB);
				goto err;
			}
			if (!BN_mod_mul(S, S, tmp, order, ctx)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_SIGN, ERR_R_BN_LIB);
				goto err;
			}

		} while (BN_is_zero(S));

	err:
		free_bn(order);
		free_bn(tmp);
		free_bn(m);
		free_bn(x);
		free_bn(a);

		free_bn(K);
		free_bn(X);
		free_bn(X_R);

		if (ctx != NULL) BN_CTX_free(ctx);
		if (tmp_point != NULL)	EC_POINT_free(tmp_point);

		return ret;
	}

	bool ecc_sm2::verify(const unsigned char *m, int m_len, const unsigned char *sig, int sig_len, unsigned char *raw_pub_key) {
		assert(group_ != NULL);

		ECDSA_SIG *s;
		s = ECDSA_SIG_new();
		if (d2i_ECDSA_SIG(&s, &sig, sig_len) == NULL)
			goto err;
		return do_verify(m, m_len, s, raw_pub_key) == 0;
	err:
		ECDSA_SIG_free(s);
		return false;
	}

	int ecc_sm2::do_verify(const unsigned char *dgst, int dgst_len, ECDSA_SIG *sig, unsigned char *raw_pub_key) {

		BIGNUM *order = NULL;
		BN_CTX *ctx = NULL;
		BIGNUM *T = NULL, *X = NULL, *m = NULL, *R = NULL;
		EC_POINT *point = NULL;
		EC_KEY *new_key = NULL;
		int ret = 1, i;

		ctx = BN_CTX_new();
		if (!ctx) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_MALLOC_FAILURE);
			return 1;
		}
		if ((new_key = EC_KEY_new()) == NULL) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_MALLOC_FAILURE);
			return 1;
		}
		EC_KEY_set_group(new_key, group_);
		if (NULL == o2i_ECPublicKey(&new_key, (const unsigned char **)&raw_pub_key, 65)) {
			ABORT;
		}

		const EC_POINT *pub_key = EC_KEY_get0_public_key(new_key);

		order = BN_CTX_get(ctx);
		if (!EC_GROUP_get_order(group_, order, ctx)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_EC_LIB);
			goto err;
		}

		// step 1: check the validation of r and s;
		if (BN_is_zero(sig->r) || BN_is_negative(sig->r) ||
			BN_ucmp(sig->r, order) >= 0 || BN_is_zero(sig->s) ||
			BN_is_negative(sig->s) || BN_ucmp(sig->s, order) >= 0) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ECDSA_R_BAD_SIGNATURE);
			ret = 2;	/* signature is invalid */
			goto err;
		}

		// step 2: T = (r + s) % n;
		T = BN_new();
		if (!BN_mod_add_quick(T, sig->s, sig->r, order)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_BN_LIB);
			goto err;
		}
		if (BN_is_zero(T)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ECDSA_R_BAD_SIGNATURE);
			ret = 2;	/* signature is invalid */
			goto err;
		}

		// step 3: (X, Y) = s * G + t * pA;
		X = BN_new();
		m = BN_new();
		if ((point = EC_POINT_new(group_)) == NULL) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_MALLOC_FAILURE);
			goto err;
		}
		if (!EC_POINT_mul(group_, point, sig->s, pub_key, T, ctx)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_EC_LIB);
			goto err;
		}

		if (EC_METHOD_get_field_type(EC_GROUP_method_of(group_)) == NID_X9_62_prime_field) {
			if (!EC_POINT_get_affine_coordinates_GFp(group_,
				point, X, NULL, ctx)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_EC_LIB);
				goto err;
			}
		}
		else /* NID_X9_62_characteristic_two_field */ {
			if (!EC_POINT_get_affine_coordinates_GF2m(group_,
				point, X, NULL, ctx)) {
				ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_EC_LIB);
				goto err;
			}
		}

		i = BN_num_bits(order);
		if (8 * dgst_len > i)
			dgst_len = (i + 7) / 8;
		if (!BN_bin2bn(dgst, dgst_len, m)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_BN_LIB);
			goto err;
		}
		if ((8 * dgst_len > i) && !BN_rshift(m, m, 8 - (i & 0x7))) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_BN_LIB);
			goto err;
		}

		// step 4 : calculate R = (m + X) % n;
		R = BN_new();
		if (!BN_mod_add_quick(R, m, X, order)) {
			ECDSAerr(ECDSA_F_ECDSA_DO_VERIFY, ERR_R_BN_LIB);
			goto err;
		}

		ret = (BN_ucmp(R, sig->r) != 0);

	err:
		free_bn(order);
		free_bn(T);
		free_bn(X);
		free_bn(m);
		free_bn(R);

		free_ec_point(point);

		if (ctx != NULL)	BN_CTX_free(ctx);
		if (new_key != NULL)	EC_KEY_free(new_key);

		return ret;
	}
};
