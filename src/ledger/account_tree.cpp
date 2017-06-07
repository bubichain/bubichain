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

#include "account_tree.h"
#include "ledger/ledger_manager.h"

namespace bubi {
	void HashTask::Run(utils::Thread *this_thread) {
		tree_->UpdateHash(root_, skip_batch_);
	}

	AccountTree::AccountTree() {
		root_ = new IMNode<4>;
	}

	unsigned char  AccountTree::GetBranch(const std::string &index, const unsigned char  &depth) {
		unsigned char c = index.at((depth - 1) / 2);
		if (depth % 2 == 0) {
			return c & 0x0f;
		}
		else {
			return 0x0f & (c >> 4);
		}
	}

	void AccountTree::PutDbValues(const std::string &k, const std::string &v) {
		utils::WriteLockGuard guard(mutex_);
		batch_.Put(k, v);
	}

	void AccountTree::UpdateHash(MNode* node, bool skip_batch) {
		if (!node->CalHash()) {
			return;
		}

		if (!node->IsLeaf()) {
			IMNode<4>* inode = dynamic_cast<IMNode<4>*>(node);
			utils::Sha256 x;
			for (int i = 0; i < 16; i++) {
				if (inode->HasChild(i)) {
					MNode* cnode = inode->children_[i];
					if (cnode->CalHash()) {
						UpdateHash(cnode, skip_batch);
					}
					x.Update(cnode->Hash());
				}
			}
			node->SetHash(x.Final());
		}
		else {
			LNode<std::string, AccountFrm::pointer>* lnode = dynamic_cast<LNode<std::string, AccountFrm::pointer>*>(node);
			if (lnode->data_) {
				std::string s;
				if (!lnode->data_->GetProtoAccount().SerializeToString(&s)) {
					BUBI_EXIT("fatal error");
				}
				if (!skip_batch) {
					PutDbValues(lnode->index_, s);
				}
				lnode->SetHash(utils::Sha256::Crypto(s));
				lnode->data_ = nullptr;
			}
			else if (lnode->Hash().empty()) {
				BUBI_EXIT("fatal error");
			}
		}
		node->SetUnRecal();
	}

	void AccountTree::UpdateTreeHash(bool multi_thread, bool skip_batch) {
		if (multi_thread && Configure::Instance().system_config_.thread_count_ > 1) {
			utils::ThreadPool thread_pool_;
			thread_pool_.Init(Configure::GetInstance()->system_config_.thread_count_);
			std::vector<HashTask*> tasks;

			for (std::size_t i = 0; i < 16; i++) {
				if (root_->HasChild(i)) {
					HashTask* t1 = new HashTask(root_->children_[i], this, skip_batch);
					thread_pool_.AddTask(t1);
					tasks.push_back(t1);
				}
			}
			thread_pool_.WaitAndJoin();
			UpdateHash(root_, skip_batch);
			for (std::size_t i = 0; i < tasks.size(); i++) {
				delete tasks[i];
			}
		}
		else {
			UpdateHash(root_, skip_batch);
		}
	}

	bool AccountTree::LoadFromDb() {
		bubi::KeyValueDb* db = Storage::Instance().keyvalue_db();

		KVDB::Iterator* it = (KVDB::Iterator*)(db->NewIterator());

		for (it->SeekToFirst(); it->Valid(); it->Next()) {
			if (it->key().size() != 32) {
				//skip non-account record
				continue;
			}

			AccountFrm acc;
			if (acc.UnSerializer(it->value().ToString())) {
				std::string hash = utils::Sha256::Crypto(acc.GetProtoAccount().SerializeAsString());
				Add(it->key().ToString(), nullptr, hash);
			}

		}
		if (!it->status().ok())  // Check for any errors found during the scan
		{
			BUBI_EXIT("rocksdb error: %s", it->status().ToString().c_str());
		}

		delete it;

		this->UpdateTreeHash(true, true);
		Record();
		return root_ != nullptr;
	}

	void AccountTree::Record() {
		batch_.Clear();
	}
}