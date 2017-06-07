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

#ifndef ASSET_FRM_H
#define ASSET_FRM_H

#include <proto/message.pb.h>
#include <utils/common.h>
#include <common/general.h>
#include <common/private_key.h>

namespace bubi {
	class AssetFrm {
	public:
		struct Sel //start ext length
		{
			Sel(int64_t s, int64_t l, const std::string &e) {
				start_ = s;
				length_ = l;
				ext_ = e;
			}
			std::string ext_;
			int64_t start_;
			int64_t length_;
			bool operator < (const Sel &r) const {
				if (length_ < 0) {
					return false;
				}
				else if (length_ == 0) {
					if (r.length_ < 0) {
						return true;
					}
					else if (r.length_ == 0) {
						return ext_ < r.ext_;
					}
					else if (r.length_ > 0) {
						return false;
					}
				}
				else if (length_ > 0) {
					if (r.length_ < 0) {
						return true;
					}
					else if (r.length_ == 0) {
						return true;
					}
					else if (r.length_ > 0) {
						int64_t a = length_ + start_ - r.length_ - r.start_;
						if (a == 0) {
							if (start_ == r.start_) {
								return ext_ < r.ext_;
							}
							else {
								return start_ > r.start_;
							}
						}
						else if (a < 0) {
							return true;
						}
						else if (a > 0) {
							return false;
						}
					}
				}
				return false;
			}
		};

		static bool IsValid(const protocol::Asset &asset) {
			int64_t namount = 0;
			std::set<Sel> tset;
			for (int i = 0; i < asset.details_size(); i++) {
				if (asset.details(i).amount() <= 0 ||
					asset.details(i).start() < 0 ||
					asset.details(i).length() < -1 ||
					asset.details(i).ext().length() > 64
					) {
					return false;
				}

				if (asset.details(i).length() > 0) {
					int64_t a = asset.details(i).length() + asset.details(i).start();
					if (a < 0) {
						return false;
					}
				}

				Sel itm(asset.details(i).start(), asset.details(i).length(), asset.details(i).ext());
				if (tset.find(itm) != tset.end()) {
					return false;
				}
				tset.insert(itm);
				namount += asset.details(i).amount();
				if (namount <= 0) {
					return false;
				}
			}
			do {
				if (asset.details_size() != 0 && namount != asset.amount()) {
					break;
				}

				if (asset.amount() <= 0) {
					return false;
				}
				if (asset.property().type() != protocol::AssetProperty_Type_IOU) {
					break;
				}

				if (!bubi::PublicKey::IsAddressValid(asset.property().issuer())) {
					break;
				}
				if (asset.property().code().length() == 0 ||
					asset.property().code().length() > 64) {
					break;
				}
				return true;
			} while (false);

			return false;
		}


		static void ToJson(const protocol::Asset &asset, Json::Value  &json) {
			const protocol::AssetProperty pro = asset.property();
			json["type"] = pro.type();
			json["issuer"] = pro.issuer();
			json["code"] = pro.code();
			json["amount"] = asset.amount();
			Json::Value &details = json["details"];
			for (int j = 0; j < asset.details_size(); j++) {
				const protocol::Detail d = asset.details(j);
				Json::Value &dtl = details[j];
				dtl["amount"] = d.amount();
				dtl["start"] = d.start();
				dtl["length"] = d.length();
				dtl["ext"] = d.ext();
			}
		}


		static bool FromJson(const Json::Value &js, protocol::Asset *asset) {

			if (!js.isMember("code") ||
				!js.isMember("issuer") ||
				!js.isMember("details") ||
				!js.isMember("amount")) {
				return false;
			}
			const Json::Value &details = js["details"];
			asset->set_amount(js["amount"].asInt64());
			asset->mutable_property()->set_type(protocol::AssetProperty_Type_IOU);
			asset->mutable_property()->set_issuer(js["issuer"].asString());
			asset->mutable_property()->set_code(js["code"].asString());

			for (Json::UInt i = 0; i < details.size(); i++) {
				const Json::Value item = details[i];
				if (!item.isMember("amount") ||
					!item.isMember("ext") ||
					!item.isMember("start") ||
					!item.isMember("length")
					) {
					return false;
					break;
				}
				protocol::Detail *d = asset->add_details();
				d->set_amount(item["amount"].asInt64());
				d->set_ext(item["ext"].asString());
				d->set_start(item["start"].asInt64());
				d->set_length(item["length"].asInt64());
			}
			return true;
		}
	};
};

#endif