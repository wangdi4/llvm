/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

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

/*
  Declartion of Clang's block literal structure. Used in OCL20 Extended execution
  Detailed doc is here:http://clang.llvm.org/docs/Block-ABI-Apple.html

  LLVM example of block literal
  %struct.__block_descriptor.10 = type { i64, i64 }
  %struct.__block_literal_generic.11 = type { i8*, i32, i32, i8*, %struct.__block_descriptor.10* }

  Block_literal contains following main things:
    - invoke point of block function
    - size of Block_literal 
    - imported variables
  
  !!! Do not use sizeof() since block_literal may also contain imported variables

*/
namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// structure representing clang's block_literal header
  /// imported variables are located in memory after block_descriptor
  /// clang generates automatically fields for imported variables 
  /// this struct is not covering imported variable
  /// Intended use:
  ///     static_cast pointer to BlockLiteral *
  ///     access invoke addr
  ///     access size 
  ///     copy to another memory location
  /// All other usages are hidden to avoid misproper copy, create, etc ops
  struct BlockLiteral {
  private:
    /// initialized to &_NSConcreteStackBlock or &_NSConcreteGlobalBlock
    void *isa; 
    int flags;
    int reserved;
    /// invoke address of Block function
    void *invoke; 
    struct Block_descriptor {
      int64_t reserved;
      /// size of BlockLiteral structure
      int64_t size;
      /// ctor
      Block_descriptor():size(0){}
    };
    Block_descriptor *desc;
    /// imported variables go here
    /// int var1;
    /// float var;
  public:
    /// @brief obtain size in bytes of BlockLiteral structure and Block descriptor structure
    /// intended use is to obtain size for allocating memory for copying structure
    size_t GetBufferSizeForSerialization() const {
      assert(desc->size != 0 && 
        "BlockLiteral::GetBufferSizeForSerialization size of block_literal is zero");
      return static_cast<size_t>(desc->size) + sizeof(Block_descriptor);
    }
    
    /// @brief Serialize block_literal and blockdescriptor to destination address
    /// Deserialization is done by byte copying buffer and calling UpdateBlockDescrPtr() function
    /// It places in memory fixed size blockdescriptor and block_literal after it 
    /// @param dst - destination memory buffer pre-allocated with GetLiteralAndDescriptorSize() 
    void Serialize(void *dst) const {
      // copy Block_descriptor
      ::memcpy(dst, desc, sizeof(Block_descriptor));
      ((Block_descriptor*)dst)->reserved = BLOCKLITERAL_ID;
      // copy BlockLiteral
      void * newDescAddr = static_cast<char*>(dst) + sizeof(Block_descriptor);
      ::memcpy(newDescAddr, this, desc->size);
    }

    /// @brief Deserialize BlockLiteral inplace in memory 
    /// @param src - memory address where serialized BlockLiteral is stored
    /// @return - ptr to correct BlockLiteral structure
    static BlockLiteral * DeserializeInBuffer(void *src) {
      assert(((Block_descriptor*)src)->reserved == BLOCKLITERAL_ID &&
        "BLOCKLITERAL_ID signature was not found ");
      BlockLiteral * Addr = reinterpret_cast<BlockLiteral *>
        (reinterpret_cast<char*>(src) + sizeof(Block_descriptor));
      Addr->desc = reinterpret_cast<Block_descriptor*>(src);
      return Addr;
    }
    
    /// @brief clones BlockLiteral. Allocates memory and creates copy
    /// @param src - source BlockLiteral
    /// @return created BlockLiteral
    static BlockLiteral * Clone(const BlockLiteral *src) {
      assert(src && "src is NULL");
      const size_t SizeBytes = src->GetBufferSizeForSerialization();
      char * mem = new char[SizeBytes];
      src->Serialize(mem);
      BlockLiteral * pBL = DeserializeInBuffer(mem);
      return pBL;
    }
    
    /// @brief free memory for BlockLiteral allocated by Clone()
    /// @param p - valid BlockLiteral object created by Clone()
    static void FreeMem(BlockLiteral * p) {
      assert(p && "p is NULL");
      assert(p->desc->reserved == BLOCKLITERAL_ID &&
        "BLOCKLITERAL_ID signature was not found. "
        "Seems this BlockLiteral was not created by Clone()."
        "We can free memory only for BlockLiteral created by Clone())");
      // move back pointer by size of Block_descriptor. It is stored before
      char * ptr = reinterpret_cast<char*>(p) - sizeof(Block_descriptor);
      delete [] ptr;
    }

    /// @brief get block function invoke address
    void * GetInvoke() const {
      return invoke;
    }

    /// ID of blockLiteral serialized copy
    static const int64_t BLOCKLITERAL_ID = 0xBFE04725;
  private:
    ////////////////////////////////////////////////////////////////
    /// hide default ctor
    BlockLiteral() LLVM_DELETED_FUNCTION;
    /// hide copy ctor
    BlockLiteral(const BlockLiteral& s) LLVM_DELETED_FUNCTION;
    /// hide assignment
    void operator =(BlockLiteral&) LLVM_DELETED_FUNCTION;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __BLOCK_LITERAL_H__
