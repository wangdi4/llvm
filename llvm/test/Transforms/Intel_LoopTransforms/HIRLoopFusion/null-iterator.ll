; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output %s 2>&1 | FileCheck %s

; Check fusion does not choke during sorting on the last fuseNode when calling getNextNode()

; CHECK:  BEGIN REGION { modified }
; CHECK:        %mul.31 = %add.10  *  %"(float)miller_mod_mp_mmode__fetch.15$";
; CHECK:        + DO i1 = 0, sext.i32.i64((500 + (500 * %miller_mod_mp_n_pol__fetch.9))) + -1, 1   <DO_LOOP>

; Check for fusion of all i1 loops
; CHECK-NOT: DO i1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@miller_mod_mp_thetan_ = common local_unnamed_addr global float 0.000000e+00, align 8, !llfort.type_idx !0
@miller_mod_mp_thetam_ = common local_unnamed_addr global float 0.000000e+00, align 8, !llfort.type_idx !1
@miller_mod_mp_deltan_ = common local_unnamed_addr global float 0.000000e+00, align 8, !llfort.type_idx !2
@miller_mod_mp_deltam_ = common local_unnamed_addr global float 0.000000e+00, align 8, !llfort.type_idx !3
@miller_mod_mp_rho_ = common local_unnamed_addr global float 0.000000e+00, align 8, !llfort.type_idx !4
@miller_mod_mp_nmode_ = common local_unnamed_addr global i32 0, align 8, !llfort.type_idx !5
@miller_mod_mp_mmode_ = common local_unnamed_addr global i32 0, align 8, !llfort.type_idx !6
@miller_mod_mp_major_z_ = common local_unnamed_addr global i32 0, align 8, !llfort.type_idx !7
@miller_mod_mp_n_pol_ = common local_unnamed_addr global i32 0, align 8, !llfort.type_idx !8

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
define void @miller_mod._() local_unnamed_addr #0 {
alloca_0:
  ret void
}

; Function Attrs: nofree nosync nounwind readonly uwtable
define void @miller_mod_mp_get_miller_() local_unnamed_addr #1 {
alloca_1:
  %miller_mod_mp_n_pol__fetch.9 = load i32, ptr @miller_mod_mp_n_pol_, align 8, !tbaa !9
  %0 = mul i32 %miller_mod_mp_n_pol__fetch.9, 500
  %mul.1 = add i32 %0, 500
  %int_sext = sext i32 %mul.1 to i64
  %1 = tail call i64 @llvm.smax.i64(i64 %int_sext, i64 0)
  %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2" = alloca float, i64 %1, align 4, !llfort.type_idx !13
  %"miller_mod_mp_get_miller_$DTRSHAPE_S6" = alloca float, i64 %1, align 4, !llfort.type_idx !14
  %"miller_mod_mp_get_miller_$RSHAPE_S10" = alloca float, i64 %1, align 4, !llfort.type_idx !15
  %"miller_mod_mp_get_miller_$Z_THETA_S14" = alloca float, i64 %1, align 4, !llfort.type_idx !16
  %"miller_mod_mp_get_miller_$R_THETA_S18" = alloca float, i64 %1, align 4, !llfort.type_idx !17
  %"miller_mod_mp_get_miller_$Z_S22" = alloca float, i64 %1, align 4, !llfort.type_idx !18
  %"miller_mod_mp_get_miller_$THADJ_S30" = alloca float, i64 %1, align 4, !llfort.type_idx !19
  %miller_mod_mp_major_z__fetch.10 = load i32, ptr @miller_mod_mp_major_z_, align 8, !tbaa !20
  %"(float)miller_mod_mp_major_z__fetch.10$" = sitofp i32 %miller_mod_mp_major_z__fetch.10 to float
  %miller_mod_mp_rho__fetch.13 = load float, ptr @miller_mod_mp_rho_, align 8, !tbaa !22
  %miller_mod_mp_deltam__fetch.14 = load float, ptr @miller_mod_mp_deltam_, align 8, !tbaa !24
  %miller_mod_mp_mmode__fetch.15 = load i32, ptr @miller_mod_mp_mmode_, align 8, !tbaa !26
  %miller_mod_mp_thetam__fetch.19 = load float, ptr @miller_mod_mp_thetam_, align 8, !tbaa !28
  %rel.11.not145 = icmp slt i32 %mul.1, 1
  br i1 %rel.11.not145, label %loop_exit53, label %loop_body13.lr.ph

loop_body13.lr.ph:                                ; preds = %alloca_1
  %add.10 = fadd reassoc ninf nsz arcp contract afn float %miller_mod_mp_deltam__fetch.14, -1.000000e+00
  %"(float)miller_mod_mp_mmode__fetch.15$" = sitofp i32 %miller_mod_mp_mmode__fetch.15 to float
  %2 = or i64 %int_sext, 1
  br label %loop_body13

