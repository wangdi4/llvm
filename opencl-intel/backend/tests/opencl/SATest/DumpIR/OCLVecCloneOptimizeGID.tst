RUN: SATest -tsize=4 -BUILD -config=%s.ocl.cfg -dump-llvm-file - | FileCheck %s --check-prefixes=CHECK,CHECK-OCL
RUN: SATest -tsize=4 -BUILD -config=%s.sycl.cfg -dump-llvm-file - | FileCheck %s --check-prefixes=CHECK,CHECK-SYCL

Checks DPCPPKernelVecClone assumes global id fits in i32 for OpenCL program, but
not for SYCL program.

CHECK-LABEL: define void @test
CHECK-OCL-NOT: ashr exact <4 x i64> %{{.*}}, <i64 32, i64 32, i64 32, i64 32>
CHECK-SYCL: ashr exact <4 x i64> %{{.*}}, <i64 32, i64 32, i64 32, i64 32>
