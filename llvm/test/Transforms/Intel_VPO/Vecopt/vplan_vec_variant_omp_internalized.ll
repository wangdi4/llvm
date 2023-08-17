; RUN: opt %s -S -vplan-force-vf=1 --passes="openmp-opt,hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec" 2>&1 | FileCheck %s

; Since vectorizing loops with vector functions is now supported for HIR, this
; test started failing because the calls to the vector functions were
; scalarized due to VF=2 and no variant was available for that VF. Also, things
; were further complicated by the fact that the loop was completely unrolled
; after vectorization and only the scalarized calls remained. The test
; originally looks at IR dumped just after VPlanDriverHIR, and this is before
; the LLVM is cleaned up so the calls were appearing in the entry block with
; the loop still present in the dump. The HIR region was shown correctly (e.g.,
; no loop). It isn't until later when hir-cg creates valid LLVM-IR. Instead of
; adding more options and changing the dumps, it was easier to just keep the
; original intent of the test by forcing VF=1 and thus preventing the unrolling
; and scalarization of the vector function calls.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct.__tgt_offload_entry.0 = type { ptr, ptr, i64, i32, i32 }

@.omp_offloading.entry.__omp_offloading_10309_bcd6a20__ZN1b1dEiii_l727 = weak target_declare constant %struct.__tgt_offload_entry.0 { ptr @__omp_offloading_10309_bcd6a20__ZN1b1dEiii_l727, ptr poison, i64 0, i32 0, i32 0 }

; CHECK: define private void @_Z1fiPfS_iiii.internalized(i32 %g, ptr %_dst_, ptr %h, i32 %i, i32 %j, i32 %0, i32 %1) #[[ATTR0:[0-9]+]] {

