;
; Test to check that vector function vectorization is detected and 
; no masked vector loop is created.
;
; RUN: opt -disable-output --passes=hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec --vplan-print-after-lcssa %s 2>&1 | FileCheck %s --check-prefixes=HIR
; RUN: opt -disable-output --passes=vplan-vec --vplan-print-after-lcssa %s 2>&1 | FileCheck %s --check-prefixes=LLVM

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; HIR-NOT: VPlan IR for: _ZGVdN8v__Z3fooi:HIR.#1.cloned.masked
; LLVM-NOT: VPlan IR for: _ZGVdN8v__Z3fooi:simd.loop.header.#1.cloned.masked

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) uwtable
define dso_local noundef <8 x i32> @_ZGVdN8v__Z3fooi(<8 x i32> noundef %v) local_unnamed_addr #1 {
entry:
  %vec.v = alloca <8 x i32>, align 32
  %vec.retval = alloca <8 x i32>, align 32
  store <8 x i32> %v, ptr %vec.v, align 32
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.header, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.header ]
  %vec.v.gep = getelementptr i32, ptr %vec.v, i32 %index
  %vec.v.elem = load i32, ptr %vec.v.gep, align 4
  %add = shl nsw i32 %vec.v.elem, 1
  %vec.retval.gep = getelementptr i32, ptr %vec.retval, i32 %index
  store i32 %add, ptr %vec.retval.gep, align 4
  %indvar = add nuw nsw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region, !llvm.loop !4

simd.end.region:                                  ; preds = %simd.loop.header
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  %vec.ret = load <8 x i32>, ptr %vec.retval, align 32
  ret <8 x i32> %vec.ret
}

attributes #0 = { nounwind }
attributes #1 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "target-cpu"="skylake-avx512" "unsafe-fp-math"="true" }

!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.unroll.disable"}
