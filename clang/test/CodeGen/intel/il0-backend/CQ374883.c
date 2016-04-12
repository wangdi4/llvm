// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -O0 -fintel-compatibility %s -emit-llvm -o - -verify | FileCheck %s
// expected-no-diagnostics

// CHECK-LABEL: @strcat
char *__builtin_strcat(char *dest, const char *src) { return dest; }
// CHECK-LABEL: @expect
long __builtin_expect(long exp, long c) { return exp + c; }
// CHECK-LABEL: @inff
float __builtin_inff(void) { return 1234.5678; }

char *char_ptr;
long lng;
// CHECK-LABEL: @main
int main() {
  lng = __builtin_expect(lng, lng);
  // CHECK: call i8* @__builtin_strcat(i8* %{{.+}}, i8* %{{.+}})
  char_ptr = __builtin_strcat(char_ptr, char_ptr);
  return __builtin_inff() + lng;
}
