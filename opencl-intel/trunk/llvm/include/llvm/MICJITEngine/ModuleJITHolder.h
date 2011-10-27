//===-- ModuleJITHolder.h - Contains for module JIT -------------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file defines interface for holding complete module JIT, including 
// binary code and contained function information. Used for MIC.
//
//===----------------------------------------------------------------------===//

#ifndef __MODULE_JIT_HOLDER_H__
#define __MODULE_JIT_HOLDER_H__

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/ValueMap.h"

namespace llvm {
class Function;
class Module;

//===--------------------------------------------------------------------===//
// KernelInfo - Kernel information. Kernel size and offset fro JIT buffer start
//
struct KernelInfo {
  size_t size;
  size_t offset;
};

typedef DenseMap <const Function *, KernelInfo> KernelMap;

//===--------------------------------------------------------------------===//
// ModuleJITHolder - container for jitted module: the bit code and a list of
// contained kernels.
//
class ModuleJITHolder {
public:
  ModuleJITHolder(void *buf, unsigned char *code, unsigned int size,
               unsigned int align);

  ~ModuleJITHolder();

  void addKernel(const Function * F, size_t offset, size_t size);

  bool verify() const;

  ///////////////////////////////////////////////
  // return: The size of the code binaries in bytes.
  ///////////////////////////////////////////////
  size_t getJITCodeSize() const;

  ///////////////////////////////////////////////
  // return: The required code alignment when executed
  ///////////////////////////////////////////////
  size_t getJITCodeAlignment() const;

  ///////////////////////////////////////////////
  // return: pointer to the code binaries.
  ///////////////////////////////////////////////
  const void* getJITCodeStartPoint() const;

  ///////////////////////////////////////////////
  // return: pointer to the buffer that holds the code.
  ///////////////////////////////////////////////
  void* getJITBufferPointer() const;

  ///////////////////////////////////////////////
  // return: the number of kernels held by this object
  ///////////////////////////////////////////////
  size_t getKernelCount() const;

  ///////////////////////////////////////////////////////////////
  // return: the size(in bytes) of the kernel with the given id
  ///////////////////////////////////////////////////////////////
  size_t getKernelSize(const Function *) const;

  ///////////////////////////////////////////////////////////////
  // return: the offset(in bytes) of the kernel with the given id,
  // relative to the binaries buffer.
  ///////////////////////////////////////////////////////////////
  size_t getKernelOffset(const Function *) const;

  const KernelMap & getKernelMap() const;

private:
    // address of buffer that conpatins the code. used only for freeing memory
  void *codeBuf;

    // pointer to first byte of code, code size and required alignment on the 
    // device
  void *codePtr;
  size_t codeSize;
  size_t codeAlignment;

  KernelMap kernelMap;
};



//===--------------------------------------------------------------------===//
// ModuleJITStore - container for jitted modules since generated until
// retrived by the client
//
class ModuleJITStore
{
  public:
    static ModuleJITStore *instance();
    // Add ModuleJITHolder to store
    void addModuleJit (Module *M, ModuleJITHolder *J);
    // Get ownership over ModuleJITHolder from store
    const ModuleJITHolder *retrieveModule(Module *M);

  private:
    ModuleJITStore() {}
    ModuleJITStore(ModuleJITStore const&);
    ModuleJITStore& operator=(ModuleJITStore const&);
    ~ModuleJITStore();

    static ModuleJITStore* instancePtr;

    // TODO: this is not thread safe. it must be replaced with 
    // thread safe mechanism
    typedef DenseMap<const Module *, const ModuleJITHolder *> ModuleJITMap_t;
    ModuleJITMap_t moduleJITMap;
};

}

#endif //__MODULE_JIT_HOLDER_H__
