; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-if-reversal  -print-before=hir-if-reversal -print-after=hir-if-reversal -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-if-reversal" -print-before=hir-if-reversal -print-after=hir-if-reversal -aa-pipeline="basic-aa" -disable-output 2>&1 < %s  | FileCheck %s

; Check that we reverse the lexically backward dependency for the flow edge on
; %"sub_$A". This enables vectorization for the loop.
; Reversible DDEdge: 20:14 (%"sub_$A")[i1 + 2] --> (%"sub_$A")[i1 + 1] FLOW (<) (1)

;*** IR Dump Before HIR Loop Reversal (hir-loop-reversal) ***
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, sext.i32.i64((2 * %"sub_$N_fetch.1")) + -3, 1   <DO_LOOP>
; CHECK:        |   if ((%"sub_$A")[i1 + 2] == 1.000000e+02)
;               |   {
;               |      %add.1 = (%"sub_$A")[i1 + 1]  +  (%"sub_$B")[i1 + 2];
;               |      (%"sub_$B")[i1 + 2] = %add.1;
;               |   }
;               |   else
;               |   {
;               |      %"(float)sub_$I_fetch.10$" = sitofp.i32.float(i1 + 3);
;               |      (%"sub_$A")[i1 + 2] = %"(float)sub_$I_fetch.10$";
;               |   }
;               + END LOOP
;         END REGION

;*** IR Dump After HIR Loop Reversal (hir-loop-reversal) ***
;CHECK:   BEGIN REGION { }
;CHECK:         + DO i1 = 0, sext.i32.i64((2 * %"sub_$N_fetch.1")) + -3, 1   <DO_LOOP>
;CHECK:         |   if ((%"sub_$A")[i1 + 2] !=u 1.000000e+02)
;CHECK:         |   {
;CHECK:         |      %"(float)sub_$I_fetch.10$" = sitofp.i32.float(i1 + 3);
;CHECK:         |      (%"sub_$A")[i1 + 2] = %"(float)sub_$I_fetch.10$";
;               |   }
;CHECK:         |   else
;               |   {
;               |      %add.1 = (%"sub_$A")[i1 + 1]  +  (%"sub_$B")[i1 + 2];
;               |      (%"sub_$B")[i1 + 2] = %add.1;
;               |   }
;CHECK:         + END LOOP
;CHECK:   END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @sub_(float* noalias nocapture dereferenceable(4) %"sub_$A", i32* noalias nocapture readonly dereferenceable(4) %"sub_$N", i32 %"sub_$N_fetch.1") #0 {
alloca_0:
  %"sub_$N_fetch.11" = load i32, i32* null, align 1
  %mul.1 = shl i32 %"sub_$N_fetch.1", 1
  br label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %0 = or i32 %mul.1, 1
  %wide.trip.count = sext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb6_endif, %bb2.preheader
  %indvars.iv = phi i64 [ 3, %bb2.preheader ], [ %indvars.iv.next, %bb6_endif ]
  %"sub_$A_entry[]10" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$A", i64 %indvars.iv)
  %"sub_$A_entry[]_fetch.4" = load float, float* %"sub_$A_entry[]10", align 1
  %rel.2 = fcmp reassoc ninf nsz arcp contract afn oeq float %"sub_$A_entry[]_fetch.4", 1.000000e+02
  br i1 %rel.2, label %bb_new3_then, label %bb_new4_else

bb_new3_then:                                     ; preds = %bb2
  %1 = add nsw i64 1, -1
  %"sub_$A_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$A", i64 3)
  %"sub_$A_entry[]_fetch.6" = load float, float* %"sub_$A_entry[]", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"sub_$A_entry[]_fetch.6", 0.000000e+00
  br label %bb6_endif

bb_new4_else:                                     ; preds = %bb2
  %2 = trunc i64 %indvars.iv to i32
  %"(float)sub_$I_fetch.10$" = sitofp i32 %2 to float
  store float %"(float)sub_$I_fetch.10$", float* %"sub_$A_entry[]10", align 1
  br label %bb6_endif

bb6_endif:                                        ; preds = %bb_new4_else, %bb_new3_then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb6_endif
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }
