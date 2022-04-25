; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -analyze -enable-new-pm=0 -hir-array-section-analysis | FileCheck %s
; RUN: opt < %s -aa-pipeline=basic-aa -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-array-section-analysis>" -disable-output 2>&1 | FileCheck %s

; Outer dimension is set to [*:*:*] because not all refs in @fmcom_ group have it.

; BEGIN REGION { }
;       + DO i1 = 0, sext.i32.i64(%"wtot_$NBF_fetch.i.i") + -1, 1   <DO_LOOP>
;       |   + DO i2 = 0, i1, 1   <DO_LOOP>
;       |   |      %"wtot_$DUM.2.i.i" = %"wtot_$DUM.0.i.i";
;       |   |   + DO i3 = 0, sext.i32.i64(%"wtot_$NOC_fetch.i.i") + -1, 1   <DO_LOOP>
;       |   |   |   %mul17.i.i = (bitcast ([8 x i8]* @fmcom_ to double*))[i3][i1 + -1]  *  (bitcast ([8 x i8]* @fmcom_ to double*))[i3 + -1];
;       |   |   |   %mul30.i.i = %mul17.i.i  *  (bitcast ([8 x i8]* @fmcom_ to double*))[i3][i2 + -1];
;       |   |   |   %"wtot_$DUM.2.i.i" = %"wtot_$DUM.2.i.i"  -  %mul30.i.i;
;       |   |   + END LOOP
;       |   |      %"wtot_$DUM.0.i.i" = %"wtot_$DUM.2.i.i";
;       |   |
;       |   |   %mul46.i.i = %"wtot_$DUM.0.i.i"  *  2.000000e+00;
;       |   |   %add48.i.i.lcssa33 = %add48.i.i.lcssa33  +  %mul46.i.i;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: + DO i3 = 0, sext.i32.i64(%"wtot_$NOC_fetch.i.i") + -1, 1   <DO_LOOP>
; CHECK: bitcast ([8 x i8]* @fmcom_ to double*): (USE) [*:*:*][i3 + -1,i2 + -1,i1 + -1:-1:i1 + -1]
; CHECK: + END LOOP

; ModuleID = 'groups-diff-num-dims.ll'
source_filename = "cisgrd.test.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@fmcom_ = external unnamed_addr global [8 x i8], align 32
@0 = external hidden unnamed_addr constant i32

; Function Attrs: nounwind uwtable
define void @cisao_() local_unnamed_addr #0 {
alloca_0:
  %"cisao_$NVIR" = alloca i32, align 8
  %"cisao_$NOC" = alloca i32, align 8
  %"cisao_$NBF2" = alloca i32, align 8
  %"cisao_$NBF" = alloca i32, align 8
  %"val$[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) bitcast ([8 x i8]* @fmcom_ to double*), i64 0)
  call void (...) @cisaod_(double* nonnull %"val$[]", double* nonnull %"val$[]", double* nonnull %"val$[]", double* nonnull %"val$[]", double* nonnull %"val$[]", double* nonnull %"val$[]", double* nonnull %"val$[]", i32* nonnull %"cisao_$NBF", i32* nonnull %"cisao_$NBF2", i32* nonnull %"cisao_$NOC", i32* nonnull %"cisao_$NVIR", i32* nonnull @0) #2
  %"wtot_$NBF_fetch.i.i" = load i32, i32* %"cisao_$NBF", align 8, !alias.scope !0, !noalias !3
  %int_sext.i.i = sext i32 %"wtot_$NBF_fetch.i.i" to i64
  %mul.i.i = shl nsw i64 %int_sext.i.i, 3
  %rel.i.i = icmp slt i32 %"wtot_$NBF_fetch.i.i", 1
  br i1 %rel.i.i, label %wtot_.t0p.t0p.t0p.t1p.t1p.t0p.exit, label %bb39.i.i.preheader

bb39.i.i.preheader:                               ; preds = %alloca_0
  %"wtot_$NOC_fetch.i.i" = load i32, i32* %"cisao_$NOC", align 8, !alias.scope !9, !noalias !10
  %rel6.i.i = icmp slt i32 %"wtot_$NOC_fetch.i.i", 1
  %"wtot_$W2[].i.i" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"val$[]", i64 0) #2
  %"wtot_$W2[].i.i.promoted32" = load double, double* %"wtot_$W2[].i.i", align 1, !alias.scope !11, !noalias !12
  %0 = add nuw nsw i32 %"wtot_$NOC_fetch.i.i", 1
  %1 = add nuw nsw i32 %"wtot_$NBF_fetch.i.i", 1
  %wide.trip.count44 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %bb39.i.i

bb39.i.i:                                         ; preds = %bb44.i.i, %bb39.i.i.preheader
  %indvars.iv42 = phi i64 [ 1, %bb39.i.i.preheader ], [ %indvars.iv.next43, %bb44.i.i ]
  %indvars.iv40 = phi i64 [ 2, %bb39.i.i.preheader ], [ %indvars.iv.next41, %bb44.i.i ]
  %add48.i.i.lcssa33 = phi double [ %"wtot_$W2[].i.i.promoted32", %bb39.i.i.preheader ], [ %add48.i.i.lcssa, %bb44.i.i ]
  %"wtot_$DUM.0.i.i" = phi double [ undef, %bb39.i.i.preheader ], [ %"wtot_$DUM.3.i.i.lcssa", %bb44.i.i ]
  br label %bb43.i.i

