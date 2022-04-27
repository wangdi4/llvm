// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -no-opaque-pointers -O0 -cl-std=CL2.0 -o - %s | FileCheck %s --check-prefix SPIR_20
// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -no-opaque-pointers -O0 -cl-std=CL1.2 -o - %s | FileCheck %s --check-prefix SPIR_12
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -emit-llvm -no-opaque-pointers -O0 -cl-std=CL2.0 -o - %s | FileCheck %s --check-prefix X86
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -emit-llvm -no-opaque-pointers -O0 -cl-std=CL1.2 -o - %s | FileCheck %s --check-prefix X86

// SPIR_20: %opencl.pipe_ro_t = type opaque
// SPIR_20: %opencl.pipe_wo_t = type opaque
// SPIR_12: %opencl.pipe_ro_t = type opaque
// SPIR_12: %opencl.pipe_wo_t = type opaque
// X86: %opencl.pipe_ro_t = type opaque
// X86: %opencl.pipe_wo_t = type opaque

void test1(read_only pipe int p, global int *ptr) {
  // SPIR_20: call spir_func i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__read_pipe_2_AS1(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(1)* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__read_pipe_2(%opencl.pipe_ro_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr);
  local int *ptr_l;
  // SPIR_20: call spir_func i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__read_pipe_2_AS3(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(3)* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__read_pipe_2(%opencl.pipe_ro_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr_l);
  int *ptr_p;
  // SPIR_20: call spir_func i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__read_pipe_2_AS0(%opencl.pipe_ro_t addrspace(1)* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__read_pipe_2(%opencl.pipe_ro_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr_p);
}
// SPIR_20: declare spir_func i32 @__read_pipe_2(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)
// SPIR_12: declare spir_func i32 @__read_pipe_2_AS1(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(1)*, i32, i32)
// SPIR_12: declare spir_func i32 @__read_pipe_2_AS3(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(3)*, i32, i32)
// SPIR_12: declare spir_func i32 @__read_pipe_2_AS0(%opencl.pipe_ro_t addrspace(1)*, i8*, i32, i32)
// X86: declare i32 @__read_pipe_2(%opencl.pipe_ro_t*, i8*, i32, i32)

void test2(write_only pipe int p, global int *ptr) {
  // SPIR_20: call spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS1(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(1)* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(%opencl.pipe_wo_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr);
  local int *ptr_l;
  // SPIR_20: call spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS3(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(3)* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(%opencl.pipe_wo_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_l);
  int *ptr_p;
  // SPIR_20: call spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS0(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(%opencl.pipe_wo_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_p);
  constant int *ptr_c;
  // SPIR_20: call spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // SPIR_12: call spir_func i32 @__write_pipe_2_AS2(%opencl.pipe_wo_t addrspace(1)* %{{.*}}, i8 addrspace(2)* %{{.*}}, i32 4, i32 4)
  // X86: call i32 @__write_pipe_2(%opencl.pipe_wo_t* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_c);
}
// SPIR_20: declare spir_func i32 @__write_pipe_2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(4)*, i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS1(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(1)*, i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS3(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(3)*, i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS0(%opencl.pipe_wo_t addrspace(1)*, i8*, i32, i32)
// SPIR_12: declare spir_func i32 @__write_pipe_2_AS2(%opencl.pipe_wo_t addrspace(1)*, i8 addrspace(2)*, i32, i32)
// X86: declare i32 @__write_pipe_2(%opencl.pipe_wo_t*, i8*, i32, i32)
