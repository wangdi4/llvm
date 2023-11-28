// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#pragma once

#include "cl_framework.h"
#include "cl_shared_ptr.h"
#include "cl_types.h"
#include <map>

using Intel::OpenCL::Utils::SharedPtr;

namespace Intel {
namespace OpenCL {
namespace Framework {

// define Graphics system sharing type
enum tGfxSharing {
  CL_MEMOBJ_GFX_SHARE_NONE = 0,
  CL_MEMOBJ_GFX_SHARE_GL = 1,
  CL_MEMOBJ_GFX_SHARE_DX9 = 2,
  CL_MEMOBJ_GFX_SHARE_DX10 = 4,
  CL_MEMOBJ_GFX_SHARE_DX11 = 8
};

class Context;
class MemoryObject;

#define REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL(                               \
    SUPPORTED_DEVICES, GFX_SHARE, OBJECT_TYPE, AUX_ID, CLASS, IMPLEMETATION)   \
  struct CLASS##CreatorRegister {                                              \
    CLASS##CreatorRegister() {                                                 \
      MemoryObjectFactory::GetInstance()->RegisterMemoryObjectCreator(         \
          SUPPORTED_DEVICES, GFX_SHARE, OBJECT_TYPE, AUX_ID,                   \
          &CLASS##CreatorRegister::Create);                                    \
    }                                                                          \
    static SharedPtr<MemoryObject> Create(SharedPtr<Context> pContext,         \
                                          cl_mem_object_type clObjType) {      \
      /* This is to prevent class B that inherits from A to not define         \
       * Allocate, while A does. This would compile if we just used 'return',  \
       * but we'd get an object of class A instead of one of type B */         \
      SharedPtr<IMPLEMETATION> pMemObj =                                       \
          IMPLEMETATION::Allocate(pContext, clObjType);                        \
      return pMemObj;                                                          \
    }                                                                          \
  };                                                                           \
  CLASS##CreatorRegister class##CLASS##CreatorRegister;

typedef SharedPtr<MemoryObject>
fn_MemoryObjectCreator(SharedPtr<Context> pContext,
                       cl_mem_object_type clObjType);

class MemoryObjectFactory {
public:
  static MemoryObjectFactory *GetInstance();
  static void Destroy() {
    if (Instance)
      delete Instance;
  }

  void RegisterMemoryObjectCreator(cl_bitfield iSupportedDevices,
                                   int iGfxSysSharing,
                                   cl_mem_object_type clObjType, int iAuxId,
                                   fn_MemoryObjectCreator *pMemObjCreator);

  cl_err_code
  CreateMemoryObject(cl_bitfield iRequiredDevices, cl_mem_object_type clObjType,
                     int iGfxSysSharing, SharedPtr<Context> pContext,
                     SharedPtr<MemoryObject> *pMemObject, int iAuxId = 0);

protected:
  struct FactoryKey {
    cl_mem_object_type clObjType;
    cl_bitfield iSupportedDevices;
    int iGfxSysSharing;
    int iAuxId; // an auxiliary ID for registering different creators for the
                // same object type

    bool operator<(const FactoryKey &_Right) const;
  };

  std::map<FactoryKey, fn_MemoryObjectCreator *> m_memObjMap;

private:
  static MemoryObjectFactory *Instance;
};

// This macro level constructs an object creator class name as a concatenation
// of a memory object class name and random number
#define REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP(                          \
    SUPPORTED_DEVICES, GFX_SHARE, OBJECT_TYPE, AUX_ID, CLASS, NUMBER)          \
  REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL(SUPPORTED_DEVICES, GFX_SHARE,        \
                                          OBJECT_TYPE, AUX_ID,                 \
                                          CLASS##_##NUMBER##_, CLASS)

// This macro level just desacralizes __LINE__ macro and converts it into simple
// number
#define REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP1(                         \
    SUPPORTED_DEVICES, GFX_SHARE, OBJECT_TYPE, AUX_ID, CLASS, NUMBER)          \
  REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP(                                \
      SUPPORTED_DEVICES, GFX_SHARE, OBJECT_TYPE, AUX_ID, CLASS, NUMBER)

// This macro level is required to add a __LINE__ macro operand
#define REGISTER_MEMORY_OBJECT_CREATOR(SUPPORTED_DEVICES, GFX_SHARE,           \
                                       OBJECT_TYPE, AUX_ID, CLASS)             \
  REGISTER_MEMORY_OBJECT_CREATOR_INTERNAL_WRAP1(                               \
      SUPPORTED_DEVICES, GFX_SHARE, OBJECT_TYPE, AUX_ID, CLASS, __LINE__)

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
