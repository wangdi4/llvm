// RUN: %clang_cc1 -fintel-compatibility -faligned-allocation \
// RUN:  -triple x86_64-msvc-win32 -std=c++03 -DNAMESPACE \
// RUN:  -emit-llvm -o - %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility -faligned-allocation \
// RUN:  -triple x86_64-unknown-linux-gnu -std=c++03 \
// RUN:  -emit-llvm -o - %s | FileCheck %s

// RUN: %clang_cc1 -fintel-compatibility-enable=PredeclareAlignValT \
// RUN:  -faligned-allocation -triple x86_64-unknown-linux-gnu \
// RUN:  -std=c++03 -emit-llvm -o - %s | FileCheck %s

// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -faligned-allocation \
// RUN:  -triple x86_64-unknown-linux-gnu -std=c++03 \
// RUN:  -fintel-compatibility-disable=PredeclareAlignValT -verify %s -DERROR

#ifdef NAMESPACE
namespace std {
}
#endif // NAMESPACE

// CHECK: ret i8* null
void *operator new(
    __SIZE_TYPE__ _Sz,
    std::align_val_t _A) throw() {
#ifdef ERROR
// expected-error@-2 {{use of undeclared identifier 'std'}}
#endif
  return 0;
}
