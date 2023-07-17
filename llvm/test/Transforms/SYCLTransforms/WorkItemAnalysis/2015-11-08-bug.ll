; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

; IR was generated with:
; clang -cc1 -triple x86_64-pc-win32-elf -emit-llvm -fwrapv -O3 -x cl -I src/cl_api -I src/backend/clang_headers -I cclang/cl_headers/ -I src/backend/libraries/ocl_builtins -include opencl_.h -fblocks -D__OPENCL_C_VERSION__=200 -target-cpu core-avx2 < 2015-11-08-bug.cl
;
;kernel void generateHitDataPrim(global const int *restrict early_exit,
;                                global const float *in_select_arg1,
;                                global const float *in_select_arg2,
;                                global float *out_dets) {
;  const size_t idX = get_global_id(0);
;  const size_t idY = get_global_id(1);
;  const size_t sizeX = get_global_size(0);
;  const size_t sizeY = get_global_size(1);
;
;  const size_t currIndex = idY * sizeX + idX;
;
;  const int early_exit_condition = early_exit[currIndex];
;
;  if (early_exit_condition != INT_MAX) {
;    int scalar_select_mask = 0;
;
;    float scalar_select_result = select(in_select_arg1[currIndex], in_select_arg2[currIndex], scalar_select_mask == 0);
;
;    out_dets[currIndex] = scalar_select_result * scalar_select_result;
;  } else {
;    out_dets[currIndex] = 0;
;  }
;}

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-elf"

; CHECK: WorkItemAnalysis for function generateHitDataPrim:
; CHECK-NEXT: SEQ   %call = tail call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: UNI   %call1 = tail call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: UNI   %call2 = tail call i64 @_Z15get_global_sizej(i32 0)
; CHECK-NEXT: UNI   %mul = mul i64 %call2, %call1
; CHECK-NEXT: SEQ   %add = add i64 %mul, %call
; CHECK-NEXT: PTR   %arrayidx = getelementptr i32, ptr addrspace(1) %early_exit, i64 %add
; CHECK-NEXT: RND   %0 = load i32, ptr addrspace(1) %arrayidx, align 4
; CHECK-NEXT: RND   %cmp = icmp eq i32 %0, 2147483647
; CHECK-NEXT: RND   br i1 %cmp, label %if.else, label %if.then
; CHECK-NEXT: PTR   %arrayidx4 = getelementptr float, ptr addrspace(1) %in_select_arg1, i64 %add
; CHECK-NEXT: RND   %1 = load float, ptr addrspace(1) %arrayidx4, align 4
; CHECK-NEXT: PTR   %arrayidx5 = getelementptr float, ptr addrspace(1) %in_select_arg2, i64 %add
; CHECK-NEXT: RND   %2 = load float, ptr addrspace(1) %arrayidx5, align 4
; CHECK-NEXT: RND   %call7 = tail call float @_Z6selectffi(float %1, float %2, i32 1)
; CHECK-NEXT: RND   %mul8 = fmul float %call7, %call7
; CHECK-NEXT: PTR   %arrayidx9 = getelementptr float, ptr addrspace(1) %out_dets, i64 %add
; CHECK-NEXT: RND   store float %mul8, ptr addrspace(1) %arrayidx9, align 4
; CHECK-NEXT: UNI   br label %if.end
; CHECK-NEXT: PTR   %arrayidx10 = getelementptr float, ptr addrspace(1) %out_dets, i64 %add
; CHECK-NEXT: RND   store float 0.000000e+00, ptr addrspace(1) %arrayidx10, align 4
; CHECK-NEXT: UNI   br label %if.end
; CHECK-NEXT: UNI   ret void

define void @generateHitDataPrim(ptr addrspace(1) noalias nocapture %early_exit, ptr addrspace(1) nocapture %in_select_arg1, ptr addrspace(1) nocapture %in_select_arg2, ptr addrspace(1) nocapture %out_dets) !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %call1 = tail call i64 @_Z13get_global_idj(i32 1)
  %call2 = tail call i64 @_Z15get_global_sizej(i32 0)
  %mul = mul i64 %call2, %call1
  %add = add i64 %mul, %call
  %arrayidx = getelementptr i32, ptr addrspace(1) %early_exit, i64 %add
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4
  %cmp = icmp eq i32 %0, 2147483647
  br i1 %cmp, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %arrayidx4 = getelementptr float, ptr addrspace(1) %in_select_arg1, i64 %add
  %1 = load float, ptr addrspace(1) %arrayidx4, align 4
  %arrayidx5 = getelementptr float, ptr addrspace(1) %in_select_arg2, i64 %add
  %2 = load float, ptr addrspace(1) %arrayidx5, align 4
  %call7 = tail call float @_Z6selectffi(float %1, float %2, i32 1)
  %mul8 = fmul float %call7, %call7
  %arrayidx9 = getelementptr float, ptr addrspace(1) %out_dets, i64 %add
  store float %mul8, ptr addrspace(1) %arrayidx9, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %arrayidx10 = getelementptr float, ptr addrspace(1) %out_dets, i64 %add
  store float 0.000000e+00, ptr addrspace(1) %arrayidx10, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

declare i64 @_Z13get_global_idj(i32)

declare i64 @_Z15get_global_sizej(i32)

declare float @_Z6selectffi(float, float, i32)

!0 = !{!"int*", !"float*", !"float*", !"float*"}
!1 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}
