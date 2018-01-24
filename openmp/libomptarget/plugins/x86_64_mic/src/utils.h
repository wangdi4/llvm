//===----- plugins/xeon-phi-coi/utils.h - Device executable -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Utilites shared between host and target parts of plugin.
///
//===----------------------------------------------------------------------===//

#ifndef _XEON_PHI_COI_UTILS_H_
#define _XEON_PHI_COI_UTILS_H_

#include <algorithm>
#include <assert.h>
#include <dlfcn.h>

class DynLibTy {
public:
  DynLibTy() : Handle(nullptr) {}

  explicit DynLibTy(const char *FileName, int Flags = RTLD_LAZY) {
    Handle = dlopen(FileName, Flags);
  }

  DynLibTy(DynLibTy && That) : DynLibTy() {
    std::swap(Handle, That.Handle);
  }

  ~DynLibTy() {
    // TODO: Do not close library handle for now because it is causing a
    // segfault originating from libcoi_host.so at plugin cleanup (needs
    // further investigation). This class is currently used only for loading
    // COI on the host side. We will get rid of it once we start linking
    // libcoi_host.so to the plugin instead of loading it at runtime.
    //close();
  }

  DynLibTy& operator=(DynLibTy && That) {
    if (Handle != That.Handle) {
      close();
    }
    Handle = That.Handle;
    That.Handle = nullptr;
    return *this;
  }

  void close() {
    if (Handle) {
      dlclose(Handle);
      Handle = nullptr;
    }
  }

  operator bool() const { return Handle != nullptr; }

  template<typename T = void>
  T* getSymbol(const char *Name, const char *Version = nullptr) const {
    assert(Handle != nullptr);
    union {
      T* Typed;
      void *Void;
    } R;
    R.Void = Version ? dlvsym(Handle, Name, Version) : dlsym(Handle, Name);
    return R.Typed;
  }

  // Disable copy from lvalue reference
  DynLibTy(const DynLibTy&) = delete;
  DynLibTy& operator=(const DynLibTy &) = delete;

private:
  void *Handle;
};

#endif // _XEON_PHI_COI_UTILS_H_