loop_test19.preheader:                            ; preds = %loop_body13
  br label %loop_body27.preheader

loop_body13:                                      ; preds = %loop_body13.lr.ph, %loop_body13
  %"$loop_ctr.0146" = phi i64 [ 1, %loop_body13.lr.ph ], [ %add.14, %loop_body13 ]
  %"miller_mod_mp_get_miller_$THADJ_S30[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$THADJ_S30", i64 %"$loop_ctr.0146"), !llfort.type_idx !30
  %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.18" = load float, ptr %"miller_mod_mp_get_miller_$THADJ_S30[]", align 4, !tbaa !31
  %add.12 = fadd reassoc ninf nsz arcp contract afn float %miller_mod_mp_thetam__fetch.19, %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.18"
  %mul.19 = fmul reassoc ninf nsz arcp contract afn float %add.12, %"(float)miller_mod_mp_mmode__fetch.15$"
  %func_result32 = tail call reassoc ninf nsz arcp contract afn float @llvm.cos.f32(float %mul.19), !llfort.type_idx !33
  %sub.5 = fsub reassoc ninf nsz arcp contract afn float 1.000000e+00, %func_result32
  %mul.20 = fmul reassoc ninf nsz arcp contract afn float %add.10, %sub.5
  %div.9 = fmul reassoc ninf nsz arcp contract afn float %mul.20, 5.000000e-01
  %add.13 = fadd reassoc ninf nsz arcp contract afn float %div.9, 1.000000e+00
  %mul.21 = fmul reassoc ninf nsz arcp contract afn float %miller_mod_mp_rho__fetch.13, %add.13
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$RSHAPE_S10", i64 %"$loop_ctr.0146"), !llfort.type_idx !34
  store float %mul.21, ptr %"miller_mod_mp_get_miller_$RSHAPE_S10[]", align 4, !tbaa !35
  %add.14 = add nuw nsw i64 %"$loop_ctr.0146", 1
  %exitcond = icmp eq i64 %add.14, %2
  br i1 %exitcond, label %loop_test19.preheader, label %loop_body13

loop_body27.preheader:                            ; preds = %loop_test19.preheader
  br label %loop_body27

loop_test33.preheader:                            ; preds = %loop_body27
  br label %loop_body34.lr.ph

loop_body34.lr.ph:                                ; preds = %loop_test33.preheader
  %mul.31 = fmul reassoc ninf nsz arcp contract afn float %add.10, %"(float)miller_mod_mp_mmode__fetch.15$"
  br label %loop_body34

loop_body27:                                      ; preds = %loop_body27.preheader, %loop_body27
  %"$loop_ctr43.0148" = phi i64 [ %add.24, %loop_body27 ], [ 1, %loop_body27.preheader ]
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]46" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$RSHAPE_S10", i64 %"$loop_ctr43.0148"), !llfort.type_idx !37
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]_fetch.38" = load float, ptr %"miller_mod_mp_get_miller_$RSHAPE_S10[]46", align 4, !tbaa !35
  %"miller_mod_mp_get_miller_$THADJ_S30[]47" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$THADJ_S30", i64 %"$loop_ctr43.0148"), !llfort.type_idx !38
  %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.41" = load float, ptr %"miller_mod_mp_get_miller_$THADJ_S30[]47", align 4, !tbaa !31
  %func_result45 = tail call reassoc ninf nsz arcp contract afn float @llvm.sin.f32(float %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.41"), !llfort.type_idx !33
  %mul.29 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$RSHAPE_S10[]_fetch.38", %func_result45
  %add.23 = fadd reassoc ninf nsz arcp contract afn float %mul.29, %"(float)miller_mod_mp_major_z__fetch.10$"
  %"miller_mod_mp_get_miller_$Z_S22[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$Z_S22", i64 %"$loop_ctr43.0148"), !llfort.type_idx !39
  store float %add.23, ptr %"miller_mod_mp_get_miller_$Z_S22[]", align 4, !tbaa !40
  %add.24 = add nuw nsw i64 %"$loop_ctr43.0148", 1
  %exitcond155 = icmp eq i64 %add.24, %2
  br i1 %exitcond155, label %loop_test33.preheader, label %loop_body27

loop_test42.preheader:                            ; preds = %loop_body34
  br label %loop_body43.preheader

loop_body43.preheader:                            ; preds = %loop_test42.preheader
  br label %loop_body43

