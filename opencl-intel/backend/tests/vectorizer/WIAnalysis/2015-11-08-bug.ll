; Regression test for assertion in demangler when using a scalar select in divergent section
; Mangler::demangle_fake_builtin()
; intel::WIAnalysis::calculate_dep()

; IR was generated with:
; clang -cc1 -triple x86_64-pc-win32-elf -emit-llvm -fwrapv -O3 -x cl -I src/cl_api -I src/backend/clang_headers -I cclang/cl_headers/ -I src/backend/libraries/ocl_builtins -include opencl_.h -fblocks -D__OPENCL_C_VERSION__=200 -target-cpu core-avx2 < src\backend\tests\vectorizer\WIAnalysis\2015-11-08-bug.cl

; RUN: opt -CLBltnPreVec -predicate -WIAnalysis -S < %s

; ModuleID = '2015-11-08-bug.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-elf"

; Function Attrs: nounwind
define void @generateHitDataPrim(i32 addrspace(1)* noalias nocapture %early_exit, float addrspace(1)* nocapture %in_select_arg1, float addrspace(1)* nocapture %in_select_arg2, float addrspace(1)* nocapture %out_dets) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %call1 = tail call i64 @_Z13get_global_idj(i32 1) #2
  %call2 = tail call i64 @_Z15get_global_sizej(i32 0) #2
  %mul = mul i64 %call2, %call1
  %add = add i64 %mul, %call
  %arrayidx = getelementptr i32 addrspace(1)* %early_exit, i64 %add
  %0 = load i32 addrspace(1)* %arrayidx, align 4
  %cmp = icmp eq i32 %0, 2147483647
  br i1 %cmp, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %arrayidx4 = getelementptr float addrspace(1)* %in_select_arg1, i64 %add
  %1 = load float addrspace(1)* %arrayidx4, align 4
  %arrayidx5 = getelementptr float addrspace(1)* %in_select_arg2, i64 %add
  %2 = load float addrspace(1)* %arrayidx5, align 4
  %call7 = tail call float @_Z6selectffi(float %1, float %2, i32 1) #2
  %mul8 = fmul float %call7, %call7
  %arrayidx9 = getelementptr float addrspace(1)* %out_dets, i64 %add
  store float %mul8, float addrspace(1)* %arrayidx9, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %arrayidx10 = getelementptr float addrspace(1)* %out_dets, i64 %add
  store float 0.000000e+00, float addrspace(1)* %arrayidx10, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

; Function Attrs: nounwind readnone
declare i64 @_Z15get_global_sizej(i32) #1

; Function Attrs: nounwind readnone
declare float @_Z6selectffi(float, float, i32) #1