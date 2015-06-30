
; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -CLBltnPreVec -print-wia-check -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll  | FileCheck %s


; ModuleID = 'check_fake.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define spir_kernel void @check_fake(i32 addrspace(1)* %in, i32 addrspace(1)* %out, i32 %num, i32 %inc) nounwind {
  %1 = alloca i32 addrspace(1)*, align 8
  %2 = alloca i32 addrspace(1)*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %gid = alloca i32, align 4
  store i32 addrspace(1)* %in, i32 addrspace(1)** %1, align 8
  store i32 addrspace(1)* %out, i32 addrspace(1)** %2, align 8
  store i32 %num, i32* %3, align 4
  store i32 %inc, i32* %4, align 4
  %5 = call spir_func i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %6 = trunc i64 %5 to i32
  store i32 %6, i32* %gid, align 4
  %7 = load i32* %3, align 4
  %8 = load i32* %4, align 4
  %9 = call spir_func i32 @_Z6selectiii(i32 0, i32 %7, i32 %8) nounwind readnone
  %10 = load i32 addrspace(1)** %1, align 8
  %11 = sext i32 %9 to i64
  %12 = getelementptr inbounds i32 addrspace(1)* %10, i64 %11
  store i32 addrspace(1)* %12, i32 addrspace(1)** %1, align 8
  %13 = load i32* %3, align 4
  %14 = load i32* %4, align 4
  %15 = call spir_func i32 @_Z6selectiii(i32 0, i32 %13, i32 %14) nounwind readnone
  %16 = load i32 addrspace(1)** %2, align 8
  %17 = sext i32 %15 to i64
  %18 = getelementptr inbounds i32 addrspace(1)* %16, i64 %17
  store i32 addrspace(1)* %18, i32 addrspace(1)** %2, align 8
  %19 = load i32* %gid, align 4
  %20 = sext i32 %19 to i64
  %21 = load i32 addrspace(1)** %1, align 8
  %22 = getelementptr inbounds i32 addrspace(1)* %21, i64 %20
  %23 = load i32 addrspace(1)* %22, align 4
  %24 = load i32* %gid, align 4
  %25 = sext i32 %24 to i64
  %26 = load i32 addrspace(1)** %2, align 8
  %27 = getelementptr inbounds i32 addrspace(1)* %26, i64 %25
  store i32 %23, i32 addrspace(1)* %27, align 4
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32) nounwind readnone

declare spir_func i32 @_Z6selectiii(i32, i32, i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32, i32)* @check_fake, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*", !"int", !"int"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"const", !"const"}
!5 = !{!"kernel_arg_name", !"in", !"out", !"num", !"inc"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}

;CHECK: WIA 0   %{{[0-9]*}} = tail call spir_func i32 @_f_v._Z6selectiii


;__kernel void check_fake (__global  int *in, __global int *out, const int num, const int inc)
;{
;  int gid = get_global_id(0);
;        
;  in += select(0, num, inc);
;  out += select(0, num, inc);
;
;  out[gid] = in[gid];
;}

