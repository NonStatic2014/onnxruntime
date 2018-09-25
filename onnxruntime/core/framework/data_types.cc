// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "core/framework/data_types.h"
#include "core/framework/tensor.h"
#include "core/inc/op_kernel_author.h"
#include "core/graph/onnx_protobuf.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "onnx/defs/data_type_utils.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

using namespace ONNX_NAMESPACE;
namespace onnxruntime {
template <>
MLDataType DataTypeImpl::GetType<Tensor>() {
  return TensorTypeBase::Type();
}

static bool IsTensorTypeScalar(const ONNX_NAMESPACE::TypeProto_Tensor& tensor_type_proto) {
  int sz = tensor_type_proto.shape().dim_size();
  return sz == 0 || sz == 1;
}

namespace data_types_internal {

template<typename T>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType();

template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<float>() {
  return ONNX_NAMESPACE::TensorProto_DataType_FLOAT;
}
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<uint8_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT8;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<int8_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT8;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<uint16_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT16;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<int16_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT16;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<int32_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT32;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<int64_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_INT64;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<std::string>() {
  return ONNX_NAMESPACE::TensorProto_DataType_STRING;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<bool>() {
  return ONNX_NAMESPACE::TensorProto_DataType_BOOL;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<MLFloat16>() {
  return ONNX_NAMESPACE::TensorProto_DataType_FLOAT16;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<double>() {
  return ONNX_NAMESPACE::TensorProto_DataType_DOUBLE;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<uint32_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT32;
};
template <>
constexpr ONNX_NAMESPACE::TensorProto_DataType ToTensorDataType<uint64_t>() {
  return ONNX_NAMESPACE::TensorProto_DataType_UINT64;
};

template<typename T>
struct TensorContainedTypeSetter<T> {
  static void SetTensorElementType(ONNX_NAMESPACE::TypeProto& proto) {
    proto.mutable_tensor_type()->set_elem_type(ToTensorDataType<T>());
  }
  static void SetMapKeyType(ONNX_NAMESPACE::TypeProto& proto) {
    proto.mutable_map_type()->set_key_type(ToTensorDataType<T>());
  }
};

// Pre-instantiate
template struct
    TensorContainedTypeSetter<float>;
template struct
    TensorContainedTypeSetter<uint8_t>;
template struct
    TensorContainedTypeSetter<int8_t>;
template struct
    TensorContainedTypeSetter<uint16_t>;
template struct
    TensorContainedTypeSetter<int16_t>;
template struct
    TensorContainedTypeSetter<int32_t>;
template struct
    TensorContainedTypeSetter<int64_t>;
template struct
    TensorContainedTypeSetter<std::string>;
template struct
    TensorContainedTypeSetter<bool>;
template struct
    TensorContainedTypeSetter<MLFloat16>;
template struct
    TensorContainedTypeSetter<double>;
template struct
    TensorContainedTypeSetter<uint32_t>;
template struct
    TensorContainedTypeSetter<uint64_t>;

void CopyMutableMapValue(const ONNX_NAMESPACE::TypeProto& value_proto,
                         ONNX_NAMESPACE::TypeProto& map_proto) {
  map_proto.mutable_map_type()->mutable_value_type()->CopyFrom(value_proto);
}

void CopyMutableSeqElement(const ONNX_NAMESPACE::TypeProto& elem_proto,
                           ONNX_NAMESPACE::TypeProto& proto) {
  proto.mutable_sequence_type()->mutable_elem_type()->CopyFrom(elem_proto);
}

void AssignOpaqueDomainName(const char* domain, const char* name,
                            ONNX_NAMESPACE::TypeProto& proto) {
  auto* mutable_opaque = proto.mutable_opaque_type();
  mutable_opaque->mutable_domain()->assign(domain);
  mutable_opaque->mutable_name()->assign(name);
}

void AddOpaqueParameter(const ONNX_NAMESPACE::TypeProto& param_proto,
                        ONNX_NAMESPACE::TypeProto& proto) {
  proto.mutable_opaque_type()->add_parameters()->CopyFrom(param_proto);
}

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Tensor& tensor_proto,
                  const ONNX_NAMESPACE::TypeProto_Tensor& type_proto);

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Map& map_proto,
                  const ONNX_NAMESPACE::TypeProto_Map& type_proto);

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Sequence& sequence_proto,
                  const ONNX_NAMESPACE::TypeProto_Sequence& type_proto);

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Opaque& opaque_proto,
                  const ONNX_NAMESPACE::TypeProto_Opaque& type_proto);

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Tensor& tensor_proto,
                  const ONNX_NAMESPACE::TypeProto_Tensor& type_proto) {
  if (!(type_proto.has_elem_type() &&
        type_proto.elem_type() == tensor_proto.elem_type())) {
    return false;
  }
  return true;
  /* Currently all Tensors with all kinds of shapes
     are mapped into the same MLDataType (same element type)
     so we omit shape from IsCompatible consideration
     */
}

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Map& map_proto,
                  const ONNX_NAMESPACE::TypeProto_Map& type_proto) {
  if (!(type_proto.has_key_type() &&
        type_proto.key_type() == map_proto.key_type())) {
    return false;
  }
  const auto& lhs = map_proto;
  const auto& rhs = type_proto;
  bool result = true;
  if (lhs.key_type() == rhs.key_type() &&
      lhs.value_type().value_case() == rhs.value_type().value_case()) {
    switch (lhs.value_type().value_case()) {
      case TypeProto::ValueCase::kTensorType:
        result = IsCompatible(lhs.value_type().tensor_type(), rhs.value_type().tensor_type());
        break;
      case TypeProto::ValueCase::kSequenceType:
        result = IsCompatible(lhs.value_type().sequence_type(), rhs.value_type().sequence_type());
        break;
      case TypeProto::ValueCase::kMapType:
        result = IsCompatible(lhs.value_type().map_type(), rhs.value_type().map_type());
        break;
      case TypeProto::ValueCase::kOpaqueType:
        result = IsCompatible(lhs.value_type().opaque_type(), rhs.value_type().opaque_type());
        break;
      default:
        LOTUS_ENFORCE(false);
        break;
    }
  } else {
    result = false;
  }
  return result;
}

bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Sequence& sequence_proto,
                  const ONNX_NAMESPACE::TypeProto_Sequence& type_proto) {
  bool result = true;
  const auto& lhs = sequence_proto;
  const auto& rhs = type_proto;
  if (rhs.has_elem_type() &&
      lhs.elem_type().value_case() == rhs.elem_type().value_case()) {
    switch (lhs.elem_type().value_case()) {
      case TypeProto::ValueCase::kTensorType:
        result = IsCompatible(lhs.elem_type().tensor_type(), rhs.elem_type().tensor_type());
        break;
      case TypeProto::ValueCase::kSequenceType:
        result = IsCompatible(lhs.elem_type().sequence_type(), rhs.elem_type().sequence_type());
        break;
      case TypeProto::ValueCase::kMapType:
        result = IsCompatible(lhs.elem_type().map_type(), rhs.elem_type().map_type());
        break;
      case TypeProto::ValueCase::kOpaqueType:
        result = IsCompatible(lhs.elem_type().opaque_type(), rhs.elem_type().opaque_type());
        break;
      default:
        LOTUS_ENFORCE(false);
        break;
    }
  } else {
    result = false;
  }
  return result;
}
bool IsCompatible(const ONNX_NAMESPACE::TypeProto_Opaque& opaque_proto, const ONNX_NAMESPACE::TypeProto_Opaque& type_proto) {
  const auto& lhs = opaque_proto;
  const auto& rhs = type_proto;
  if (!rhs.has_domain() && !lhs.domain().empty()) {
    return false;
  }
  if (rhs.has_domain() && lhs.domain() != rhs.domain()) {
    return false;
  }
  if (!rhs.has_name() && !lhs.name().empty()) {
    return false;
  }
  if (rhs.has_name() && lhs.name() != rhs.name()) {
    return false;
  }
  bool result = true;
  if (lhs.parameters_size() == rhs.parameters_size()) {
    for (int i = 0; i < rhs.parameters_size() && result; ++i) {
      const auto& lparam = lhs.parameters(i);
      const auto& rparam = rhs.parameters(i);
      if (lparam.value_case() == rparam.value_case()) {
        switch (lparam.value_case()) {
          case TypeProto::ValueCase::kTensorType:
            result = IsCompatible(lparam.tensor_type(), rparam.tensor_type());
            break;
          case TypeProto::ValueCase::kSequenceType:
            result = IsCompatible(lparam.sequence_type(), rparam.sequence_type());
            break;
          case TypeProto::ValueCase::kMapType:
            result = IsCompatible(lparam.map_type(), rparam.map_type());
            break;
          case TypeProto::ValueCase::kOpaqueType:
            result = IsCompatible(lparam.opaque_type(), rparam.opaque_type());
            break;
          default:
            LOTUS_ENFORCE(false);
            break;
        }
      } else {
        result = false;
      }
    }
  } else {
    result = false;
  }
  return result;
}