define void @_Z1fiPfS_iiii(i32 %g, ptr %_dst_, ptr %h, i32 %i, i32 %j, i32 %0, i32 %1) #1 {
; CHECK:       define void @_Z1fiPfS_iiii(i32 %g, ptr %_dst_, ptr %h, i32 %i, i32 %j, i32 %0, i32 %1) #[[ATTR1:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[DOTOMP_UB:%.*]] = alloca i32, i32 0, align 4
; CHECK-NEXT:    br label [[DIR_OMP_SIMD_214:%.*]]
; CHECK:       DIR.OMP.SIMD.214:
; CHECK-NEXT:    store i32 [[I:%.*]], ptr [[DOTOMP_UB]], align 4
; CHECK-NEXT:    br label [[DIR_OMP_SIMD_1_SPLIT:%.*]]
; CHECK:       DIR.OMP.SIMD.1.split:
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr [[DOTOMP_UB]], align 4
; CHECK-NEXT:    br label [[DIR_OMP_SIMD_2:%.*]]
; CHECK:       DIR.OMP.SIMD.2:
; CHECK-NEXT:    [[CMP4_NOT17:%.*]] = icmp sgt i32 0, [[TMP2]]
; CHECK-NEXT:    br i1 [[CMP4_NOT17]], label [[DIR_OMP_END_SIMD_3_LOOPEXIT:%.*]], label [[OMP_INNER_FOR_BODY_LR_PH:%.*]]
; CHECK:       omp.inner.for.body.lr.ph:
; CHECK-NEXT:    [[TMP3:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
; CHECK-NEXT:    [[DOTOMP_IV_LOCAL_018_IN:%.*]] = call i32 @llvm.ssa.copy.i32(i32 0), !in.de.ssa !5
; CHECK-NEXT:    br label [[OMP_INNER_FOR_BODY:%.*]]
; CHECK:       omp.inner.for.body:
; CHECK-NEXT:    [[DOTOMP_IV_LOCAL_018:%.*]] = phi i32 [ 0, [[OMP_INNER_FOR_BODY_LR_PH]] ], [ 1, [[OMP_INNER_FOR_BODY]] ], !in.de.ssa !5
; CHECK-NEXT:    call void @_Z1fiPfS_iiii(i32 0, ptr null, ptr null, i32 0, i32 0, i32 0, i32 0)
; CHECK-NEXT:    [[CMP4_NOT:%.*]] = icmp sgt i32 1, [[DOTOMP_IV_LOCAL_018]]
; CHECK-NEXT:    [[DOTOMP_IV_LOCAL_018_IN1:%.*]] = call i32 @llvm.ssa.copy.i32(i32 1), !in.de.ssa !5
; CHECK-NEXT:    br i1 [[CMP4_NOT]], label [[OMP_INNER_FOR_BODY]], label [[OMP_INNER_FOR_COND_DIR_OMP_END_SIMD_3_LOOPEXIT_CRIT_EDGE:%.*]]
; CHECK:       omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge:
; CHECK-NEXT:    call void @llvm.directive.region.exit(token [[TMP3]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK-NEXT:    br label [[OMP_INNER_FOR_COND_DIR_OMP_END_SIMD_3_LOOPEXIT_CRIT_EDGE_SPLIT:%.*]]
; CHECK:       omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split:
; CHECK-NEXT:    br label [[DIR_OMP_END_SIMD_3_LOOPEXIT]]
; CHECK:       DIR.OMP.END.SIMD.3.loopexit:
; CHECK-NEXT:    ret void
;
entry:
  %.omp.ub = alloca i32, i32 0, align 4
  br label %DIR.OMP.SIMD.214

DIR.OMP.SIMD.214:                                 ; preds = %entry
  store i32 %i, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1.split

DIR.OMP.SIMD.1.split:                             ; preds = %DIR.OMP.SIMD.214
  %2 = load i32, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1.split
  %cmp4.not17 = icmp sgt i32 0, %2
  br i1 %cmp4.not17, label %DIR.OMP.END.SIMD.3.loopexit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %.omp.iv.local.018 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ 1, %omp.inner.for.body ]
  call void @_Z1fiPfS_iiii(i32 0, ptr null, ptr null, i32 0, i32 0, i32 0, i32 0)
  %cmp4.not = icmp sgt i32 1, %.omp.iv.local.018
  br i1 %cmp4.not, label %omp.inner.for.body, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3.loopexit

DIR.OMP.END.SIMD.3.loopexit:                      ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, %DIR.OMP.SIMD.2
  ret void
}

define internal void @__omp_offloading_10309_bcd6a20__ZN1b1dEiii_l727() {
; CHECK-LABEL: @__omp_offloading_10309_bcd6a20__ZN1b1dEiii_l727(
; CHECK-NEXT:  newFuncRoot:
; CHECK-NEXT:    br label [[FOR_COND4:%.*]]
; CHECK:       for.cond4:
; CHECK-NEXT:    call void @_Z1fiPfS_iiii.internalized(i32 0, ptr null, ptr null, i32 1, i32 0, i32 0, i32 0)
; CHECK-NEXT:    unreachable
;
newFuncRoot:
  br label %for.cond4

for.cond4:                                        ; preds = %for.cond4, %newFuncRoot
  call void @_Z1fiPfS_iiii(i32 0, ptr null, ptr null, i32 1, i32 0, i32 0, i32 0)
  br label %for.cond4
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "vector-variants"="_ZGVbN4vvvvvvv__Z1fiPfS_iiii,_ZGVcN8vvvvvvv__Z1fiPfS_iiii,_ZGVdN8vvvvvvv__Z1fiPfS_iiii,_ZGVeN16vvvvvvv__Z1fiPfS_iiii,_ZGVbM4vvvvvvv__Z1fiPfS_iiii,_ZGVcM8vvvvvvv__Z1fiPfS_iiii,_ZGVdM8vvvvvvv__Z1fiPfS_iiii,_ZGVeM16vvvvvvv__Z1fiPfS_iiii" }

; CHECK:      attributes #[[ATTR0]] = { noreturn nounwind "vector-variants"="_ZGVbN4vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVcN8vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVdN8vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVeN16vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVbM4vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVcM8vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVdM8vvvvvvv__Z1fiPfS_iiii.internalized,_ZGVeM16vvvvvvv__Z1fiPfS_iiii.internalized" }
; CHECK-NEXT: attributes #[[ATTR1]] = { "vector-variants"="_ZGVbN4vvvvvvv__Z1fiPfS_iiii,_ZGVcN8vvvvvvv__Z1fiPfS_iiii,_ZGVdN8vvvvvvv__Z1fiPfS_iiii,_ZGVeN16vvvvvvv__Z1fiPfS_iiii,_ZGVbM4vvvvvvv__Z1fiPfS_iiii,_ZGVcM8vvvvvvv__Z1fiPfS_iiii,_ZGVdM8vvvvvvv__Z1fiPfS_iiii,_ZGVeM16vvvvvvv__Z1fiPfS_iiii" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"openmp-device", i32 50}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"uwtable", i32 2}
