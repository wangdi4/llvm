// RUN: %clang_cc1 -emit-llvm -fcilkplus < %s | FileCheck %s
// XFAIL: *

// CHECK: define i32 @setArray0() #0 {
__declspec(vector)
extern int setArray0() {
  return 0;
}

// CHECK: define i32 @setArray1(i32* %a) #1 {
__declspec(vector(uniform(a)))
extern int setArray1(int *a) {
  a[0] = a[0] + 42;
  return a[0];
}

// CHECK: define i32 @setArray2(i32* %a, i32 %k) #2 {
__declspec(vector(uniform(a), linear(k:4)))
extern int setArray2(int *a, int k) {
  a[k] = a[k] + 1;
  return a[k];
}

// CHECK: define i8 @setArray3(i8* %a, i8 %x, i8 %k) #3 {
__declspec(vector(linear(x:2), linear(k:8), vectorlength(16)))
extern char setArray3(char *a, char x, unsigned char k) {
  a[k] = a[k] + x;
  return a[k];
}

// CHECK: define i32 @setArray4(i16* %a, double %x, i8 %k) #4 {
__declspec(vector(uniform(a,x,k), processor("core_2_duo_ssse3")))
extern int setArray4(short *a, double x, char k) {
  a[k] = a[k] + x;
  return a[k];
}

// CHECK: define float @setArray5(float* %a, float %x, i16 %k) #5 {
__declspec(vector(mask, processor("core_3rd_gen_avx")))
extern float setArray5(float *a, const float x, short int k) {
  a[k] = a[k] + x;
  return a[k];
}

// CHECK: define double @setArray6(double* %a, i32* %x, i64 %y, i64 %k) #6 {
__declspec(vector(uniform(a,y), linear(k:16), nomask, processor("core_4th_gen_avx")))
extern double setArray6(double *a, unsigned int *x, const long long y, long long k) {
  a[k] = a[k] + *x - y;
  return a[k];
}

struct S {
  char id;
  int data;
};

// CHECK: define i32 @setArray7(%struct.S* %a, i32 %x, i32 %k) #7 {
__declspec(vector(uniform(x)))
extern unsigned int setArray7(struct S *a, const int x, int k) {
  a[k].id = k;
  a[k].data = x;
  return a[k].data;
}

// CHECK: attributes #0 = { {{.*}} "_ZGVxM4_setArray0" "_ZGVxN4_setArray0"

// CHECK: attributes #1 = { {{.*}} "_ZGVxM4u_setArray1" "_ZGVxN4u_setArray1"

// CHECK: attributes #2 = { {{.*}} "_ZGVxM4ul4_setArray2" "_ZGVxN4ul4_setArray2"

// CHECK: attributes #3 = { {{.*}} "_ZGVxM12vl2l8_setArray3" "_ZGVxN12vl2l8_setArray3"

// CHECK: attributes #4 = { {{.*}} "_ZGVxM4uuu_setArray4" "_ZGVxN4uuu_setArray4"

// CHECK: attributes #5 = { {{.*}} "_ZGVyM8vvv_setArray5"
// CHECK-NOT: attributes #5 = { {{.*}} "_ZGVyN8vvv_setArray5"

// CHECK: attributes #6 = { {{.*}} "_ZGVYN4uvul16_setArray6"
// CHECK-NOT: attributes #6 = { {{.*}} "_ZGVYM4uvul16_setArray6"

// CHECK: attributes #7 = { {{.*}} "_ZGVxM4vuv_setArray7" "_ZGVxN4vuv_setArray7"