struct TypeProtoImpl {
  const TypeProto* GetProto() const {
    return &proto_;
  }
  TypeProto& mutable_type_proto() {
    return proto_;
  }
  TypeProto proto_;
};

}  // namespace data_types_internal

/// TensorTypeBase
struct TensorTypeBase::Impl : public data_types_internal::TypeProtoImpl {
};

const ONNX_NAMESPACE::TypeProto* TensorTypeBase::GetTypeProto() const {
  return impl_->GetProto();
}

TensorTypeBase::TensorTypeBase() : impl_(new Impl()) {}
TensorTypeBase::~TensorTypeBase() {
  delete impl_;
}

size_t TensorTypeBase::Size() const {
  return sizeof(Tensor);
}

template <typename T>
static void Delete(void* p) {
  delete static_cast<T*>(p);
}

DeleteFunc TensorTypeBase::GetDeleteFunc() const {
  return &Delete<Tensor>;
}

ONNX_NAMESPACE::TypeProto& TensorTypeBase::mutable_type_proto() {
  return impl_->mutable_type_proto();
}

bool TensorTypeBase::IsCompatible(const ONNX_NAMESPACE::TypeProto& type_proto) const {
  const auto* thisProto = GetTypeProto();
  if (&type_proto == thisProto) {
    return true;
  }
  if (type_proto.value_case() != TypeProto::ValueCase::kTensorType) {
    return false;
  }

  LOTUS_ENFORCE(thisProto->value_case() == TypeProto::ValueCase::kTensorType);
  LOTUS_ENFORCE(thisProto->tensor_type().has_elem_type());

  return data_types_internal::IsCompatible(thisProto->tensor_type(), type_proto.tensor_type());
}

/// NoTensorTypeBase
struct NonTensorTypeBase::Impl : public data_types_internal::TypeProtoImpl {};

NonTensorTypeBase::NonTensorTypeBase() : impl_(new Impl()) {
}

NonTensorTypeBase::~NonTensorTypeBase() {
  delete impl_;
}

ONNX_NAMESPACE::TypeProto& NonTensorTypeBase::mutable_type_proto() {
  return impl_->mutable_type_proto();
}

const ONNX_NAMESPACE::TypeProto* NonTensorTypeBase::GetTypeProto() const {
  return impl_->GetProto();
}

bool NonTensorTypeBase::IsMapCompatible(const ONNX_NAMESPACE::TypeProto& type_proto) const {
  const auto* thisProto = impl_->GetProto();
  if (&type_proto == thisProto) {
    return true;
  }
  if (type_proto.value_case() != TypeProto::ValueCase::kMapType) {
    return false;
  }
  LOTUS_ENFORCE(thisProto->value_case() == TypeProto::ValueCase::kMapType);
  LOTUS_ENFORCE(thisProto->map_type().has_key_type());
  LOTUS_ENFORCE(thisProto->map_type().has_value_type());
  return data_types_internal::IsCompatible(thisProto->map_type(), type_proto.map_type());
}

bool NonTensorTypeBase::IsSequenceCompatible(const ONNX_NAMESPACE::TypeProto& type_proto) const {
  const auto* thisProto = impl_->GetProto();
  if (&type_proto == thisProto) {
    return true;
  }
  if (type_proto.value_case() != TypeProto::ValueCase::kSequenceType) {
    return false;
  }
  LOTUS_ENFORCE(thisProto->value_case() == TypeProto::ValueCase::kSequenceType);
  LOTUS_ENFORCE(thisProto->sequence_type().has_elem_type());
  return data_types_internal::IsCompatible(thisProto->sequence_type(), type_proto.sequence_type());
}

bool NonTensorTypeBase::IsOpaqueCompatible(const ONNX_NAMESPACE::TypeProto& type_proto) const {
  const auto* thisProto = impl_->GetProto();
  if (&type_proto == thisProto) {
    return true;
  }
  if (type_proto.value_case() != TypeProto::ValueCase::kOpaqueType) {
    return false;
  }
  LOTUS_ENFORCE(thisProto->value_case() == TypeProto::ValueCase::kOpaqueType);
  LOTUS_ENFORCE(thisProto->opaque_type().has_domain());
  LOTUS_ENFORCE(thisProto->opaque_type().has_name());
  return data_types_internal::IsCompatible(thisProto->opaque_type(), type_proto.opaque_type());
}

LOTUS_REGISTER_TENSOR_TYPE(int32_t);
LOTUS_REGISTER_TENSOR_TYPE(float);
LOTUS_REGISTER_TENSOR_TYPE(bool);
LOTUS_REGISTER_TENSOR_TYPE(std::string);
LOTUS_REGISTER_TENSOR_TYPE(int8_t);
LOTUS_REGISTER_TENSOR_TYPE(uint8_t);
LOTUS_REGISTER_TENSOR_TYPE(uint16_t);
LOTUS_REGISTER_TENSOR_TYPE(int16_t);
LOTUS_REGISTER_TENSOR_TYPE(int64_t);
LOTUS_REGISTER_TENSOR_TYPE(double);
LOTUS_REGISTER_TENSOR_TYPE(uint32_t);
LOTUS_REGISTER_TENSOR_TYPE(uint64_t);
LOTUS_REGISTER_TENSOR_TYPE(MLFloat16);

LOTUS_REGISTER_MAP(MapStringToString);
LOTUS_REGISTER_MAP(MapStringToInt64);
LOTUS_REGISTER_MAP(MapStringToFloat);
LOTUS_REGISTER_MAP(MapStringToDouble);
LOTUS_REGISTER_MAP(MapInt64ToString);
LOTUS_REGISTER_MAP(MapInt64ToInt64);
LOTUS_REGISTER_MAP(MapInt64ToFloat);
LOTUS_REGISTER_MAP(MapInt64ToDouble);

LOTUS_REGISTER_SEQ(VectorString);
LOTUS_REGISTER_SEQ(VectorFloat);
LOTUS_REGISTER_SEQ(VectorInt64);
LOTUS_REGISTER_SEQ(VectorDouble);

LOTUS_REGISTER_SEQ(VectorMapStringToFloat);
LOTUS_REGISTER_SEQ(VectorMapInt64ToFloat);

