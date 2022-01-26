//===- llvm/unittest/Support/DynamicLibrary/DynamicLibraryTest.cpp --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Config/config.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "gtest/gtest.h"

#include "PipSqueak.h"

using namespace llvm;
using namespace llvm::sys;

std::string LibPath(const std::string Name = "PipSqueak") {
  const auto &Argvs = testing::internal::GetArgvs();
  const char *Argv0 =
      Argvs.size() > 0 ? Argvs[0].c_str() : "DynamicLibraryTests";
  void *Ptr = (void*)(intptr_t)TestA;
  std::string Path = fs::getMainExecutable(Argv0, Ptr);
  llvm::SmallString<256> Buf(path::parent_path(Path));
  path::append(Buf, (Name + LLVM_PLUGIN_EXT).c_str());
  return std::string(Buf.str());
}

#if defined(_WIN32) || (defined(HAVE_DLFCN_H) && defined(HAVE_DLOPEN))

#if INTEL_CUSTOMIZATION
typedef void (*SetStrings)(int &GStr, int &LStr);
typedef void (*TestOrder)(std::vector<int> &V);
#endif // INTEL_CUSTOMIZATION
typedef const char *(*GetString)();

template <class T> static T FuncPtr(void *Ptr) {
  union {
    T F;
    void *P;
  } Tmp;
  Tmp.P = Ptr;
  return Tmp.F;
}
template <class T> static void* PtrFunc(T *Func) {
  union {
    T *F;
    void *P;
  } Tmp;
  Tmp.F = Func;
  return Tmp.P;
}

static const char *OverloadTestA() { return "OverloadCall"; }

std::string StdString(const char *Ptr) { return Ptr ? Ptr : ""; }

