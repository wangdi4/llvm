/*****************************************************************************\

Copyright (c) Intel Corporation (2013 - 2018).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  BlockLiteral.h

\*****************************************************************************/
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
