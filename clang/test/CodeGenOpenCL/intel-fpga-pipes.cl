// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -opaque-pointers -O0 -cl-std=CL2.0 -o - %s | FileCheck %s --check-prefix SPIR_20
// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -opaque-pointers -O0 -cl-std=CL1.2 -o - %s | FileCheck %s --check-prefix SPIR_12
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -emit-llvm -opaque-pointers -O0 -cl-std=CL2.0 -o - %s | FileCheck %s --check-prefix X86
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -emit-llvm -opaque-pointers -O0 -cl-std=CL1.2 -o - %s | FileCheck %s --check-prefix X86

void test1(read_only pipe int p, global int *ptr) {
  // SPIR_20: call spir_func i32 @__read_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__read_pipe_2_AS1(ptr addrspace(1) %{{.*}}, ptr addrspace(1) %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__read_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr);
  local int *ptr_l;
  // SPIR_20: call spir_func i32 @__read_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__read_pipe_2_AS3(ptr addrspace(1) %{{.*}}, ptr addrspace(3) %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__read_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr_l);
  int *ptr_p;
  // SPIR_20: call spir_func i32 @__read_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__read_pipe_2_AS0(ptr addrspace(1) %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__read_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr_p);
}
// SPIR_20: declare spir_func i32 @__read_pipe_2(ptr addrspace(1), ptr addrspace(4), i32, i32)
// SPIR_12: declare spir_func i32 @__read_pipe_2_AS1(ptr addrspace(1), ptr addrspace(1), i32, i32)
// SPIR_12: declare spir_func i32 @__read_pipe_2_AS3(ptr addrspace(1), ptr addrspace(3), i32, i32)
// SPIR_12: declare spir_func i32 @__read_pipe_2_AS0(ptr addrspace(1), ptr, i32, i32)
// X86: declare i32 @__read_pipe_2(ptr, ptr, i32, i32)

void test2(write_only pipe int p, global int *ptr) {
  // SPIR_20: call spir_func i32 @__write_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS1(ptr addrspace(1) %{{.*}}, ptr addrspace(1) %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr);
  local int *ptr_l;
  // SPIR_20: call spir_func i32 @__write_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS3(ptr addrspace(1) %{{.*}}, ptr addrspace(3) %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_l);
  int *ptr_p;
  // SPIR_20: call spir_func i32 @__write_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS0(ptr addrspace(1) %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_p);
  constant int *ptr_c;
  // SPIR_20: call spir_func i32 @__write_pipe_2(ptr addrspace(1) %{{.*}}, ptr addrspace(4) %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS2(ptr addrspace(1) %{{.*}}, ptr addrspace(2) %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(ptr %{{.*}}, ptr %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_c);
}
// SPIR_20: declare spir_func i32 @__write_pipe_2(ptr addrspace(1), ptr addrspace(4), i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS1(ptr addrspace(1), ptr addrspace(1), i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS3(ptr addrspace(1), ptr addrspace(3), i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS0(ptr addrspace(1), ptr, i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS2(ptr addrspace(1), ptr addrspace(2), i32, i32)
// X86: declare i32 @__write_pipe_2(ptr, ptr, i32, i32)
