// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef __LOGGER__H__
#define __LOGGER__H__

#include "llvm/Support/raw_ostream.h"

#ifdef WIN_DEBUG

#include <stdlib.h>
extern FILE *prtFile;
extern FILE *moduleDmp;

#define V_INIT_PRINT                                                           \
  {                                                                            \
    prtFile = fopen("vectorizer.txt", "a");                                    \
    moduleDmp = fopen("module.txt", "a");                                      \
  }
#define V_DESTROY_PRINT                                                        \
  {                                                                            \
    fclose(prtFile);                                                           \
    fclose(moduleDmp);                                                         \
  }
#define V_PRINT(type, x)                                                       \
  {                                                                            \
    std::string tmpStr;                                                        \
    llvm::raw_string_ostream strstr(tmpStr);                                   \
    strstr << x;                                                               \
    fprintf(prtFile, "%s", strstr.str().c_str());                              \
    fflush(prtFile);                                                           \
  }
#define V_DUMP(ptr)                                                            \
  {                                                                            \
    std::string tmpStr;                                                        \
    llvm::raw_string_ostream strstr(tmpStr);                                   \
    ptr->print(strstr, nullptr);                                               \
    fprintf(prtFile, "%s", strstr.str().c_str());                              \
    fflush(prtFile);                                                           \
  }
#define V_DUMP_MODULE(ptr)                                                     \
  {                                                                            \
    std::string tmpStr;                                                        \
    llvm::raw_string_ostream strstr(tmpStr);                                   \
    ptr->print(strstr, nullptr);                                               \
    fprintf(moduleDmp, "%s", strstr.str().c_str());                            \
    fflush(moduleDmp);                                                         \
  }
#define V_ASSERT(x)                                                            \
  {                                                                            \
    if (!(x)) {                                                                \
      V_PRINT(assert, "Assertion in: " << __FILE__ << ":" << __LINE__ << ": "  \
                                       << #x << "\n");                         \
      assert(0);                                                               \
    }                                                                          \
  }

#else

#include "llvm/Support/Debug.h"
#define V_INIT_PRINT
#define V_DESTROY_PRINT
#ifndef NDEBUG
#define V_PRINT(type, x) DEBUG_WITH_TYPE(#type, errs() << x)
#else
#define V_PRINT(type, x)                                                       \
  do {                                                                         \
  } while (0)
#endif
#define V_DUMP(ptr)
#define V_DUMP_MODULE(ptr)
#define V_ASSERT(x) assert(x)

#endif

#ifndef NDEBUG
#define V_STAT(x) x
#else
#define V_STAT(x)
#endif

#endif // __LOGGER__H__
