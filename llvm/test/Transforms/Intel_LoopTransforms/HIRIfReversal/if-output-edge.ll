; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-if-reversal" -print-before=hir-if-reversal -print-after=hir-if-reversal -aa-pipeline="basic-aa" -disable-output 2>&1 < %s  | FileCheck %s

; Check that the if is reversed for OUTPUT edge for sub_$A
; DDEdge Candidate: 17:11 (%"sub_$A")[i1 + 2] --> (%"sub_$A")[i1 + 1] OUTPUT (<) (1)

;*** IR Dump Before HIR If Reversal (hir-if-reversal) ***
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, sext.i32.i64((2 * %"sub_$N_fetch.1")) + -3, 1   <DO_LOOP>
; CHECK:        |   if ((%"sub_$B")[i1 + 2] == 1.000000e+02)
;               |   {
;               |      (%"sub_$A")[i1 + 1] = 1.000000e+02;
;               |   }
;               |   else
;               |   {
;               |      (%"sub_$A")[i1 + 2] = (%"sub_$B")[i1 + 1];
;               |   }
;               + END LOOP
;         END REGION

;*** IR Dump After HIR If Reversal (hir-if-reversal) ***
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, sext.i32.i64((2 * %"sub_$N_fetch.1")) + -3, 1   <DO_LOOP>
; CHECK:        |   if ((%"sub_$B")[i1 + 2] !=u 1.000000e+02)
;               |   {
; CHECK:        |      (%"sub_$A")[i1 + 2] = (%"sub_$B")[i1 + 1];
;               |   }
; CHECK:        |   else
;               |   {
; CHECK:        |      (%"sub_$A")[i1 + 1] = 1.000000e+02;
;               |   }
; CHECK:        + END LOOP
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(ptr noalias nocapture writeonly dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N", i32 %"sub_$N_fetch.1") #0 {
alloca_0:
  %"sub_$N_fetch.11" = load i32, ptr null, align 1
  %mul.1 = shl i32 %"sub_$N_fetch.1", 1
  br label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %0 = or i32 %mul.1, 1
  %wide.trip.count = sext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb6_endif, %bb2.preheader
  %indvars.iv = phi i64 [ 3, %bb2.preheader ], [ %indvars.iv.next, %bb6_endif ]
  %"sub_$B_entry[]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr nonnull elementtype(float) %"sub_$B", i64 %indvars.iv)
  %"sub_$B_entry[]_fetch.4" = load float, ptr %"sub_$B_entry[]10", align 1
  %rel.2 = fcmp reassoc ninf nsz arcp contract afn oeq float %"sub_$B_entry[]_fetch.4", 1.000000e+02
  %1 = add nsw i64 %indvars.iv, -1
  br i1 %rel.2, label %bb_new3_then, label %bb_new4_else

bb_new3_then:                                     ; preds = %bb2
  %"sub_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A", i64 %1)
  store float 1.000000e+02, ptr %"sub_$A_entry[]", align 1
  br label %bb6_endif

bb_new4_else:                                     ; preds = %bb2
  %"sub_$B_entry[]4" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr nonnull elementtype(float) %"sub_$B", i64 %1)
  %"sub_$B_entry[]_fetch.9" = load float, ptr %"sub_$B_entry[]4", align 1
  %"sub_$A_entry[]7" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv)
  store float %"sub_$B_entry[]_fetch.9", ptr %"sub_$A_entry[]7", align 1
  br label %bb6_endif

bb6_endif:                                        ; preds = %bb_new4_else, %bb_new3_then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb6_endif
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }
