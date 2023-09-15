// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifndef __SERIALIZER
#define __SERIALIZER
#include "IAbstractBackendFactory.h"
#include "exceptions.h"
#include <malloc.h>
#include <map>
#include <string>

/**
 * This file contains utlity functions which will be used for serialization
 */
namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class IInputStream;
class IOutputStream;

/**
 * This class saves some flow stages in the serialization process
 */
class SerializationStatus {
public:
  SerializationStatus();

  void SetPointerMark(const std::string &mark, void *pointer);
  void *GetPointerMark(const std::string &mark);

  void SetJITAllocator(ICLDevBackendJITAllocator *pJITAllocator);
  ICLDevBackendJITAllocator *GetJITAllocator();

  void SetBackendFactory(IAbstractBackendFactory *pBackendFactory);
  IAbstractBackendFactory *GetBackendFactory();

  void SerializeVersion(IOutputStream &stream);
  void DeserialVersion(IInputStream &stream);
  int GetRuntimeVersion() const;
  int GetLLVMVersion() const;
  unsigned int m_binaryVersion = 0;

private:
  ICLDevBackendJITAllocator *m_pJITAllocator;
  IAbstractBackendFactory *m_pBackendFactory;
  int m_RuntimeVersion;
  int m_LLVMVersion;

  std::map<std::string, void *> m_marksMap;
};

/**
 * This class reperesents interface for output serialization stream
 */
class IOutputStream {
public:
  virtual IOutputStream &Write(const char *s, size_t count) = 0;
  virtual ~IOutputStream() {}
};

/**
 * This class reperesents interface for input serialization stream
 */
class IInputStream {
public:
  virtual IInputStream &Read(char *s, size_t count) = 0;
  virtual ~IInputStream() {}
};

/**
 * This class represents serialization exception which will be thrown in case
 * errors occured during serialize\deserialize
 */
DEFINE_EXCEPTION(SerializationException)

/**
 * This class consists from general methods used for serialization and it
 * can handle common data structures
 */
class Serializer {
public:
  template <class T>
  static void SerialPrimitive(const T *item, IOutputStream &stream) {
    stream.Write((const char *)item, sizeof(T));
  }

  template <class T>
  static void DeserialPrimitive(T *item, IInputStream &stream) {
    stream.Read((char *)item, sizeof(T));
  }

  template <class T>
  static void SerialNullTerminatedPrimitivesBuffer(const T *item,
                                                   IOutputStream &stream) {
    int length = 0;
    const T *elemnt = (const T *)item;
    if (NULL == item) {
      stream.Write((const char *)(&length), sizeof(int));
      return;
    }

    while ((T)NULL != *elemnt) {
      elemnt++;
      length++;
    }

    length++;
    stream.Write((const char *)&length, sizeof(length));
    stream.Write((const char *)item, sizeof(T) * length);
  }

  template <class T>
  static void DeserialNullTerminatedPrimitivesBuffer(T *&item,
                                                     IInputStream &stream) {
    int length = 0;
    stream.Read((char *)&length, sizeof(int));
    if (0 == length) {
      item = NULL;
      return;
    }

    item = (T *)malloc(sizeof(T) * length);
    if (NULL == item)
      throw Exceptions::SerializationException("Cannot Allocate Memory");

    stream.Read((char *)item, sizeof(T) * length);
  }

  template <class T>
  static void SerialPrimitivesBuffer(const T *item, unsigned int length,
                                     IOutputStream &stream) {
    stream.Write((const char *)&length, sizeof(length));
    if (0 == length)
      return;

    stream.Write((const char *)item, sizeof(T) * length);
  }

  template <class T>
  static void DeserialPrimitivesBuffer(T *&item, unsigned int &length,
                                       IInputStream &stream) {
    stream.Read((char *)&length, sizeof(length));
    if (0 == length) {
      item = NULL;
      return;
    }

    item = (T *)malloc(sizeof(T) * length);
    if (NULL == item)
      throw Exceptions::SerializationException("Cannot Allocate Memory");

    stream.Read((char *)item, sizeof(T) * length);
  }

  static void SerialString(const std::string &item, IOutputStream &stream) {
    SerialNullTerminatedPrimitivesBuffer<char>(item.c_str(), stream);
  }

  static void DeserialString(std::string &item, IInputStream &stream) {
    char *temp = NULL;
    DeserialNullTerminatedPrimitivesBuffer<char>(temp, stream);

    // KW warning
    if (temp) {
      try {
        item = std::string(temp);
      } catch (std::bad_alloc &) {
        free(temp);
        throw Exceptions::SerializationException("Cannot Allocate Memory");
      }
      free(temp);
    }
  }

  static void SerialPointerHint(const void **item, IOutputStream &stream) {
    bool isNullPointer = (NULL == item);
    SerialPrimitive<bool>(&isNullPointer, stream);
  }

  static void DeserialPointerHint(void **item, IInputStream &stream) {
    bool isNullPointer = false;
    DeserialPrimitive<bool>(&isNullPointer, stream);
    *item = reinterpret_cast<void *>((intptr_t)!isNullPointer);
  }
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __SERIALIZER