TEST(DynamicLibrary, Overload) {
  {
    std::string Err;
    llvm_shutdown_obj Shutdown;
    DynamicLibrary DL =
        DynamicLibrary::getPermanentLibrary(LibPath().c_str(), &Err);
    EXPECT_TRUE(DL.isValid());
    EXPECT_TRUE(Err.empty());

    GetString GS = FuncPtr<GetString>(DL.getAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_NE(GS, &TestA);
    EXPECT_EQ(StdString(GS()), "LibCall");

    GS = FuncPtr<GetString>(DynamicLibrary::SearchForAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_NE(GS, &TestA);
    EXPECT_EQ(StdString(GS()), "LibCall");

    DL = DynamicLibrary::getPermanentLibrary(nullptr, &Err);
    EXPECT_TRUE(DL.isValid());
    EXPECT_TRUE(Err.empty());

    // Test overloading local symbols does not occur by default
    GS = FuncPtr<GetString>(DynamicLibrary::SearchForAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_EQ(GS, &TestA);
    EXPECT_EQ(StdString(GS()), "ProcessCall");

    GS = FuncPtr<GetString>(DL.getAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_EQ(GS, &TestA);
    EXPECT_EQ(StdString(GS()), "ProcessCall");

    // Test overloading by forcing library priority when searching for a symbol
    DynamicLibrary::SearchOrder = DynamicLibrary::SO_LoadedFirst;
    GS = FuncPtr<GetString>(DynamicLibrary::SearchForAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_NE(GS, &TestA);
    EXPECT_EQ(StdString(GS()), "LibCall");

    DynamicLibrary::AddSymbol("TestA", PtrFunc(&OverloadTestA));
    GS = FuncPtr<GetString>(DL.getAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_NE(GS, &OverloadTestA);

    GS = FuncPtr<GetString>(DynamicLibrary::SearchForAddressOfSymbol("TestA"));
    EXPECT_NE(GS, nullptr);
    EXPECT_EQ(GS, &OverloadTestA);
    EXPECT_EQ(StdString(GS()), "OverloadCall");
  }
  EXPECT_TRUE(FuncPtr<GetString>(DynamicLibrary::SearchForAddressOfSymbol(
                  "TestA")) == nullptr);

  // Check serach ordering is reset to default after call to llvm_shutdown
  EXPECT_EQ(DynamicLibrary::SearchOrder, DynamicLibrary::SO_Linker);
}

TEST(DynamicLibrary, Shutdown) {
#if INTEL_CUSTOMIZATION
  std::string A_lib("PipSqueak"), C_lib("SecondLib");
  int A = State::FIRST_CONSTANT, B = 0, C = State::SECOND_CONSTANT;
  std::vector<int> Order;
  Order.reserve(2);
#endif // INTEL_CUSTOMIZATION
  {
    std::string Err;
    llvm_shutdown_obj Shutdown;
    DynamicLibrary DL =
        DynamicLibrary::getPermanentLibrary(LibPath(A_lib).c_str(), &Err); // INTEL
    EXPECT_TRUE(DL.isValid());
    EXPECT_TRUE(Err.empty());

    SetStrings SS_0 = FuncPtr<SetStrings>(
        DynamicLibrary::SearchForAddressOfSymbol("SetStrings"));
    EXPECT_NE(SS_0, nullptr);

    SS_0(A, B);
    EXPECT_TRUE(B == State::LOCAL_CONSTRUCTOR_CALL); // INTEL

    TestOrder TO_0 = FuncPtr<TestOrder>(
        DynamicLibrary::SearchForAddressOfSymbol("TestOrder"));
<<<<<<< HEAD
    EXPECT_TRUE(TO_0 != nullptr);
=======
    EXPECT_NE(TO_0, nullptr);
>>>>>>> 38ac4093d9d2ae28d631ca1cc5802533989165c5

    DynamicLibrary DL2 =
        DynamicLibrary::getPermanentLibrary(LibPath(C_lib).c_str(), &Err); // INTEL
    EXPECT_TRUE(DL2.isValid());
    EXPECT_TRUE(Err.empty());

    // Should find latest version of symbols in SecondLib
    SetStrings SS_1 = FuncPtr<SetStrings>(
        DynamicLibrary::SearchForAddressOfSymbol("SetStrings"));
    EXPECT_NE(SS_1, nullptr);
    EXPECT_NE(SS_0, SS_1);

    TestOrder TO_1 = FuncPtr<TestOrder>(
        DynamicLibrary::SearchForAddressOfSymbol("TestOrder"));
    EXPECT_NE(TO_1, nullptr);
    EXPECT_NE(TO_0, TO_1);

    B = 0;
    SS_1(C, B);
    EXPECT_TRUE(B == State::LOCAL_CONSTRUCTOR_CALL); // INTEL

    TO_0(Order);
    TO_1(Order);
  }
<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
  EXPECT_TRUE(A == State::GLOBAL_DESTRUCTOR_CALL);
  EXPECT_TRUE(B == State::LOCAL_DESTRUCTOR_CALL);
#endif // INTEL_CUSTOMIZATION

  EXPECT_TRUE(FuncPtr<SetStrings>(DynamicLibrary::SearchForAddressOfSymbol(
                  "SetStrings")) == nullptr);
=======
  EXPECT_EQ(A, "Global::~Global");
  EXPECT_EQ(B, "Local::~Local");
  EXPECT_EQ(FuncPtr<SetStrings>(
                DynamicLibrary::SearchForAddressOfSymbol("SetStrings")),
            nullptr);
>>>>>>> 38ac4093d9d2ae28d631ca1cc5802533989165c5

  // Test unload/destruction ordering
  EXPECT_EQ(Order.size(), 2UL);
#if INTEL_CUSTOMIZATION
  EXPECT_EQ(Order.front(), State::SECOND_CONSTANT);
  EXPECT_EQ(Order.back(), State::FIRST_CONSTANT);
#endif // INTEL_CUSTOMIZATION
}

#else

TEST(DynamicLibrary, Unsupported) {
  std::string Err;
  DynamicLibrary DL =
      DynamicLibrary::getPermanentLibrary(LibPath().c_str(), &Err);
  EXPECT_FALSE(DL.isValid());
  EXPECT_EQ(Err, "dlopen() not supported on this platform");
}

#endif
