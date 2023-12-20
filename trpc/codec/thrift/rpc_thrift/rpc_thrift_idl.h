// Copyright (c) 2021, Tencent Inc.
// All rights reserved.
#ifdef BUILD_INCLUDE_CODEC_THRIFT
#pragma once

#include <stddef.h>

#include <list>
#include <map>
#include <set>
#include <vector>

#include "rpc_thrift_buffer.h"
#include "rpc_thrift_enum.h"

namespace trpc {

class ThriftDescriptor {
 public:
  using ReaderFunctionPTR = bool (*)(ThriftBuffer*, void*);
  using WriterFunctionPTR = uint32_t (*)(const void*, ThriftBuffer*);
  virtual ~ThriftDescriptor() = default;

  ThriftDescriptor() : reader(nullptr), writer(nullptr) {}

  ThriftDescriptor(const ReaderFunctionPTR r, const WriterFunctionPTR w) : reader(r), writer(w) {}

 public:
  const ReaderFunctionPTR reader;
  const WriterFunctionPTR writer;
  int8_t data_type{0};
};

struct struct_element {
  const ThriftDescriptor* desc;
  const char* name;
  ptrdiff_t isset_offset;
  ptrdiff_t data_offset;
  int16_t field_id;
  int8_t required_state;
};

template <class T>
class ThriftElementsImpl {
 public:
  static const std::list<struct_element>* GetInstance() {
    static const ThriftElementsImpl<T> kInstance;

    return &kInstance.elements;
  }

 private:
  ThriftElementsImpl<T>() { T::StaticElementsImpl(&this->elements); }

  std::list<struct_element> elements;
};

class ThriftIDLMessage {
 public:
  virtual ~ThriftIDLMessage() = default;

 public:
  const ThriftDescriptor* descriptor = nullptr;
  const std::list<struct_element>* elements = nullptr;
};

template <class T, int8_t DATA, class KEYIMPL, class VALIMPL>
class ThriftDescriptorImpl : public ThriftDescriptor {
 public:
  static const ThriftDescriptor* GetInstance() {
    static const ThriftDescriptorImpl<T, DATA, KEYIMPL, VALIMPL> kInstance;

    return &kInstance;
  }

 public:
  static bool Read(ThriftBuffer* buffer, void* data);
  static uint32_t Write(const void* data, ThriftBuffer* buffer);

 private:
  ThriftDescriptorImpl<T, DATA, KEYIMPL, VALIMPL>()
      : ThriftDescriptor(ThriftDescriptorImpl<T, DATA, KEYIMPL, VALIMPL>::Read,
                         ThriftDescriptorImpl<T, DATA, KEYIMPL, VALIMPL>::Write) {
    this->data_type = DATA;
  }
};

template <class T, class VALIMPL>
class ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kList), void, VALIMPL>
    : public ThriftDescriptor {
 public:
  static const ThriftDescriptor* GetInstance() {
    static const ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kList), void, VALIMPL>
        kInstance;

    return &kInstance;
  }

  static bool Read(ThriftBuffer* buffer, void* data) {
    T* list = static_cast<T*>(data);
    const auto* val_desc = VALIMPL::GetInstance();
    int8_t field_type;
    int32_t count;

    if (!buffer->ReadI08(field_type)) return false;

    if (!buffer->ReadI32(count)) return false;

    list->resize(count);
    for (auto& ele : *list) {
      if (!val_desc->reader(buffer, &ele)) return false;
    }

    return true;
  }

  static uint32_t Write(const void* data, ThriftBuffer* buffer) {
    const T* list = static_cast<const T*>(data);
    const auto* val_desc = VALIMPL::GetInstance();
    uint32_t wsize = 0;
    wsize += buffer->WriteI08(val_desc->data_type);

    wsize += buffer->WriteI32(list->size());

    for (const auto& ele : *list) {
      wsize += val_desc->writer(&ele, buffer);
    }

    return wsize;
  }

 private:
  ThriftDescriptorImpl()
      : ThriftDescriptor(
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kList), void, VALIMPL>::Read,
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kList), void, VALIMPL>::Write) {
    this->data_type = codec::ToUType(ThriftDataType::kList);
  }
};

template <class T, class VALIMPL>
class ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kSet), void, VALIMPL>
    : public ThriftDescriptor {
 public:
  static const ThriftDescriptor* GetInstance() {
    static const ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kSet), void, VALIMPL>
        kInstance;

    return &kInstance;
  }

  static bool Read(ThriftBuffer* buffer, void* data) {
    T* set = static_cast<T*>(data);
    const auto* val_desc = VALIMPL::GetInstance();
    int8_t field_type;
    int32_t count;
    typename T::value_type ele;

    if (!buffer->ReadI08(field_type)) return false;

    if (!buffer->ReadI32(count)) return false;

    set->clear();
    for (int i = 0; i < count; i++) {
      if (!val_desc->reader(buffer, &ele)) return false;

      set->insert(std::move(ele));
    }

    return true;
  }

  static uint32_t Write(const void* data, ThriftBuffer* buffer) {
    const T* set = static_cast<const T*>(data);
    const auto* val_desc = VALIMPL::GetInstance();
    uint32_t wsize = 0;
    wsize += buffer->WriteI08(val_desc->data_type);
    wsize += buffer->WriteI32(set->size());

    for (const auto& ele : *set) {
      wsize += val_desc->writer(&ele, buffer);
    }

    return wsize;
  }

 private:
  ThriftDescriptorImpl()
      : ThriftDescriptor(
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kSet), void, VALIMPL>::Read,
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kSet), void, VALIMPL>::Write) {
    this->data_type = codec::ToUType(ThriftDataType::kSet);
  }
};

template <class T, class KEYIMPL, class VALIMPL>
class ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kMap), KEYIMPL, VALIMPL>
    : public ThriftDescriptor {
 public:
  static const ThriftDescriptor* GetInstance() {
    static const ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kMap), KEYIMPL, VALIMPL>
        kInstance;

    return &kInstance;
  }

  static bool Read(ThriftBuffer* buffer, void* data) {
    T* map = static_cast<T*>(data);
    const auto* key_desc = KEYIMPL::GetInstance();
    const auto* val_desc = VALIMPL::GetInstance();
    int8_t field_type;
    int32_t count;
    typename T::key_type key;
    typename T::mapped_type val;

    if (!buffer->ReadI08(field_type)) return false;

    if (!buffer->ReadI08(field_type)) return false;

    if (!buffer->ReadI32(count)) return false;

    map->clear();
    for (int i = 0; i < count; i++) {
      if (!key_desc->reader(buffer, &key)) return false;

      if (!val_desc->reader(buffer, &val)) return false;

      map->insert(std::make_pair(std::move(key), std::move(val)));
    }

    return true;
  }

  static uint32_t Write(const void* data, ThriftBuffer* buffer) {
    const T* map = static_cast<const T*>(data);
    const auto* key_desc = KEYIMPL::GetInstance();
    const auto* val_desc = VALIMPL::GetInstance();
    uint32_t wsize = 0;

    wsize += buffer->WriteI08(key_desc->data_type);
    wsize += buffer->WriteI08(val_desc->data_type);
    wsize += buffer->WriteI32(map->size());

    for (const auto& kv : *map) {
      wsize += key_desc->writer(&kv.first, buffer);
      wsize += val_desc->writer(&kv.second, buffer);
    }

    return wsize;
  }

 private:
  ThriftDescriptorImpl()
      : ThriftDescriptor(
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kMap), KEYIMPL, VALIMPL>::Read,
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kMap), KEYIMPL,
                                 VALIMPL>::Write) {
    this->data_type = codec::ToUType(ThriftDataType::kMap);
  }
};

template <class T>
class ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kStruct), void, void>
    : public ThriftDescriptor {
 public:
  static const ThriftDescriptor* GetInstance() {
    static const ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kStruct), void, void>
        kInstance;

    return &kInstance;
  }

 public:
  static bool Read(ThriftBuffer* buffer, void* data) {
    T* st = static_cast<T*>(data);
    char* base = static_cast<char*>(data);
    int8_t field_type;
    int16_t field_id;
    auto it = st->elements->cbegin();

    while (true) {
      if (!buffer->ReadFieldBegin(field_type, field_id)) return false;

      if (field_type == codec::ToUType(ThriftDataType::kStop)) return true;

      if (field_type == codec::ToUType(ThriftDataType::kStruct)) continue; 

      while (it != st->elements->cend() && it->field_id < field_id) ++it;

      if (it != st->elements->cend() && it->field_id == field_id &&
          it->desc->data_type == field_type) {
        if (it->required_state != kThriftStructFieldRequired)
          *(reinterpret_cast<bool*>(base + it->isset_offset)) = true;

        if (!it->desc->reader(buffer, base + it->data_offset)) return false;
      } else {
        if (!buffer->Skip(field_type)) return false;
      }
    }

    return true;
  }

  static uint32_t Write(const void* data, ThriftBuffer* buffer) {
    const T* st = static_cast<const T*>(data);
    const char* base = (const char*)data;
    uint32_t wsize = 0;
    for (const auto& ele : *(st->elements)) {
      if (ele.required_state != kThriftStructFieldOptional ||
          *(const_cast<bool*>(reinterpret_cast<const bool*>(base + ele.isset_offset)))) {
        wsize += buffer->WriteFieldBegin(ele.desc->data_type, ele.field_id);
        wsize += ele.desc->writer(base + ele.data_offset, buffer);
      }
    }

    wsize += buffer->WriteFieldStop();
    return wsize;
  }

 private:
  ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kStruct), void, void>()
      : ThriftDescriptor(
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kStruct), void, void>::Read,
            ThriftDescriptorImpl<T, codec::ToUType(ThriftDataType::kStruct), void, void>::Write) {
    this->data_type = codec::ToUType(ThriftDataType::kStruct);
  }
};

