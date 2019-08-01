// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-unknown -emit-llvm %s -o - | FileCheck %s

int foo() { return 0; }

void check() {
  long y = 0;
  // CHECK: "", "m[[CLOBBERS:[a-zA-Z0-9@%{},~_ ]*\"]](i32 %{{.+}})
  __asm__("" : : "m"(foo()));

  // CHECK: "mov $1, $0", "=r,m[[CLOBBERS]](i64 55)
  __asm__("mov %1, %0"
          : "=r"(y)
          : "m"((long)55));
  // CHECK: "mov $1, $0", "=r,m[[CLOBBERS]](i64 %{{.+}})
  __asm__("mov %1, %0"
          : "=r"(y)
          : "m"(y++));
  // CHECK: "nop", "m[[CLOBBERS]](i32 %{{.+}})
  __asm__("nop"
          :
          : "m"(({
            unsigned __v;
            __v = 42;
          })));
}
