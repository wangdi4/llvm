// INTEL UNSUPPORTED: intel_opencl && i686-pc-windows
// RUN: %clang_cc1 -fintel-compatibility -fasm-blocks -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s

__declspec(naked) int foo(void) {
  // CHECK: "pop edx{{.+}}pop eax{{.+}}jmp edx", "[[CLOBBERS:[a-zA-Z0-9@%{},~_ ]*\"]]()
  __asm {
    pop edx
    pop eax
    jmp edx
  }
}

__declspec(naked) int bar(void) {
  // CHECK: "push $$42{{.+}}call qword ptr ${0:P}{{.+}}ret", "*m[[CLOBBERS:[a-zA-Z0-9@%{},~_ ]*\"]](i32 ()* elementtype(i32 ()) @foo)
  __asm {
    push 42
    call foo
    ret
  }
}