MLDataType DataTypeImpl::TypeFromProto(const ONNX_NAMESPACE::TypeProto& proto) {
  switch (proto.value_case()) {
    case TypeProto::ValueCase::kTensorType: {
      auto tensor_type = proto.tensor_type();
      LOTUS_ENFORCE(tensor_type.has_elem_type());
      switch (tensor_type.elem_type()) {
        case TensorProto_DataType_FLOAT:
          return DataTypeImpl::GetTensorType<float>();
        case TensorProto_DataType_BOOL:
          return DataTypeImpl::GetTensorType<bool>();
        case TensorProto_DataType_INT32:
          return DataTypeImpl::GetTensorType<int32_t>();
        case TensorProto_DataType_DOUBLE:
          return DataTypeImpl::GetTensorType<double>();
        case TensorProto_DataType_STRING:
          return DataTypeImpl::GetTensorType<std::string>();
        case TensorProto_DataType_UINT8:
          return DataTypeImpl::GetTensorType<uint8_t>();
        case TensorProto_DataType_UINT16:
          return DataTypeImpl::GetTensorType<uint16_t>();
        case TensorProto_DataType_INT8:
          return DataTypeImpl::GetTensorType<int8_t>();
        case TensorProto_DataType_INT16:
          return DataTypeImpl::GetTensorType<int16_t>();
        case TensorProto_DataType_INT64:
          return DataTypeImpl::GetTensorType<int64_t>();
        case TensorProto_DataType_UINT32:
          return DataTypeImpl::GetTensorType<uint32_t>();
        case TensorProto_DataType_UINT64:
          return DataTypeImpl::GetTensorType<uint64_t>();
        case TensorProto_DataType_FLOAT16:
          return DataTypeImpl::GetTensorType<MLFloat16>();
        default:
          LOTUS_NOT_IMPLEMENTED("tensor type ", tensor_type.elem_type(), " is not supported");
      }
    } break;
    case TypeProto::ValueCase::kMapType: {
      auto maptype = proto.map_type();
      auto keytype = maptype.key_type();
      auto value_type = maptype.value_type();
      if (value_type.value_case() != TypeProto::ValueCase::kTensorType ||
          !IsTensorTypeScalar(value_type.tensor_type())) {
        LOTUS_NOT_IMPLEMENTED("Nested map/sequence type is not supported");
      }

      auto value_elem_type = value_type.tensor_type().elem_type();
      switch (value_elem_type) {
        case TensorProto_DataType_STRING: {
          switch (keytype) {
            case TensorProto_DataType_STRING:
              return DataTypeImpl::GetType<MapStringToString>();
            case TensorProto_DataType_INT64:
              return DataTypeImpl::GetType<MapInt64ToString>();
            default:
              LOTUS_NOT_IMPLEMENTED("Map with key type: ", keytype, " is not supported");
          }
        }
        case TensorProto_DataType_INT64:
          switch (keytype) {
            case TensorProto_DataType_STRING:
              return DataTypeImpl::GetType<MapStringToInt64>();
            case TensorProto_DataType_INT64:
              return DataTypeImpl::GetType<MapInt64ToInt64>();
            default:
              LOTUS_NOT_IMPLEMENTED("Map with key type: ", keytype, " is not supported");
          }
        case TensorProto_DataType_FLOAT:
          switch (keytype) {
            case TensorProto_DataType_STRING:
              return DataTypeImpl::GetType<MapStringToFloat>();
            case TensorProto_DataType_INT64:
              return DataTypeImpl::GetType<MapInt64ToFloat>();
            default:
              LOTUS_NOT_IMPLEMENTED("Map with key type: ", keytype, " is not supported");
          }
        case TensorProto_DataType_DOUBLE:
          switch (keytype) {
            case TensorProto_DataType_STRING:
              return DataTypeImpl::GetType<MapStringToDouble>();
            case TensorProto_DataType_INT64:
              return DataTypeImpl::GetType<MapInt64ToDouble>();
            default:
              LOTUS_NOT_IMPLEMENTED("Map with key type: ", keytype, " is not supported");
          }
        default:
          LOTUS_NOT_IMPLEMENTED("Map with value type: ", value_elem_type, " is not supported");
      }
    } break;
    case TypeProto::ValueCase::kSequenceType: {
      auto& seq_type = proto.sequence_type();
      auto& val_type = seq_type.elem_type();

      switch (val_type.value_case()) {
        case TypeProto::ValueCase::kMapType: {
          auto& maptype = val_type.map_type();
          auto keytype = maptype.key_type();
          auto& value_type = maptype.value_type();
          if (value_type.value_case() != TypeProto::ValueCase::kTensorType ||
              !IsTensorTypeScalar(value_type.tensor_type())) {
            LOTUS_THROW("Nested map/sequence type is not supported");
          }

          auto value_elem_type = value_type.tensor_type().elem_type();
          switch (value_elem_type) {
            case TensorProto_DataType_FLOAT: {
              switch (keytype) {
                case TensorProto_DataType_STRING:
                  return DataTypeImpl::GetType<VectorMapStringToFloat>();
                case TensorProto_DataType_INT64:
                  return DataTypeImpl::GetType<VectorMapInt64ToFloat>();
                default:
                  LOTUS_THROW("Map with key type: ", keytype, " is not supported");
              }
            }
            default:
              LOTUS_THROW("Sequence type that has a map of value type other than float not supported for now.");
          }
        }
        case TypeProto::ValueCase::kTensorType: {
          auto val_elem_type = val_type.tensor_type().elem_type();
          switch (val_elem_type) {
            case TensorProto_DataType_STRING:
              return DataTypeImpl::GetType<VectorString>();
            case TensorProto_DataType_INT64:
              return DataTypeImpl::GetType<VectorInt64>();
            case TensorProto_DataType_FLOAT:
              return DataTypeImpl::GetType<VectorFloat>();
            case TensorProto_DataType_DOUBLE:
              return DataTypeImpl::GetType<VectorDouble>();
            default:
              LOTUS_THROW("Sequence with value type: ", val_elem_type, " is not supported");
          }
        }
        default:
          throw ::onnxruntime::NotImplementedException("type is not supported");
      }
    }
    default:
      throw ::onnxruntime::NotImplementedException(::onnxruntime::MakeString("Onnx type: ", proto.value_case(), " is not supported."));
  }
}

