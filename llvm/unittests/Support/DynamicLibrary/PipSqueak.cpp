//===- llvm/unittest/Support/DynamicLibrary/PipSqueak.cpp -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PipSqueak.h"

struct Global {
#if INTEL_CUSTOMIZATION
  int *Str;
  std::vector<int> *Vec;
#endif // INTEL_CUSTOMIZATION
  Global() : Str(nullptr), Vec(nullptr) {}
  ~Global() {
    if (Str) {
      if (Vec)
        Vec->push_back(*Str);
      *Str = State::GLOBAL_DESTRUCTOR_CALL; // INTEL
    }
  }
};

static Global Glb;

struct Local {
#if INTEL_CUSTOMIZATION
  int &Str;
  Local(int &S) : Str(S) {
    Str = State::LOCAL_CONSTRUCTOR_CALL;
  }
  ~Local() { Str = State::LOCAL_DESTRUCTOR_CALL; }
#endif // INTEL_CUSTOMIZATION
};

#if INTEL_CUSTOMIZATION
extern "C" PIPSQUEAK_EXPORT void SetStrings(int &GStr,
                                            int &LStr) {
#endif // INTEL_CUSTOMIZATION
  Glb.Str = &GStr;
  static Local Lcl(LStr);
}

#if INTEL_CUSTOMIZATION
extern "C" PIPSQUEAK_EXPORT void TestOrder(std::vector<int> &V) {
#endif // INTEL_CUSTOMIZATION
  Glb.Vec = &V;
}

#define PIPSQUEAK_TESTA_RETURN "LibCall"
#include "ExportedFuncs.cpp"