bb43.i.i:                                         ; preds = %bb48.i.i, %bb39.i.i
  %indvars.iv34 = phi i64 [ %indvars.iv.next35, %bb48.i.i ], [ 1, %bb39.i.i ]
  %add48.i.i31 = phi double [ %add48.i.i, %bb48.i.i ], [ %add48.i.i.lcssa33, %bb39.i.i ]
  %"wtot_$DUM.1.i.i" = phi double [ %"wtot_$DUM.3.i.i", %bb48.i.i ], [ %"wtot_$DUM.0.i.i", %bb39.i.i ]
  br i1 %rel6.i.i, label %bb48.i.i, label %bb47.i.i.preheader

bb47.i.i.preheader:                               ; preds = %bb43.i.i
  br label %bb47.i.i

bb47.i.i:                                         ; preds = %bb47.i.i, %bb47.i.i.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb47.i.i ], [ 1, %bb47.i.i.preheader ]
  %"wtot_$DUM.2.i.i" = phi double [ %sub.i.i, %bb47.i.i ], [ %"wtot_$DUM.1.i.i", %bb47.i.i.preheader ]
  %"wtot_$E[].i.i" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"val$[]", i64 %indvars.iv) #2
  %"wtot_$E[]_fetch.i.i" = load double, double* %"wtot_$E[].i.i", align 1, !alias.scope !13, !noalias !14
  %"wtot_$C[].i.i" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.i.i, double* elementtype(double) nonnull %"val$[]", i64 %indvars.iv) #2
  %"wtot_$C[][].i.i" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"wtot_$C[].i.i", i64 %indvars.iv42) #2
  %"wtot_$C[][]_fetch.i.i" = load double, double* %"wtot_$C[][].i.i", align 1, !alias.scope !15, !noalias !16
  %mul17.i.i = fmul fast double %"wtot_$C[][]_fetch.i.i", %"wtot_$E[]_fetch.i.i"
  %"wtot_$C[]28[].i.i" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"wtot_$C[].i.i", i64 %indvars.iv34) #2
  %"wtot_$C[]28[]_fetch.i.i" = load double, double* %"wtot_$C[]28[].i.i", align 1, !alias.scope !15, !noalias !16
  %mul30.i.i = fmul fast double %mul17.i.i, %"wtot_$C[]28[]_fetch.i.i"
  %sub.i.i = fsub fast double %"wtot_$DUM.2.i.i", %mul30.i.i
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb48.i.i.loopexit, label %bb47.i.i

bb48.i.i.loopexit:                                ; preds = %bb47.i.i
  %sub.i.i.lcssa = phi double [ %sub.i.i, %bb47.i.i ]
  br label %bb48.i.i

bb48.i.i:                                         ; preds = %bb48.i.i.loopexit, %bb43.i.i
  %"wtot_$DUM.3.i.i" = phi double [ %"wtot_$DUM.1.i.i", %bb43.i.i ], [ %sub.i.i.lcssa, %bb48.i.i.loopexit ]
  %mul46.i.i = fmul fast double %"wtot_$DUM.3.i.i", 2.000000e+00
  %add48.i.i = fadd fast double %add48.i.i31, %mul46.i.i
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond39 = icmp eq i64 %indvars.iv.next35, %indvars.iv40
  br i1 %exitcond39, label %bb44.i.i, label %bb43.i.i

bb44.i.i:                                         ; preds = %bb48.i.i
  %"wtot_$DUM.3.i.i.lcssa" = phi double [ %"wtot_$DUM.3.i.i", %bb48.i.i ]
  %add48.i.i.lcssa = phi double [ %add48.i.i, %bb48.i.i ]
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next43, %wide.trip.count44
  br i1 %exitcond45, label %wtot_.t0p.t0p.t0p.t1p.t1p.t0p.exit.loopexit, label %bb39.i.i

wtot_.t0p.t0p.t0p.t1p.t1p.t0p.exit.loopexit:      ; preds = %bb44.i.i
  %add48.i.i.lcssa.lcssa = phi double [ %add48.i.i.lcssa, %bb44.i.i ]
  store double %add48.i.i.lcssa.lcssa, double* %"wtot_$W2[].i.i", align 1, !alias.scope !11, !noalias !12
  br label %wtot_.t0p.t0p.t0p.t1p.t1p.t0p.exit

wtot_.t0p.t0p.t0p.t1p.t1p.t0p.exit:               ; preds = %wtot_.t0p.t0p.t0p.t1p.t1p.t0p.exit.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare void @cisaod_(...) local_unnamed_addr

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{}

!0 = !{!1}
!1 = distinct !{!1, !2, !"wtot_: %wtot_$NBF"}
!2 = distinct !{!2, !"wtot_"}
!3 = !{!4, !5, !6, !7, !8}
!4 = distinct !{!4, !2, !"wtot_: %wtot_$C"}
!5 = distinct !{!5, !2, !"wtot_: %wtot_$E"}
!6 = distinct !{!6, !2, !"wtot_: %wtot_$W2"}
!7 = distinct !{!7, !2, !"wtot_: %wtot_$NOC"}
!8 = distinct !{!8, !2, !"wtot_: %wtot_$OCC"}
!9 = !{!7}
!10 = !{!4, !5, !6, !1, !8}
!11 = !{!6}
!12 = !{!4, !5, !7, !1, !8}
!13 = !{!5}
!14 = !{!4, !6, !7, !1, !8}
!15 = !{!4}
!16 = !{!5, !6, !7, !1, !8}
