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

#include <google/protobuf/descriptor.h>
#include <google/protobuf/reflection.h>
#include <google/protobuf/message.h>
#include "message.pb.h"
#include <json/json.h>
#include <utils/strings.h>
using namespace google::protobuf;
static Json::Value _pb2json(const Message& msg);
static Json::Value  _field2json(const google::protobuf::Message& msg, const google::protobuf::FieldDescriptor *field, size_t index) {
	const Reflection *ref = msg.GetReflection();
	const bool repeated = field->is_repeated();
	Json::Value jf;
	switch (field->cpp_type()) {

		case FieldDescriptor::CPPTYPE_BOOL:
		{
			jf = repeated ? ref->GetRepeatedBool(msg, field, index) : ref->GetBool(msg, field);
		}
		case FieldDescriptor::CPPTYPE_DOUBLE:
		{
			jf = repeated ? ref->GetRepeatedDouble(msg, field, index) : ref->GetDouble(msg, field);
			break;
		}
		case FieldDescriptor::CPPTYPE_INT32:
		{
			jf = repeated ? ref->GetRepeatedInt32(msg, field, index) : ref->GetInt32(msg, field);
			break;
		}
		case FieldDescriptor::CPPTYPE_UINT32:
		{
			jf = repeated ? ref->GetRepeatedUInt32(msg, field, index) : ref->GetUInt32(msg, field);
			break;
		}
		case FieldDescriptor::CPPTYPE_INT64:
		{
			jf = repeated ? ref->GetRepeatedInt64(msg, field, index) : ref->GetInt64(msg, field);
			break;
		}
		case FieldDescriptor::CPPTYPE_UINT64:
		{
			jf = repeated ? ref->GetRepeatedUInt64(msg, field, index) : ref->GetUInt64(msg, field);
			break;
		}
		case FieldDescriptor::CPPTYPE_FLOAT:
		{
			jf = repeated ? ref->GetRepeatedFloat(msg, field, index) : ref->GetFloat(msg, field);
			break;
		}
		case FieldDescriptor::CPPTYPE_STRING: {
			std::string scratch;
			const std::string &v = (repeated) ?
				ref->GetRepeatedStringReference(msg, field, index, &scratch) :
				ref->GetStringReference(msg, field, &scratch);
			if (field->type() == FieldDescriptor::TYPE_BYTES)
				jf = utils::String::BinToHexString(v);
			else
				jf = v;
			break;
		}
		case FieldDescriptor::CPPTYPE_MESSAGE: {
			const Message& mf = (repeated) ? ref->GetRepeatedMessage(msg, field, index) : ref->GetMessage(msg, field);
			jf = _pb2json(mf);
			break;
		}
		case FieldDescriptor::CPPTYPE_ENUM: {
			const EnumValueDescriptor* ef = (repeated) ?
				ref->GetRepeatedEnum(msg, field, index) :
				ref->GetEnum(msg, field);

			jf = ef->number();
			break;
		}
		default:
			break;
	}
	
	return jf;
}

static Json::Value _pb2json(const Message& msg) {
	const Descriptor *d = msg.GetDescriptor();
	const Reflection *ref = msg.GetReflection();
	if (!d || !ref) return 0;
	Json::Value va;

	std::vector<const FieldDescriptor *> fields;
	ref->ListFields(msg, &fields);

	for (size_t i = 0; i != fields.size(); i++) {
		const FieldDescriptor *field = fields[i];

		Json::Value jf;
		if (field->is_repeated()) {
			size_t count = ref->FieldSize(msg, field);
			if (!count) continue;

			for (size_t j = 0; j < count; j++)
				jf[i] = _field2json(msg, field, j);
		}
		else if (ref->HasField(msg, field))
			jf = _field2json(msg, field, 0);
		else
			continue;

		const std::string &name = (field->is_extension()) ? field->full_name() : field->name();
		va[name] = jf;
	}
	return va;
}