loop_body34:                                      ; preds = %loop_body34.lr.ph, %loop_body34
  %"$loop_ctr51.0150" = phi i64 [ 1, %loop_body34.lr.ph ], [ %add.29, %loop_body34 ]
  %"miller_mod_mp_get_miller_$THADJ_S30[]54" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$THADJ_S30", i64 %"$loop_ctr51.0150"), !llfort.type_idx !42
  %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.52" = load float, ptr %"miller_mod_mp_get_miller_$THADJ_S30[]54", align 4, !tbaa !31
  %add.28 = fadd reassoc ninf nsz arcp contract afn float %miller_mod_mp_thetam__fetch.19, %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.52"
  %mul.33 = fmul reassoc ninf nsz arcp contract afn float %add.28, %"(float)miller_mod_mp_mmode__fetch.15$"
  %func_result53 = tail call reassoc ninf nsz arcp contract afn float @llvm.sin.f32(float %mul.33), !llfort.type_idx !33
  %mul.34 = fmul reassoc ninf nsz arcp contract afn float %mul.31, %func_result53
  %div.10 = fmul reassoc ninf nsz arcp contract afn float %mul.34, 5.000000e-01
  %mul.35 = fmul reassoc ninf nsz arcp contract afn float %miller_mod_mp_rho__fetch.13, %div.10
  %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$DTRSHAPE_S6", i64 %"$loop_ctr51.0150"), !llfort.type_idx !43
  store float %mul.35, ptr %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]", align 4, !tbaa !44
  %add.29 = add nuw nsw i64 %"$loop_ctr51.0150", 1
  %exitcond156 = icmp eq i64 %add.29, %2
  br i1 %exitcond156, label %loop_test42.preheader, label %loop_body34

loop_test51.preheader:                            ; preds = %loop_body43
  br label %loop_body52.preheader

loop_body52.preheader:                            ; preds = %loop_test51.preheader
  br label %loop_body52

loop_body43:                                      ; preds = %loop_body43.preheader, %loop_body43
  %"$loop_ctr57.0152" = phi i64 [ %add.36, %loop_body43 ], [ 1, %loop_body43.preheader ]
  %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2", i64 %"$loop_ctr57.0152"), !llfort.type_idx !46
  %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]_fetch.60" = load float, ptr %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]", align 4, !tbaa !47
  %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]64" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$DTRSHAPE_S6", i64 %"$loop_ctr57.0152"), !llfort.type_idx !49
  %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]_fetch.63" = load float, ptr %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]64", align 4, !tbaa !44
  %"miller_mod_mp_get_miller_$THADJ_S30[]65" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$THADJ_S30", i64 %"$loop_ctr57.0152"), !llfort.type_idx !50
  %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.66" = load float, ptr %"miller_mod_mp_get_miller_$THADJ_S30[]65", align 4, !tbaa !31
  %func_result59 = tail call reassoc ninf nsz arcp contract afn float @llvm.cos.f32(float %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.66"), !llfort.type_idx !33
  %mul.40 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]_fetch.63", %func_result59
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]60" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$RSHAPE_S10", i64 %"$loop_ctr57.0152"), !llfort.type_idx !51
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]60_fetch.69" = load float, ptr %"miller_mod_mp_get_miller_$RSHAPE_S10[]60", align 4, !tbaa !35
  %func_result63 = tail call reassoc ninf nsz arcp contract afn float @llvm.sin.f32(float %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.66"), !llfort.type_idx !33
  %mul.43 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$RSHAPE_S10[]60_fetch.69", %func_result63
  %sub.28 = fsub reassoc ninf nsz arcp contract afn float %mul.40, %mul.43
  %mul.44 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]_fetch.60", %sub.28
  %"miller_mod_mp_get_miller_$R_THETA_S18[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$R_THETA_S18", i64 %"$loop_ctr57.0152"), !llfort.type_idx !52
  store float %mul.44, ptr %"miller_mod_mp_get_miller_$R_THETA_S18[]", align 4, !tbaa !53
  %add.36 = add nuw nsw i64 %"$loop_ctr57.0152", 1
  %exitcond157 = icmp eq i64 %add.36, %2
  br i1 %exitcond157, label %loop_test51.preheader, label %loop_body43

