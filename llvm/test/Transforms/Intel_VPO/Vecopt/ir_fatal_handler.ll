; RUN: opt %s -disable-output -passes=vplan-vec -vplan-force-vf=4 -vplan-debug-error-handler 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test to check that VecErrorHandler is called from CG when a fatal error occurs,
; In this test the error is in that there is no suitable vector variant for
; the indirect call.
;
; CHECK: Fatal error signaled on _ZGVbN4_direct

declare i32 @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr, ...) #2

define void @_ZGVbN4_direct(ptr %call.i) #1 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = call i32 (ptr, ...) @__intel_indirect_call_i32_p0p0f_i32i32f32f(ptr %call.i, i32 5, float 2.000000e+00) #2
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %simd.loop
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3


attributes #1 = { "vector-variants"="_ZGVbM4_direct,_ZGVbN4_direct" }
attributes #2 = { "vector-variants"="_ZGVbM4lv___intel_indirect_call_i32_p0p0f_i32i32f32f,_ZGVbN4lv___intel_indirect_call_i32_p0p0f_i32i32f32f" }
attributes #3 = { nounwind }
