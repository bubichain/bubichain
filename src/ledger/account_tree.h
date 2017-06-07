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

#ifndef ACCOUNT_TREE_H_
#define ACCOUNT_TREE_H_

#include <common/storage.h>
#include "merkle.h"
#include "account.h"

namespace bubi {
	const std::string ACCOUNT_TREE_ROOT_KEY = "bubi";
	class AccountTree;
	class HashTask :public utils::Runnable {
	public:
		HashTask(MNode* node, AccountTree* tree, bool skip_batch) {
			root_ = node;
			tree_ = tree;
			skip_batch_ = skip_batch;
		}
		void Run(utils::Thread *this_thread) override;
	public:
		MNode* root_;
		AccountTree* tree_;
		bool skip_batch_;
	};
	class AccountTree :public MTree<std::string, AccountFrm::pointer, 4> {
		friend class HashTask;
	public:
		AccountTree();
		void Record();
		bool LoadFromDb();
		void UpdateTreeHash(bool multi_thread, bool skip_batch = false);
	public:
		WRITE_BATCH batch_;
		utils::ReadWriteLock mutex_;
	private:
		virtual  unsigned char  GetBranch(const std::string &index, const unsigned char  &depth) override;
		unsigned char MaxDepth() override { return 64; }
		void PutDbValues(const std::string &k, const std::string &v);
		void UpdateHash(MNode* node, bool skip_batch = false);
	};
}

#endif