loop_body52:                                      ; preds = %loop_body52.preheader, %loop_body52
  %"$loop_ctr72.0154" = phi i64 [ %add.44, %loop_body52 ], [ 1, %loop_body52.preheader ]
  %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]79" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2", i64 %"$loop_ctr72.0154"), !llfort.type_idx !55
  %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]_fetch.79" = load float, ptr %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]79", align 4, !tbaa !47
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]80" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$RSHAPE_S10", i64 %"$loop_ctr72.0154"), !llfort.type_idx !56
  %"miller_mod_mp_get_miller_$RSHAPE_S10[]_fetch.82" = load float, ptr %"miller_mod_mp_get_miller_$RSHAPE_S10[]80", align 4, !tbaa !35
  %"miller_mod_mp_get_miller_$THADJ_S30[]81" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$THADJ_S30", i64 %"$loop_ctr72.0154"), !llfort.type_idx !57
  %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.85" = load float, ptr %"miller_mod_mp_get_miller_$THADJ_S30[]81", align 4, !tbaa !31
  %func_result74 = tail call reassoc ninf nsz arcp contract afn float @llvm.cos.f32(float %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.85"), !llfort.type_idx !33
  %mul.49 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$RSHAPE_S10[]_fetch.82", %func_result74
  %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]75" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$DTRSHAPE_S6", i64 %"$loop_ctr72.0154"), !llfort.type_idx !58
  %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]75_fetch.88" = load float, ptr %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]75", align 4, !tbaa !44
  %func_result78 = tail call reassoc ninf nsz arcp contract afn float @llvm.sin.f32(float %"miller_mod_mp_get_miller_$THADJ_S30[]_fetch.85"), !llfort.type_idx !33
  %mul.52 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$DTRSHAPE_S6[]75_fetch.88", %func_result78
  %add.43 = fadd reassoc ninf nsz arcp contract afn float %mul.49, %mul.52
  %mul.53 = fmul reassoc ninf nsz arcp contract afn float %"miller_mod_mp_get_miller_$DTHETA_DCHI_S2[]_fetch.79", %add.43
  %"miller_mod_mp_get_miller_$Z_THETA_S14[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"miller_mod_mp_get_miller_$Z_THETA_S14", i64 %"$loop_ctr72.0154"), !llfort.type_idx !59
  store float %mul.53, ptr %"miller_mod_mp_get_miller_$Z_THETA_S14[]", align 4, !tbaa !60
  %add.44 = add nuw nsw i64 %"$loop_ctr72.0154", 1
  %exitcond158 = icmp eq i64 %add.44, %2
  br i1 %exitcond158, label %loop_exit53.loopexit, label %loop_body52

loop_exit53.loopexit:                             ; preds = %loop_body52
  br label %loop_exit53

loop_exit53:                                      ; preds = %loop_exit53.loopexit, %alloca_1
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare !llfort.intrin_id !62 !llfort.type_idx !63 float @llvm.cos.f32(float) #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare !llfort.intrin_id !64 !llfort.type_idx !65 float @llvm.sin.f32(float) #3

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i64 @llvm.smax.i64(i64, i64) #4

attributes #0 = { mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree nosync nounwind readonly uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!omp_offload.info = !{}

!0 = !{i64 19}
!1 = !{i64 20}
!2 = !{i64 21}
!3 = !{i64 22}
!4 = !{i64 23}
!5 = !{i64 24}
!6 = !{i64 25}
!7 = !{i64 26}
!8 = !{i64 27}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$1", !11, i64 0}
!11 = !{!"Generic Fortran Symbol", !12, i64 0}
!12 = !{!"ifx$root$1$miller_mod_mp_get_miller_"}
!13 = !{i64 43}
!14 = !{i64 45}
!15 = !{i64 47}
!16 = !{i64 49}
!17 = !{i64 51}
!18 = !{i64 53}
!19 = !{i64 57}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$2", !11, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$5", !11, i64 0}
!24 = !{!25, !25, i64 0}
!25 = !{!"ifx$unique_sym$6", !11, i64 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"ifx$unique_sym$7", !11, i64 0}
!28 = !{!29, !29, i64 0}
!29 = !{!"ifx$unique_sym$9", !11, i64 0}
!30 = !{i64 60}
!31 = !{!32, !32, i64 0}
!32 = !{!"ifx$unique_sym$8", !11, i64 0}
!33 = !{i64 5}
!34 = !{i64 59}
!35 = !{!36, !36, i64 0}
!36 = !{!"ifx$unique_sym$10", !11, i64 0}
!37 = !{i64 67}
!38 = !{i64 68}
!39 = !{i64 66}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$13", !11, i64 0}
!42 = !{i64 71}
!43 = !{i64 70}
!44 = !{!45, !45, i64 0}
!45 = !{!"ifx$unique_sym$14", !11, i64 0}
!46 = !{i64 74}
!47 = !{!48, !48, i64 0}
!48 = !{!"ifx$unique_sym$15", !11, i64 0}
!49 = !{i64 75}
!50 = !{i64 76}
!51 = !{i64 78}
!52 = !{i64 73}
!53 = !{!54, !54, i64 0}
!54 = !{!"ifx$unique_sym$16", !11, i64 0}
!55 = !{i64 82}
!56 = !{i64 83}
!57 = !{i64 84}
!58 = !{i64 86}
!59 = !{i64 81}
!60 = !{!61, !61, i64 0}
!61 = !{!"ifx$unique_sym$17", !11, i64 0}
!62 = !{i32 446}
!63 = !{i64 85}
!64 = !{i32 770}
!65 = !{i64 88}
