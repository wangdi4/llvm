/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BWOptions.cpp

\*****************************************************************************/
#include "BWOptions.h"

#if defined(__LP64__)
#include <sys/mman.h>
#endif

JITAllocator *JITAllocator::s_pInstance = NULL;

void JITAllocator::Init() {
  assert(!s_pInstance);
  s_pInstance = new JITAllocator();
}

void JITAllocator::Terminate() {
  if (NULL != s_pInstance) {
    delete s_pInstance;
    s_pInstance = NULL;
  }
}

JITAllocator *JITAllocator::GetInstance() {
  assert(s_pInstance);
  return s_pInstance;
}

void *JITAllocator::AllocateExecutable(size_t size, size_t alignment) {
  size_t required_size = (size % PAGE_SIZE == 0)
                             ? size
                             : ((size_t)(size / PAGE_SIZE) + 1) * PAGE_SIZE;

  size_t aligned_size =
      required_size +   // required size
      (alignment - 1) + // for alignment
      sizeof(void *) +  // for the free ptr
      sizeof(size_t);   // to save the original size (for mprotect)
  void *pMem = malloc(aligned_size);
  if (NULL == pMem)
    return NULL;

  char *pAligned = ((char *)pMem) + aligned_size - required_size;
  pAligned = (char *)(((size_t)pAligned) & ~(alignment - 1));
  ((void **)pAligned)[-1] = pMem;
  void *pSize = (void *)(((char *)pAligned) - sizeof(void *));
  ((size_t *)pSize)[-1] = required_size;

#if defined(__LP64__)
  int ret = mprotect((void *)pAligned, required_size,
                     PROT_READ | PROT_WRITE | PROT_EXEC);
  if (0 != ret) {
    free(pMem);
    return NULL;
  }
#else
  assert(false && "Not implemented");
#endif

  return pAligned;
}

void JITAllocator::FreeExecutable(void *ptr) {
  void *pMem = ((void **)ptr)[-1];

#if defined(__LP64__)
  void *pSize = (void *)(((char *)ptr) - sizeof(void *));
  size_t size = ((size_t *)pSize)[-1];
  mprotect((void *)ptr, size, PROT_READ | PROT_WRITE);
#else
  assert(false && "Not implemented");
#endif

  free(pMem);
}