//Below are the types the we need to execute the runtime
//They are not compatible with TypeProto in ONNX.
LOTUS_REGISTER_NON_ONNX_TYPE(int32_t);
LOTUS_REGISTER_NON_ONNX_TYPE(float);
LOTUS_REGISTER_NON_ONNX_TYPE(bool);
LOTUS_REGISTER_NON_ONNX_TYPE(std::string);
LOTUS_REGISTER_NON_ONNX_TYPE(int8_t);
LOTUS_REGISTER_NON_ONNX_TYPE(uint8_t);
LOTUS_REGISTER_NON_ONNX_TYPE(uint16_t);
LOTUS_REGISTER_NON_ONNX_TYPE(int16_t);
LOTUS_REGISTER_NON_ONNX_TYPE(int64_t);
LOTUS_REGISTER_NON_ONNX_TYPE(double);
LOTUS_REGISTER_NON_ONNX_TYPE(uint32_t);
LOTUS_REGISTER_NON_ONNX_TYPE(uint64_t);
LOTUS_REGISTER_NON_ONNX_TYPE(MLFloat16);

const std::vector<MLDataType>& DataTypeImpl::AllFixedSizeTensorTypes() {
  static std::vector<MLDataType> all_fixed_size_tensor_types =
      {DataTypeImpl::GetTensorType<float>(),
       DataTypeImpl::GetTensorType<double>(),
       DataTypeImpl::GetTensorType<int64_t>(),
       DataTypeImpl::GetTensorType<uint64_t>(),
       DataTypeImpl::GetTensorType<int32_t>(),
       DataTypeImpl::GetTensorType<uint32_t>(),
       DataTypeImpl::GetTensorType<int16_t>(),
       DataTypeImpl::GetTensorType<uint16_t>(),
       DataTypeImpl::GetTensorType<int8_t>(),
       DataTypeImpl::GetTensorType<uint8_t>(),
       DataTypeImpl::GetTensorType<MLFloat16>(),
       DataTypeImpl::GetTensorType<bool>()};

  return all_fixed_size_tensor_types;
}

const std::vector<MLDataType>& DataTypeImpl::AllTensorTypes() {
  static std::vector<MLDataType> all_tensor_types =
      {DataTypeImpl::GetTensorType<float>(),
       DataTypeImpl::GetTensorType<double>(),
       DataTypeImpl::GetTensorType<int64_t>(),
       DataTypeImpl::GetTensorType<uint64_t>(),
       DataTypeImpl::GetTensorType<int32_t>(),
       DataTypeImpl::GetTensorType<uint32_t>(),
       DataTypeImpl::GetTensorType<int16_t>(),
       DataTypeImpl::GetTensorType<uint16_t>(),
       DataTypeImpl::GetTensorType<int8_t>(),
       DataTypeImpl::GetTensorType<uint8_t>(),
       DataTypeImpl::GetTensorType<MLFloat16>(),
       DataTypeImpl::GetTensorType<bool>(),
       DataTypeImpl::GetTensorType<std::string>()};

  return all_tensor_types;
}

// helper to stream. expected to only be used for error output, so any typeid lookup
// cost should be fine. alternative would be to add a static string field to DataTypeImpl
// that we set in the register macro to the type name, and output that instead.
std::ostream& operator<<(std::ostream& out, const MLDataType data_type) {
  if (data_type == nullptr)
    return out << "(null)";

  return out << typeid(*data_type).name();
}

}  // namespace onnxruntime