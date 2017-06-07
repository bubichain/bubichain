/*
Copyright 漏 Bubi Technologies Co., Ltd. 2017 All Rights Reserved.
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

#ifndef _MERKLE_H_
#define _MERKLE_H_

#include <utils/thread.h>

namespace bubi {
	class MNode {

		std::string hash_;
		bool cal_hash_;
	public:
		unsigned short  mask_;

		MNode() :cal_hash_(false), mask_(0), hash_("") {}

		MNode(const std::string &hash) : hash_(hash), cal_hash_(false), mask_(0) {}

		virtual ~MNode() {};

		virtual bool IsLeaf() = 0;

		void SetRecal() {
			cal_hash_ = true;
		}

		void SetUnRecal() {
			cal_hash_ = false;
		}

		bool CalHash() {
			return cal_hash_;
		}

		std::string Hash() {
			return hash_;
		}

		void SetHash(const std::string &h) {
			hash_ = h;
		}

		//virtual void UpdateHash() = 0;
	};

	template <int BIT>
	class IMNode :public MNode {
	public:
		MNode* children_[1 << BIT];
		IMNode() :MNode() {}

		~IMNode() {}

		bool virtual IsLeaf() override {
			return false;
		}

		bool HasChild(unsigned char  branch) {
			return ((1 << branch)  &mask_) != 0;
		}

		bool SetChild(unsigned char  branch, MNode* child) {
			children_[branch] = child;
			mask_ |= (1 << branch);
			return true;
		}
	};

	template<class Index, class Data>
	class LNode : public MNode {
	public:
		Index index_;
		Data data_;

		LNode(const Index &index, const Data &data) :MNode(), data_(data), index_(index) {}

		bool virtual IsLeaf() override {
			return true;
		}

		LNode(const std::string &hash, const Index &index, const Data &data) :MNode(hash), index_(index), data_(data) {}
		~LNode() {

		}

	};

	template<class Index, class Data, int BIT>
	class MTree {

		virtual  unsigned char  GetBranch(const Index &index, const unsigned char  &depth) = 0;
		virtual  unsigned char  MaxDepth() = 0;

		//virtual MNode* NodeFromDb(const std::string &location) = 0;

	public:

		//std::map<std::string, MNode*> nodes_;
		int leaf_count_;
		IMNode<BIT>* root_;

		typedef LNode<Index, Data> Leaf;

	public:
		MTree() {
			leaf_count_ = 0;
		}

		std::string RootHash() {
			return root_->Hash();
		}

		int LeafCount() {
			return leaf_count_;
		}


		bool Add(const Index &index, const Data &data, const std::string &hash) {

			IMNode<BIT>* pnode = root_;
			for (unsigned char depth = 1; depth <= MaxDepth(); depth++) {
				pnode->SetRecal();

				unsigned char  branch = GetBranch(index, depth);
				if (!pnode->HasChild(branch)) {
					LNode<Index, Data>* new_leaf = new LNode<Index, Data>(hash, index, data);
					if (hash.empty()) {
						new_leaf->SetRecal();
					}
					pnode->SetChild(branch, static_cast<MNode*>(new_leaf));
					leaf_count_++;
					return true;
				}

				MNode* cnode = pnode->children_[branch];

				if (!cnode->IsLeaf()) {
					pnode = dynamic_cast<IMNode<BIT>*> (cnode);
					continue;
				}


				//cnode is leaf
				LNode<Index, Data>* old_leaf = dynamic_cast<LNode<Index, Data>*>(cnode);

				Index index_old = old_leaf->index_;
				if (index_old == index) {
					return false;
				}
				for (; depth <= MaxDepth(); depth++) {
					unsigned char  branch_new = GetBranch(index, depth);
					unsigned char  branch_old = GetBranch(index_old, depth);
					pnode->SetRecal();//设置该节点需要重算hash

					if (branch_new == branch_old) {
						IMNode<BIT>* new_inode = new IMNode<BIT>();
						pnode->SetChild(branch_new, static_cast<MNode*>(new_inode));
						pnode = new_inode;

						continue;
					}
					else {
						LNode<Index, Data>* new_leaf = new LNode<Index, Data>(hash, index, data);
						if (hash.empty()) {
							new_leaf->SetRecal();
						}
						pnode->SetChild(branch_new, dynamic_cast<MNode*>(new_leaf));
						pnode->SetChild(branch_old, cnode);
						leaf_count_++;
						return true;
					}
				}
				return false;
			}
			return false;
		}


		bool Set(const Index &index, const Data &data, const std::string &hash) {
			IMNode<BIT>* pnode = root_;
			for (unsigned char depth = 1; depth < MaxDepth(); depth++) {
				pnode->SetRecal();
				unsigned char  branch = GetBranch(index, depth);
				if (!pnode->HasChild(branch)) {
					return false;
				}

				MNode* cnode = pnode->children_[branch];

				if (!cnode->IsLeaf()) {
					pnode = dynamic_cast<IMNode<BIT>*> (cnode);
					continue;
				}

				LNode<Index, Data>* leaf = dynamic_cast<LNode<Index, Data>*>(cnode);

				if (leaf->index_ != index) {
					return false;
				}
				leaf->data_ = data;
				leaf->SetHash(hash);
				if (hash.empty()) {
					leaf->SetRecal();
				}
				return true;
			}
			return false;
		}


		bool Has(const Index &index) {
			IMNode<BIT>* pnode = root_;
			for (unsigned char depth = 1; depth < MaxDepth(); depth++) {

				unsigned char  branch = GetBranch(index, depth);
				if (!pnode->HasChild(branch)) {
					return false;
				}

				MNode* cnode = pnode->children_[branch];

				if (!cnode->IsLeaf()) {
					pnode = dynamic_cast<IMNode<BIT>*> (cnode);
					continue;
				}

				LNode<Index, Data>* leaf = dynamic_cast<LNode<Index, Data>*>(cnode);
				if (leaf->index_ != index) {
					return false;
				}
				return true;
			}
			return false;
		}

		bool Get(const Index &index, Data &data) {
			IMNode<BIT>* pnode = root_;
			for (unsigned char depth = 1; depth < MaxDepth(); depth++) {
				pnode->SetRecal();
				unsigned char  branch = GetBranch(index, depth);
				if (!pnode->HasChild(branch)) {
					return false;
				}

				MNode* cnode = pnode->children_[branch];

				if (!cnode->IsLeaf()) {
					pnode = dynamic_cast<IMNode<BIT>*> (cnode);
					continue;
				}

				LNode<Index, Data>* leaf = dynamic_cast<LNode<Index, Data>*>(cnode);
				if (leaf->index_ != index) {
					return false;
				}
				data = leaf->data_;
				return true;
			}
			return false;
		}

		bool GetLeafNode(const Index &index, LNode<Index, Data>* &leafNode) {
			IMNode<BIT>* pnode = root_;
			for (unsigned char depth = 1; depth < MaxDepth(); depth++) {
				unsigned char  branch = GetBranch(index, depth);
				if (!pnode->HasChild(branch)) {
					return false;
				}

				MNode* cnode = pnode->children_[branch];

				if (!cnode->IsLeaf()) {
					pnode = dynamic_cast<IMNode<BIT>*> (cnode);
					continue;
				}

				LNode<Index, Data>* leaf = dynamic_cast<LNode<Index, Data>*>(cnode);
				if (leaf->index_ != index) {
					return false;
				}
				leafNode = leaf;
				return true;
			}
			return false;
		}
	};
}

#endif