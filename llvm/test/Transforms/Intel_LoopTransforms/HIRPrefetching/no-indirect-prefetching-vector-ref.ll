; Check that indirect prefetching is disabled for vector refs
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -hir-prefetching -hir-cg -print-after=hir-prefetching < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-prefetching,print<hir>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Prefetching (hir-prefetching) ***
;Function: sub1_
;
;<0>          BEGIN REGION { modified }
;<3>                if (%"sub1_$N_fetch.18" >= 1)
;<3>                {
;<34>                  %tgu = (zext.i32.i64(%"sub1_$N_fetch.18"))/u2;
;<36>                  if (0 <u 2 * %tgu)
;<36>                  {
;<35>                     + DO i1 = 0, 2 * %tgu + -1, 2   <DO_LOOP>  <MAX_TC_EST = 50> <auto-vectorized> <nounroll> <novectorize> <ivdep>
;<42>                     |   %.vec = (<2 x i32>*)(@"sub1_$JJ")[0][i1];
;<43>                     |   %.vec2 = (<2 x float>*)(%"sub1_$A")[%.vec];
;<44>                     |   %.vec3 = %.vec2  +  1.000000e+00;
;<45>                     |   (<2 x float>*)(%"sub1_$A")[i1 + 1] = %.vec3;
;<35>                     + END LOOP
;<36>                  }
;<31>
;<31>                  + DO i1 = 2 * %tgu, zext.i32.i64(%"sub1_$N_fetch.18") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1> <nounroll> <novectorize> <ivdep> <max_trip_count = 1>
;<13>                  |   %"sub1_$JJ[]_fetch.86" = (@"sub1_$JJ")[0][i1];
;<17>                  |   %add.22 = (%"sub1_$A")[%"sub1_$JJ[]_fetch.86"]  +  1.000000e+00;
;<19>                  |   (%"sub1_$A")[i1 + 1] = %add.22;
;<31>                  + END LOOP
;<3>                }
;<30>               ret ;
;<0>          END REGION
;
; CHECK-NOT:    @llvm.prefetch.p0i8
;
; ModuleID = 'all'
source_filename = "/tmp/ifxApC43i.i"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"sub1_$JJ" = external hidden global [100 x i32], align 16
@"sub1_$II" = external hidden global [100 x i32], align 16
@xx_ = external unnamed_addr global [4 x i8], align 32

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #0

; Function Attrs: nounwind uwtable
define void @sub1_(float* noalias nocapture dereferenceable(4) %"sub1_$A", i32* noalias nocapture readonly dereferenceable(4) %"sub1_$N", float* noalias nocapture dereferenceable(4) %"sub1_$B") local_unnamed_addr #1 {

entry:
  %"sub1_$A_entry" = bitcast float* %"sub1_$A" to [100 x float]*
  %"sub1_$N_fetch.18" = load i32, i32* %"sub1_$N", align 1, !tbaa !0
  %rel.5 = icmp slt i32 %"sub1_$N_fetch.18", 1
  br i1 %rel.5, label %bb35.loopexit, label %bb35
bb35.loopexit:
  br label %bb35

bb35:                                             ; preds = %bb35.loopexit, %bb31
  %t5 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"([100 x i32]* @"sub1_$JJ"), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 0), "QUAL.PRAGMA.VAR"([100 x float]* %"sub1_$A_entry"), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1) ]
  br i1 %rel.5, label %bb39, label %bb38.preheader

bb38.preheader:                                   ; preds = %bb35
  %t6 = add nuw nsw i32 %"sub1_$N_fetch.18", 1
  %wide.trip.count134 = zext i32 %t6 to i64
  br label %bb38

bb38:                                             ; preds = %bb38, %bb38.preheader
  %indvars.iv = phi i64 [ 1, %bb38.preheader ], [ %indvars.iv.next, %bb38 ]
  %"sub1_$JJ[]59" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) getelementptr inbounds ([100 x i32], [100 x i32]* @"sub1_$JJ", i64 0, i64 0), i64 %indvars.iv)
  %"sub1_$JJ[]_fetch.86" = load i32, i32* %"sub1_$JJ[]59", align 1, !tbaa !6
  %int_sext60 = sext i32 %"sub1_$JJ[]_fetch.86" to i64
  %"sub1_$A_entry[]62" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub1_$A", i64 %int_sext60)
  %"sub1_$A_entry[]_fetch.88" = load float, float* %"sub1_$A_entry[]62", align 1, !tbaa !10
  %add.22 = fadd reassoc ninf nsz arcp contract afn float %"sub1_$A_entry[]_fetch.88", 1.000000e+00
  %"sub1_$A_entry[]65" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* nonnull elementtype(float) %"sub1_$A", i64 %indvars.iv)
  store float %add.22, float* %"sub1_$A_entry[]65", align 1, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count134
  br i1 %exitcond, label %bb39.loopexit, label %bb38, !llvm.loop !20

bb39.loopexit:                                    ; preds = %bb38
  br label %bb39

bb39:                                             ; preds = %bb39.loopexit, %bb35
  call void @llvm.directive.region.exit(token %t5) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="corei7-avx" "target-features"="+avx,+crc32,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #2 = { nounwind }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$10", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"Simple Fortran Alias Analysis 2"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$12", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$13", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$15", !2, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$17", !2, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$19", !14, i64 0}
!14 = !{!"Generic Fortran Symbol", !15, i64 0}
!15 = !{!"Simple Fortran Alias Analysis 3"}
!16 = distinct !{!16, !17}
!17 = !{!"llvm.loop.distribute.enable", i1 true}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.unroll.count", i32 5}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.vectorize.ivdep_back"}
