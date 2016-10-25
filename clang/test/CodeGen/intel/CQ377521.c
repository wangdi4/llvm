// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -DUNI_TYPE=int8_t -fintel-compatibility %s -emit-obj -o /dev/null > %t 2>&1
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -DUNI_TYPE=int16_t -fintel-compatibility %s -emit-obj -o /dev/null > %t 2>&1
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -DUNI_TYPE=int32_t -fintel-compatibility %s -emit-obj -o /dev/null > %t 2>&1
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -DUNI_TYPE=int64_t -fintel-compatibility %s -emit-obj -o /dev/null > %t 2>&1

// expected-no-diagnostics

#include <stdint.h>

int8_t byte_1, byte_2;
int16_t int16_1, int16_2;
int32_t int32_1, int32_2;
int64_t int64_1, int64_2;
typedef struct S { } S;
S s_1;
float f;
double d;
void* v;

// Universal type, use run commands to define for all int types
UNI_TYPE uni_a, uni_b, uni_c;


// CHECK-LABEL: @main
int main() {
  // Unspecified sizes result in matching the parameter
  __atomic_store_explicit(&uni_a, uni_b, __ATOMIC_RELAXED);
  uni_b = __atomic_load_explicit(&uni_a, __ATOMIC_RELAXED);
  uni_c = __atomic_exchange_explicit(&uni_a, uni_b, __ATOMIC_RELAXED);
  __atomic_compare_exchange_weak_explicit(&uni_a, &uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
  __atomic_compare_exchange_strong_explicit(&uni_a, &uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
  uni_a = __atomic_fetch_add_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_fetch_sub_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_fetch_and_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_fetch_nand_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_fetch_or_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_fetch_xor_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_add_fetch_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_sub_fetch_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_and_fetch_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_nand_fetch_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);
  uni_a = __atomic_or_fetch_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);  
  uni_a = __atomic_xor_fetch_explicit(&uni_b, uni_c, __ATOMIC_RELAXED);

  // Specified sizes ALWAYS override the variable type
  __atomic_store_explicit_1(&uni_a,uni_b, __ATOMIC_RELAXED);
  uni_b= __atomic_load_explicit_1(&uni_a,__ATOMIC_RELAXED);
    uni_c= __atomic_exchange_explicit_1(&uni_a,uni_b, __ATOMIC_RELAXED);
    __atomic_compare_exchange_weak_explicit_1(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    __atomic_compare_exchange_strong_explicit_1(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_add_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_sub_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_and_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_nand_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_or_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_xor_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_add_fetch_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_sub_fetch_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_and_fetch_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_nand_fetch_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_or_fetch_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_xor_fetch_explicit_1(&uni_b, uni_c, __ATOMIC_RELAXED);

    __atomic_store_explicit_2(&uni_a,uni_b, __ATOMIC_RELAXED);
    uni_b= __atomic_load_explicit_2(&uni_a,__ATOMIC_RELAXED);
    uni_c= __atomic_exchange_explicit_2(&uni_a,uni_b, __ATOMIC_RELAXED);
    __atomic_compare_exchange_weak_explicit_2(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    __atomic_compare_exchange_strong_explicit_2(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_add_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_sub_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_and_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_nand_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_or_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_xor_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_add_fetch_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_sub_fetch_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_and_fetch_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_nand_fetch_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_or_fetch_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_xor_fetch_explicit_2(&uni_b, uni_c, __ATOMIC_RELAXED);

    __atomic_store_explicit_4(&uni_a,uni_b, __ATOMIC_RELAXED);
    uni_b= __atomic_load_explicit_4(&uni_a,__ATOMIC_RELAXED);
    uni_c= __atomic_exchange_explicit_4(&uni_a,uni_b, __ATOMIC_RELAXED);
    __atomic_compare_exchange_weak_explicit_4(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    __atomic_compare_exchange_strong_explicit_4(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_add_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_sub_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_and_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_nand_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_or_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_xor_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_add_fetch_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_sub_fetch_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_and_fetch_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_nand_fetch_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_or_fetch_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_xor_fetch_explicit_4(&uni_b, uni_c, __ATOMIC_RELAXED);

    __atomic_store_explicit_8(&uni_a,uni_b, __ATOMIC_RELAXED);
    uni_b= __atomic_load_explicit_8(&uni_a,__ATOMIC_RELAXED);
    uni_c= __atomic_exchange_explicit_8(&uni_a,uni_b, __ATOMIC_RELAXED);
    __atomic_compare_exchange_weak_explicit_8(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    __atomic_compare_exchange_strong_explicit_8(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_add_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_sub_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_and_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_nand_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_or_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_xor_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_add_fetch_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_sub_fetch_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_and_fetch_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_nand_fetch_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_or_fetch_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_xor_fetch_explicit_8(&uni_b, uni_c, __ATOMIC_RELAXED);

    __atomic_store_explicit_16(&uni_a,uni_b, __ATOMIC_RELAXED);
    uni_b= __atomic_load_explicit_16(&uni_a,__ATOMIC_RELAXED);
    uni_c= __atomic_exchange_explicit_16(&uni_a,uni_b, __ATOMIC_RELAXED);
    __atomic_compare_exchange_weak_explicit_16(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    __atomic_compare_exchange_strong_explicit_16(&uni_a,&uni_b, uni_c, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_add_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_sub_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_and_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_nand_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_or_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_fetch_xor_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_add_fetch_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_sub_fetch_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_and_fetch_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_nand_fetch_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_or_fetch_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);
    uni_a= __atomic_xor_fetch_explicit_16(&uni_b, uni_c, __ATOMIC_RELAXED);

  // When the SIZE is specified, ANY pointer works, not just integers
  // It is an error to specify a float, void, or struct ptr to a atomic operation
    __atomic_store_explicit_1(&f, 5, __ATOMIC_RELAXED);
    __atomic_store_explicit_2(&d, 5, __ATOMIC_RELAXED);
    __atomic_store_explicit_4(v, 5, __ATOMIC_RELAXED);
    __atomic_store_explicit_8(&s_1, 5, __ATOMIC_RELAXED);
}
