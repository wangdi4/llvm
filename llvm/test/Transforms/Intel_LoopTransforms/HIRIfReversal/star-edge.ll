; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-if-reversal  -print-after=hir-if-reversal -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-if-reversal" -print-after=hir-if-reversal -aa-pipeline="basic-aa" -disable-output 2>&1 < %s  | FileCheck %s

; Check we do not reverse the if for edges that are not LT (<)
; DDEdge Candidate: 17:13 (%"sub_$A")[%"sub_$N_fetch.1"] --> (%"sub_$A")[i1 + 2] ANTI (*) (?)

; CHECK: BEGIN REGION { }
; CHECK:     + DO i1 = 0, sext.i32.i64((2 * %"sub_$N_fetch.1")) + -3, 1   <DO_LOOP>
; CHECK-NOT: |   if ((%"sub_$C")[i1 + 2] !=u 1.000000e+02)
; CHECK:     |   if ((%"sub_$C")[i1 + 2] == 1.000000e+02)
;            |   {
;            |      (%"sub_$A")[i1 + 2] = (%"sub_$B")[i1 + 2];
;            |   }
;            |   else
;            |   {
;            |      (%"sub_$B")[i1 + 2] = (%"sub_$A")[%"sub_$N_fetch.1"];
;            |   }
;            + END LOOP
;      END REGION

define void @sub_(float* noalias nocapture dereferenceable(4) %"sub_$A", float* noalias nocapture dereferenceable(4) %"sub_$B", float* noalias nocapture readonly dereferenceable(4) %"sub_$C", i32* noalias nocapture readonly dereferenceable(4) %"sub_$N") local_unnamed_addr #0 {
alloca_0:
  %"sub_$N_fetch.1" = load i32, i32* %"sub_$N", align 1, !tbaa !0
  %mul.1 = shl i32 %"sub_$N_fetch.1", 1
  %rel.1 = icmp slt i32 %mul.1, 3
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %int_sext2 = sext i32 %"sub_$N_fetch.1" to i64
  %"sub_$A_entry[]4" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$A", i64 %int_sext2)
  %0 = or i32 %mul.1, 1
  %wide.trip.count = sext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb6_endif
  %indvars.iv = phi i64 [ 3, %bb2.preheader ], [ %indvars.iv.next, %bb6_endif ]
  %"sub_$C_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$C", i64 %indvars.iv)
  %"sub_$C_entry[]_fetch.4" = load float, float* %"sub_$C_entry[]", align 1, !tbaa !4
  %rel.2 = fcmp reassoc ninf nsz arcp contract afn oeq float %"sub_$C_entry[]_fetch.4", 1.000000e+02
  %"sub_$B_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$B", i64 %indvars.iv)
  br i1 %rel.2, label %bb_new3_then, label %bb_new4_else

bb_new3_then:                                     ; preds = %bb2
  %"sub_$B_entry[]_fetch.6" = load float, float* %"sub_$B_entry[]", align 1, !tbaa !6
  %1 = add nsw i64 %indvars.iv, -1
  %"sub_$A_entry[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub_$A", i64 %1)
  store float %"sub_$B_entry[]_fetch.6", float* %"sub_$A_entry[]", align 1, !tbaa !8
  br label %bb6_endif

bb_new4_else:                                     ; preds = %bb2
  %"sub_$A_entry[]_fetch.9" = load float, float* %"sub_$A_entry[]4", align 1, !tbaa !8
  store float %"sub_$A_entry[]_fetch.9", float* %"sub_$B_entry[]", align 1, !tbaa !6
  br label %bb6_endif

bb6_endif:                                        ; preds = %bb_new4_else, %bb_new3_then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb6_endif
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$sub_"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$3", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$4", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$5", !2, i64 0}
