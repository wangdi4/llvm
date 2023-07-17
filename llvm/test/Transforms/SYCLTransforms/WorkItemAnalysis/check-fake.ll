; RUN: llvm-as %S/builtin-lib.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;check_fake(global int *in, global int *out, const int num, const int inc) {
;  int gid = get_global_id(0);
;
;  in += select(0, num, inc);
;  out += select(0, num, inc);
;
;  out[gid] = in[gid];
;}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: WorkItemAnalysis for function check_fake:
; CHECK-NEXT: RND   %1 = alloca ptr addrspace(1), align 8
; CHECK-NEXT: RND   %2 = alloca ptr addrspace(1), align 8
; CHECK-NEXT: PTR   %3 = alloca i32, align 4
; CHECK-NEXT: PTR   %4 = alloca i32, align 4
; CHECK-NEXT: PTR   %gid = alloca i32, align 4
; CHECK-NEXT: RND   store ptr addrspace(1) %in, ptr %1, align 8
; CHECK-NEXT: RND   store ptr addrspace(1) %out, ptr %2, align 8
; CHECK-NEXT: RND   store i32 %num, ptr %3, align 4
; CHECK-NEXT: RND   store i32 %inc, ptr %4, align 4
; CHECK-NEXT: SEQ   %5 = call spir_func i64 @_Z13get_global_idj(i32 0) #1
; CHECK-NEXT: SEQ   %6 = trunc i64 %5 to i32
; CHECK-NEXT: RND   store i32 %6, ptr %gid, align 4
; CHECK-NEXT: RND   %7 = load i32, ptr %3, align 4
; CHECK-NEXT: RND   %8 = load i32, ptr %4, align 4
; CHECK-NEXT: RND   %9 = call spir_func i32 @_Z6selectiii(i32 0, i32 %7, i32 %8) #1
; CHECK-NEXT: RND   %10 = load ptr addrspace(1), ptr %1, align 8
; CHECK-NEXT: RND   %11 = sext i32 %9 to i64
; CHECK-NEXT: RND   %12 = getelementptr inbounds i32, ptr addrspace(1) %10, i64 %11
; CHECK-NEXT: RND   store ptr addrspace(1) %12, ptr %1, align 8
; CHECK-NEXT: RND   %13 = load i32, ptr %3, align 4
; CHECK-NEXT: RND   %14 = load i32, ptr %4, align 4
; CHECK-NEXT: RND   %15 = call spir_func i32 @_Z6selectiii(i32 0, i32 %13, i32 %14) #1
; CHECK-NEXT: RND   %16 = load ptr addrspace(1), ptr %2, align 8
; CHECK-NEXT: RND   %17 = sext i32 %15 to i64
; CHECK-NEXT: RND   %18 = getelementptr inbounds i32, ptr addrspace(1) %16, i64 %17
; CHECK-NEXT: RND   store ptr addrspace(1) %18, ptr %2, align 8
; CHECK-NEXT: RND   %19 = load i32, ptr %gid, align 4
; CHECK-NEXT: RND   %20 = sext i32 %19 to i64
; CHECK-NEXT: RND   %21 = load ptr addrspace(1), ptr %1, align 8
; CHECK-NEXT: RND   %22 = getelementptr inbounds i32, ptr addrspace(1) %21, i64 %20
; CHECK-NEXT: RND   %23 = load i32, ptr addrspace(1) %22, align 4
; CHECK-NEXT: RND   %24 = load i32, ptr %gid, align 4
; CHECK-NEXT: RND   %25 = sext i32 %24 to i64
; CHECK-NEXT: RND   %26 = load ptr addrspace(1), ptr %2, align 8
; CHECK-NEXT: RND   %27 = getelementptr inbounds i32, ptr addrspace(1) %26, i64 %25
; CHECK-NEXT: RND   store i32 %23, ptr addrspace(1) %27, align 4
; CHECK-NEXT: UNI   ret void

define spir_kernel void @check_fake(ptr addrspace(1) %in, ptr addrspace(1) %out, i32 %num, i32 %inc) nounwind !kernel_arg_base_type !9 !arg_type_null_val !10 {
  %1 = alloca ptr addrspace(1), align 8
  %2 = alloca ptr addrspace(1), align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %gid = alloca i32, align 4
  store ptr addrspace(1) %in, ptr %1, align 8
  store ptr addrspace(1) %out, ptr %2, align 8
  store i32 %num, ptr %3, align 4
  store i32 %inc, ptr %4, align 4
  %5 = call spir_func i64 @_Z13get_global_idj(i32 0) nounwind readnone
  %6 = trunc i64 %5 to i32
  store i32 %6, ptr %gid, align 4
  %7 = load i32, ptr %3, align 4
  %8 = load i32, ptr %4, align 4
  %9 = call spir_func i32 @_Z6selectiii(i32 0, i32 %7, i32 %8) nounwind readnone
  %10 = load ptr addrspace(1), ptr %1, align 8
  %11 = sext i32 %9 to i64
  %12 = getelementptr inbounds i32, ptr addrspace(1) %10, i64 %11
  store ptr addrspace(1) %12, ptr %1, align 8
  %13 = load i32, ptr %3, align 4
  %14 = load i32, ptr %4, align 4
  %15 = call spir_func i32 @_Z6selectiii(i32 0, i32 %13, i32 %14) nounwind readnone
  %16 = load ptr addrspace(1), ptr %2, align 8
  %17 = sext i32 %15 to i64
  %18 = getelementptr inbounds i32, ptr addrspace(1) %16, i64 %17
  store ptr addrspace(1) %18, ptr %2, align 8
  %19 = load i32, ptr %gid, align 4
  %20 = sext i32 %19 to i64
  %21 = load ptr addrspace(1), ptr %1, align 8
  %22 = getelementptr inbounds i32, ptr addrspace(1) %21, i64 %20
  %23 = load i32, ptr addrspace(1) %22, align 4
  %24 = load i32, ptr %gid, align 4
  %25 = sext i32 %24 to i64
  %26 = load ptr addrspace(1), ptr %2, align 8
  %27 = getelementptr inbounds i32, ptr addrspace(1) %26, i64 %25
  store i32 %23, ptr addrspace(1) %27, align 4
  ret void
}

declare spir_func i64 @_Z13get_global_idj(i32) nounwind readnone

declare spir_func i32 @_Z6selectiii(i32, i32, i32) nounwind readnone

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{ptr @check_fake, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*", !"int", !"int"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"const", !"const"}
!5 = !{!"kernel_arg_name", !"in", !"out", !"num", !"inc"}
!6 = !{i32 1, i32 0}
!7 = !{i32 0, i32 0}
!8 = !{}
!9 = !{!"int*", !"int*", !"int", !"int"}
!10 = !{ptr addrspace(1) null, ptr addrspace(1) null, i32 0, i32 0}
