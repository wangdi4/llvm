// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -O0 -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

// Page1
#define SGX (1ULL << 53)
#define SHSTK (1ULL << 52)

// Page2
#define ENQCMD (1ULL << (69 - 64))
#define WAITPKG (1ULL << (67 - 64))

__attribute__((allow_cpu_features(1))) void Generic() {}

template <unsigned long long I, unsigned long long J>
__attribute__((allow_cpu_features(I, J))) void Func() {}

// CHECK: define{{.+}}Genericv() #[[NO_ADDED_FEATS:[0-9]+]]

// This check ensures that 'Generic' and 'bar' have the same list of features,
// so we know that the attribute with '1' above changed nothing.
// CHECK: define{{.+}}barv() #[[NO_ADDED_FEATS]]
void bar() {
  // CHECK: call void {{.+}}Genericv()
  Generic();
  Func<SGX, 0>();
  // CHECK: call void @[[SGX:.+]]()
  Func<0, WAITPKG>();
  // CHECK: call void @[[WAITPKG:.+]]()
  Func<SHSTK, ENQCMD>();
  // CHECK: call void @[[TWO:.+]]()
  Func<SGX | SHSTK, ENQCMD | WAITPKG>();
  // CHECK: call void @[[ALL:.+]]()
}

// CHECK: define{{.+}}[[SGX]]() #[[SGX_ATTR:[0-9]+]]
// CHECK: define{{.+}}[[WAITPKG]]() #[[WAITPKG_ATTR:[0-9]+]]
// CHECK: define{{.+}}[[TWO]]() #[[TWO_ATTR:[0-9]+]]
// CHECK: define{{.+}}[[ALL]]() #[[ALL_ATTR:[0-9]+]]

// CHECK: attributes #[[SGX_ATTR]]
// CHECK-SAME: +sgx
// CHECK: attributes #[[WAITPKG_ATTR]]
// CHECK-SAME: +waitpkg
// CHECK: attributes #[[TWO_ATTR]]
// CHECK-SAME: +enqcmd
// CHECK-SAME: +shstk
// CHECK: attributes #[[ALL_ATTR]]
// CHECK-SAME: +enqcmd
// CHECK-SAME: +sgx
// CHECK-SAME: +shstk
// CHECK-SAME: +waitpkg
