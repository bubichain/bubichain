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

#ifndef _BUBI_SERIALIZER_H_
#define 	_BUBI_SERIALIZER_H_

#include <vector>
#include <memory>
#include <string>

#include "utils.h"

namespace utils{

class Serializer{
public:
	typedef std::shared_ptr <Serializer> pointer;
	
	Serializer (){}
	~Serializer (){}
	Serializer (std::string str);

	std::vector <char>& peek_data ();
	static uint256 get_prefix_hash (const char *ch, int len);
	bool add_raw (const char* ch, int len);
	bool add256 (uint256 &hash);
	uint256 get_sha512_half ();
	bool	add_serializer (Serializer &s);
	std::size_t	peek_data_size ();
	
private:
	std::vector <char> data_;
};

}

#endif
