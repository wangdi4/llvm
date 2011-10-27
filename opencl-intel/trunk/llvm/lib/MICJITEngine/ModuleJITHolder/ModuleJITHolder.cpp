//===-- ModuleJITHolder.cpp - Contains for module JIT -----------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file implements container for complete module JIT, including binary code
// and contained function information. Used for MIC.
//
//===----------------------------------------------------------------------===//

#include "llvm/MICJITEngine/ModuleJITHolder.h"
#include <malloc.h>

namespace llvm {
class Function;

ModuleJITHolder::ModuleJITHolder(void *buf, unsigned char *code,
      unsigned int size, unsigned int align) :
    codeBuf(buf), codePtr(code), codeSize(size), codeAlignment(align) {}

ModuleJITHolder::~ModuleJITHolder() {
}

void ModuleJITHolder::addKernel(const Function *F, size_t offset, size_t size)
{
  KernelInfo &KI = kernelMap[F];
  KI.size = size;
  KI.offset = offset;
}

size_t ModuleJITHolder::getJITCodeSize() const  {
  return codeSize;
}

size_t ModuleJITHolder::getJITCodeAlignment() const  {
  return codeAlignment;
}

const void* ModuleJITHolder::getJITCodeStartPoint() const  {
  return codePtr;
}

void* ModuleJITHolder::getJITBufferPointer() const  {
  return codeBuf;
}

size_t ModuleJITHolder::getKernelCount() const {
  return kernelMap.size();
}

size_t ModuleJITHolder::getKernelSize(const Function *F) const {
  KernelMap::const_iterator I = kernelMap.find(F);
  if (I == kernelMap.end())
    return 0;
  return I->second.size;
}


size_t ModuleJITHolder::getKernelOffset(const Function *F) const {
  KernelMap::const_iterator I = kernelMap.find(F);
  if (I == kernelMap.end())
    return 0;
  return I->second.offset;
}

const KernelMap & ModuleJITHolder::getKernelMap() const {
  return kernelMap;
}



ModuleJITStore* ModuleJITStore::instancePtr = NULL;

ModuleJITStore* ModuleJITStore::instance()
{
  if (!instancePtr)
    instancePtr = new ModuleJITStore;

  return instancePtr;
}

void ModuleJITStore::addModuleJit (Module *M, ModuleJITHolder *J)
{
  assert(moduleJITMap.find(M) == moduleJITMap.end() && 
      "Module doesn't exist in the store");
  moduleJITMap[M] = J;
}

const ModuleJITHolder *ModuleJITStore::retrieveModule(Module *M)
{
  const ModuleJITHolder *moduleJIT;
  ModuleJITMap_t::const_iterator VI = moduleJITMap.find(M);
  assert(VI != moduleJITMap.end() && "Module doesn't exist in the store");
  moduleJIT = VI->second;
  moduleJITMap.erase(M);
  return moduleJIT;
}

}
