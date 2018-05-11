// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -O0 -cl-std=CL2.0 -o - %s | FileCheck %s --check-prefix CL20
// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm -O0 -cl-std=CL1.2 -o - %s | FileCheck %s --check-prefix CL12

// CL20: %opencl.pipe_t = type opaque
// CL12: %opencl.pipe_t = type opaque

void test1(read_only pipe int p, global int *ptr) {
  // CL20: call i32 @__read_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__read_pipe_2_AS1(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(1)* %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr);
  local int *ptr_l;
  // CL20: call i32 @__read_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__read_pipe_2_AS3(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(3)* %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr_l);
  int *ptr_p;
  // CL20: call i32 @__read_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__read_pipe_2_AS0(%opencl.pipe_t addrspace(1)* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  read_pipe(p, ptr_p);
}
// CL20: declare i32 @__read_pipe_2(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32)
// CL12: declare i32 @__read_pipe_2_AS1(%opencl.pipe_t addrspace(1)*, i8 addrspace(1)*, i32, i32)
// CL12: declare i32 @__read_pipe_2_AS3(%opencl.pipe_t addrspace(1)*, i8 addrspace(3)*, i32, i32)
// CL12: declare i32 @__read_pipe_2_AS0(%opencl.pipe_t addrspace(1)*, i8*, i32, i32)

void test2(write_only pipe int p, global int *ptr) {
  // CL20: call i32 @__write_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__write_pipe_2_AS1(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(1)* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr);
  local int *ptr_l;
  // CL20: call i32 @__write_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__write_pipe_2_AS3(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(3)* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_l);
  int *ptr_p;
  // CL20: call i32 @__write_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__write_pipe_2_AS0(%opencl.pipe_t addrspace(1)* %{{.*}}, i8* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_p);
  constant int *ptr_c;
  // CL20: call i32 @__write_pipe_2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(4)* %{{.*}}, i32 4, i32 4)
  // CL12: call i32 @__write_pipe_2_AS2(%opencl.pipe_t addrspace(1)* %{{.*}}, i8 addrspace(2)* %{{.*}}, i32 4, i32 4)
  write_pipe(p, ptr_c);
}
// CL20: declare i32 @__write_pipe_2(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32)
// CL12: declare i32 @__write_pipe_2_AS1(%opencl.pipe_t addrspace(1)*, i8 addrspace(1)*, i32, i32)
// CL12: declare i32 @__write_pipe_2_AS3(%opencl.pipe_t addrspace(1)*, i8 addrspace(3)*, i32, i32)
// CL12: declare i32 @__write_pipe_2_AS0(%opencl.pipe_t addrspace(1)*, i8*, i32, i32)
// CL12: declare i32 @__write_pipe_2_AS2(%opencl.pipe_t addrspace(1)*, i8 addrspace(2)*, i32, i32)