template <>
inline bool ThriftDescriptorImpl<bool, codec::ToUType(ThriftDataType::kBool), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  int8_t* p = static_cast<int8_t*>(data);

  return buffer->ReadI08(*p);
}

template <>
inline bool ThriftDescriptorImpl<int8_t, codec::ToUType(ThriftDataType::kI08), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  int8_t* p = static_cast<int8_t*>(data);

  return buffer->ReadI08(*p);
}

template <>
inline bool ThriftDescriptorImpl<int16_t, codec::ToUType(ThriftDataType::kI16), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  int16_t* p = static_cast<int16_t*>(data);

  return buffer->ReadI16(*p);
}

template <>
inline bool ThriftDescriptorImpl<int32_t, codec::ToUType(ThriftDataType::kI32), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  int32_t* p = static_cast<int32_t*>(data);

  return buffer->ReadI32(*p);
}

template <>
inline bool ThriftDescriptorImpl<int64_t, codec::ToUType(ThriftDataType::kI64), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  int64_t* p = static_cast<int64_t*>(data);

  return buffer->ReadI64(*p);
}

template <>
inline bool ThriftDescriptorImpl<uint64_t, codec::ToUType(ThriftDataType::kU64), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  uint64_t* p = static_cast<uint64_t*>(data);

  return buffer->ReadU64(*p);
}

template <>
inline bool ThriftDescriptorImpl<double, codec::ToUType(ThriftDataType::kDouble), void, void>::Read(
    ThriftBuffer* buffer, void* data) {
  uint64_t* p = static_cast<uint64_t*>(data);

  return buffer->ReadU64(*p);
}

template <>
inline bool ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kString), void,
                                 void>::Read(ThriftBuffer* buffer, void* data) {
  std::string* p = static_cast<std::string*>(data);

  return buffer->ReadString(*p);
}

template <>
inline bool ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kUtf8), void,
                                 void>::Read(ThriftBuffer* buffer, void* data) {
  std::string* p = static_cast<std::string*>(data);

  return buffer->ReadString(*p);
}

template <>
inline bool ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kUtf16), void,
                                 void>::Read(ThriftBuffer* buffer, void* data) {
  std::string* p = static_cast<std::string*>(data);

  return buffer->ReadString(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<bool, codec::ToUType(ThriftDataType::kBool), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const int8_t* p = static_cast<const int8_t*>(data);

  return buffer->WriteI08(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<int8_t, codec::ToUType(ThriftDataType::kI08), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const int8_t* p = static_cast<const int8_t*>(data);

  return buffer->WriteI08(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<int16_t, codec::ToUType(ThriftDataType::kI16), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const int16_t* p = static_cast<const int16_t*>(data);

  return buffer->WriteI16(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<int32_t, codec::ToUType(ThriftDataType::kI32), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const int32_t* p = static_cast<const int32_t*>(data);

  return buffer->WriteI32(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<int64_t, codec::ToUType(ThriftDataType::kI64), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const int64_t* p = static_cast<const int64_t*>(data);

  return buffer->WriteI64(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<uint64_t, codec::ToUType(ThriftDataType::kU64), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const uint64_t* p = static_cast<const uint64_t*>(data);

  return buffer->WriteU64(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<double, codec::ToUType(ThriftDataType::kDouble), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const uint64_t* p = static_cast<const uint64_t*>(data);

  return buffer->WriteU64(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kString), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const std::string* p = static_cast<const std::string*>(data);

  return buffer->WriteString(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kUtf8), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const std::string* p = static_cast<const std::string*>(data);

  return buffer->WriteString(*p);
}

template <>
inline uint32_t ThriftDescriptorImpl<std::string, codec::ToUType(ThriftDataType::kUtf16), void,
                                     void>::Write(const void* data, ThriftBuffer* buffer) {
  const std::string* p = static_cast<const std::string*>(data);

  return buffer->WriteString(*p);
}

}  // namespace trpc
#endif
