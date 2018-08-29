// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
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

#ifndef __BLOCK_LITERAL_H__
#define __BLOCK_LITERAL_H__

#include "llvm/ADT/APInt.h"
#include "llvm/Support/Compiler.h"
#include <assert.h>
#include <string.h>
#include "cl_sys_defines.h"

/*
  LLVM example of block literal:
  -----------------------------------------------------
                                  size alignment
                                    |    |
  %struct.__block_literal = type { i32, i32, ...}
                                              |
                                     captured variables
  -----------------------------------------------------
*/

// TODO: block literal structure got quite simple representation starting from
// clang 6.0 version, so this class may be further simplified or removed
namespace Intel { namespace OpenCL { namespace DeviceBackend {

  struct BlockLiteral {
  private:
    int32_t size;
    int32_t alignment;

    /// imported (captured) variables go here
    /// int var1;
    /// float var;
  public:
    /// @brief Serialize block_literal and blockdescriptor to destination address
    /// @param dst - destination memory buffer pre-allocated with GetLiteralAndDescriptorSize() 
    void Serialize(void *dst, size_t dst_size) const {
      MEMCPY_S(dst, dst_size, this, size);
    }

    /// @brief Deserialize BlockLiteral inplace in memory 
    /// @param src - memory address where serialized BlockLiteral is stored
    /// @return - ptr to correct BlockLiteral structure
    static BlockLiteral * DeserializeInBuffer(void *src) {
      return reinterpret_cast<BlockLiteral *>(src);
    }
    
    /// @brief clones BlockLiteral. Allocates memory and creates copy
    /// @param src - source BlockLiteral
    /// @return created BlockLiteral
    static BlockLiteral * Clone(const BlockLiteral *src) {
      assert(src && "src is NULL");
      const size_t SizeBytes = src->GetSize();
      char * mem = new char[SizeBytes];
      src->Serialize(mem, SizeBytes);
      return DeserializeInBuffer(mem);
    }
    
    /// @brief free memory for BlockLiteral allocated by Clone()
    /// @param p - valid BlockLiteral object created by Clone()
    static void FreeMem(BlockLiteral * p) {
      assert(p && "p is NULL");
      delete [] p;
    }

    int64_t GetSize() const {
      return size;
    }

  private:
    ////////////////////////////////////////////////////////////////
    /// hide default ctor
    BlockLiteral() = delete;
    /// hide copy ctor
    BlockLiteral(const BlockLiteral& s) = delete;
    /// hide assignment
    void operator =(BlockLiteral&) = delete ;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __BLOCK_LITERAL_H__
