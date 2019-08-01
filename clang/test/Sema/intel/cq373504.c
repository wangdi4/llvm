// RUN: %clang_cc1 %s -ast-print | FileCheck %s

// CHECK: int * __ptr32 p32;
int *__ptr32 p32;

// CHECK: int * __ptr64 p64;
int *__ptr64 p64;

// CHECK: void * __ptr64 PtrToPtr64(const void *p) {
// CHECK: return ((void * __ptr64)(unsigned long long)p);
// CHECK: }
void *__ptr64 PtrToPtr64(const void *p) {
  return ((void *__ptr64)(unsigned __int64)p);
}

// CHECK: void * __ptr32 PtrToPtr32(const void *p) {
// CHECK: return ((void * __ptr32)(unsigned long long)p);
// CHECK: }
void *__ptr32 PtrToPtr32(const void *p) {
  return ((void *__ptr32)(unsigned __int64)p);
}

