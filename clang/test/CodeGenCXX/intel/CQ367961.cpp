// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-msvc-win32 %s -std=c++03 -DNAMESPACE -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -std=c++03 -emit-llvm -o - | FileCheck %s

#ifdef NAMESPACE
namespace std {
}
#endif // NAMESPACE

// CHECK: ret i8* null
void *operator new(
    __SIZE_TYPE__ _Sz,
    std::align_val_t _A) throw() {
  return 0;
}

