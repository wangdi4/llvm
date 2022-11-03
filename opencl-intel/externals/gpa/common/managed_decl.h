/******************************************************************************

                              INTEL CONFIDENTIAL
           Copyright (C) 2008 Intel Corporation All Rights Reserved.

 The source code contained or described herein and all documents related to the
 source code ("Material") are owned by Intel Corporation or its suppliers or
 licensors. Title to the Material remains with Intel Corporation or its
 suppliers and licensors. The Material contains trade secrets and proprietary
 and confidential information of Intel or its suppliers and licensors. The
 Material is protected by worldwide copyright and trade secret laws and treaty
 provisions. No part of the Material may be used, copied, reproduced, modified,
 published, uploaded, posted, transmitted, distributed, or disclosed in any way
 without Intel’s prior express written permission.

 No license under any patent, copyright, trade secret or other intellectual
 property right is granted to or conferred upon you by disclosure or delivery
 of the Materials, either expressly, by implication, inducement, estoppel or
 otherwise. Any license under such intellectual property rights must be
 express and approved by Intel in writing.

******************************************************************************/
#if !defined(__MANAGED_DECL_H__) || defined(INHIBIT_MANAGED_DECL)
#define __MANAGED_DECL_H__

///////////////////////////////////////////////////////////////////////////////
/// @file
/// Macros that allow us to declare managed and unmanaged versions of the same
/// struct ane enum types without duplicating the contents of the declarations.
///

#ifdef BEGIN_ENUM_DECL
#undef BEGIN_ENUM_DECL
#endif
#ifdef END_ENUM_DECL
#undef END_ENUM_DECL
#endif
#ifdef BEGIN_STRUCT_DECL
#undef BEGIN_STRUCT_DECL
#endif
#ifdef BEGIN_ANSI_STRUCT_DECL
#undef BEGIN_ANSI_STRUCT_DECL
#endif
#ifdef BEGIN_UNION_STRUCT_DECL
#undef BEGIN_UNION_STRUCT_DECL
#endif
#ifdef END_STRUCT_DECL
#undef END_STRUCT_DECL
#endif
#ifdef BEGIN_ENUM_DECL_WITH_BASE_TYPE
#undef BEGIN_ENUM_DECL_WITH_BASE_TYPE
#endif
#ifdef GT_CHAR_ARRAY_DECL
#undef GT_CHAR_ARRAY_DECL
#endif
#ifdef GT_UINT32_ARRAY_DECL
#undef GT_UINT32_ARRAY_DECL
#endif

#if defined(_MANAGED) && !defined(INHIBIT_MANAGED_DECL)
using namespace System;
using namespace System::Runtime::InteropServices;
#define BEGIN_ENUM_DECL(name)                                                  \
public                                                                         \
  enum class name
#define BEGIN_ENUM_DECL_WITH_BASE_TYPE(name, basetype)                         \
public                                                                         \
  enum class name : basetype
#define END_ENUM_DECL(name) ;
#define BEGIN_STRUCT_DECL(name)                                                \
  [StructLayout(LayoutKind::Sequential, CharSet = CharSet::Unicode,            \
                Pack = 1)] public value struct name
#define BEGIN_ANSI_STRUCT_DECL(name)                                           \
  [StructLayout(LayoutKind::Sequential, CharSet = CharSet::Ansi,               \
                Pack = 1)] public value struct name
#define ASSIGN_OFFSET(X) [System.Runtime.InteropServices.FieldOffset(X)]
#define END_STRUCT_DECL(name) ;

#define GT_CHAR_ARRAY_DECL(name, size)                                         \
  [MarshalAs(UnmanagedType::ByValTStr, SizeConst = size)] String ^ name;
#define GT_UINT32_ARRAY_DECL(name, size)                                       \
  [MarshalAs(UnmanagedType::ByValArray, ArraySubType = UnmanagedType::U4,      \
             SizeConst = size)] array<GT_UINT32> ^                             \
      name;

#else // !_MANAGED  && !defined(INHIBIT_MANAGED_DECL)

#define BEGIN_ENUM_DECL(name) typedef enum _##name
#define BEGIN_ENUM_DECL_WITH_BASE_TYPE(name, basetype) BEGIN_ENUM_DECL(name)
#define END_ENUM_DECL(name) name;
#define BEGIN_STRUCT_DECL(name) typedef struct _##name
#define BEGIN_ANSI_STRUCT_DECL(name) typedef struct _##name
#define BEGIN_UNION_STRUCT_DECL(name) typedef struct _##name
#define END_STRUCT_DECL(name) name;

#define GT_CHAR_ARRAY_DECL(name, size) wchar_t name[size];
#define GT_UINT32_ARRAY_DECL(name, size) GT_UINT32 name[size];

#endif // !_MANAGED  && ! defined(INHIBIT_MANAGED_DECL)

#ifdef GT_MANAGED_EXCEPTION
#undef BEGIN_ENUM
#define BEGIN_ENUM(name)                                                       \
public                                                                         \
  enum class MANAGED_##name
#undef END_ENUM
#define END_ENUM(name) ;
#undef ENUM_ITEM
#define ENUM_ITEM(name) name,

#else // !GT_MANAGED_EXCEPTION
#define BEGIN_ENUM(name) typedef enum _GT_##name
#define END_ENUM(name) GT_##name;
#define ENUM_ITEM(name) GT_ERROR_##name,
#endif // !GT_MANAGED_EXCEPTION

#endif //__MANAGED_DECL_H__ || defined(INHIBIT_MANAGED_DECL)

/******************************************************************************
** EOF
******************************************************************************/
