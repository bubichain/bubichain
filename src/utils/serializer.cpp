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

#include "serializer.h"

#include <openssl/sha.h>

namespace utils{


Serializer::Serializer (std::string str){
	std::size_t sz = str.length ();
	for (size_t i = 0; i < sz; i++){
		data_.push_back (str[i]);
	}
}

std::vector <char>&
Serializer::peek_data (){
	return data_;
}

uint256
Serializer::get_prefix_hash (const char* ch, int len){
	uint256 j[2];
	SHA512_CTX  ctx;
	SHA512_Init (&ctx);
	SHA512_Update (&ctx, ch, len);
	SHA512_Final (reinterpret_cast <unsigned char *> (&j[0]), &ctx);
	return j[0];
}

bool
Serializer::add_serializer (Serializer &s){
	std::vector <char>& vt = s.peek_data ();
	add_raw (&(*(vt.begin())), vt.size());
	return true;
}

std::size_t
Serializer::peek_data_size (){
	return data_.size ();
}
bool
Serializer::add_raw (const char *ch, int len){
	for (int i=0; i<len; i++){
		data_.push_back (ch[i]);
	}
	return true;
}

bool
Serializer::add256 (uint256 &hash){
	data_.insert (data_.end(), hash.begin(),hash.end());
	return true;
}

uint256
Serializer::get_sha512_half (){
	uint256 j[2];
	SHA512 ( reinterpret_cast<unsigned char*>(&(data_.front())), data_.size(), reinterpret_cast <unsigned char *>(&j[0]) );
	return j[0];
}

}
