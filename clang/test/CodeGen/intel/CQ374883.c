// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -o - -verify | FileCheck %s
// expected-no-diagnostics

char *__builtin_strcat(char *dest, const char *src) { return dest; }
long __builtin_expect (long exp, long c) { return exp; }
float __builtin_inff (void) { return 0; }

char *char_ptr;
long lng;
// CHECK-LABEL: main
int main() {
  // CHECK: call i8* @__builtin_strcat(i8* %{{.+}}, i8* %{{.+}})
  char_ptr = __builtin_strcat(char_ptr, char_ptr);
  // CHECK: call i64 @__builtin_expect(i64 %{{.+}}, i64 %{{.+}})
  lng = __builtin_expect(lng, lng);
  // CHECK: call float @__builtin_inff()
  return __builtin_inff();
}
