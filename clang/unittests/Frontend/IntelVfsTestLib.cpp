//===------ unittests/Frontend/IntelVfsTestLib.cpp - CI tests -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#if defined(_WIN32) && !defined(__GNUC__)
// Disable warnings from inclusion of xlocale & exception
#pragma warning(push)
#pragma warning(disable: 4530)
#pragma warning(disable: 4577)
#include <string>
#include <vector>
#pragma warning(pop)
#endif

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/VirtualFileSystem.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" EXPORT llvm::vfs::FileSystem *__clang_create_vfs() {
  auto *FS = new llvm::vfs::InMemoryFileSystem();

  llvm::SmallString<128> FileName("virtual.file");
  std::error_code EC = llvm::sys::fs::make_absolute(FileName);
  if (EC)
    return nullptr;

  FS->addFile(FileName, (time_t)0, llvm::MemoryBuffer::getMemBuffer("foobar"));

  return FS;
}
