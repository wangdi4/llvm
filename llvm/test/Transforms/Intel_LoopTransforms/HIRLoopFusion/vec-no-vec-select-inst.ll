
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion" -disable-output -print-after=hir-loop-fusion < %s 2>&1 | FileCheck %s

; The first loop have no vectorization preventing edges. Second loop has a vectorization
; preventing flow edge due to select instruction. Test checks that the flow edges caused by
;  select instructions would be skipped (because the select is a part of vectorizer idiom)
; and loops would be fused.

;<0>          BEGIN REGION { }
;<40>               + DO i1 = 0, sext.i32.i64(%arg) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3000001>  <LEGAL_MAX_TC = 2147483647>
;<6>                |   %i25 = @llvm.fabs.f32((@"cortesa_$VEL")[0][i1]);
;<9>                |   %i28 = %i25  +  (@"cortesa_$SOUND")[0][i1];
;<10>               |   %i29 = (@"cortesa_$DX")[0][i1]  /  %i28;
;<12>               |   (%i13)[i1] = %i29;
;<40>               + END LOOP
;<40>
;<21>               (%i3)[0] = 1;
;<22>               %i36 = 0x7FF0000000000000;
;<24>               %i38 = 1;
;<41>
;<41>               + DO i1 = 0, sext.i32.i64(%arg) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<29>               |   %i40 = (%i13)[i1];
;<32>               |   %i38 = (%i40 < %i36) ? i1 + 1 : %i38;
;<33>               |   %i36 = (%i40 < %i36) ? %i40 : %i36;
;<41>               + END LOOP
;<0>          END REGION

; CHECK: BEGIN REGION { modified }
;           %i36 = 0x7FF0000000000000;
;           %i38 = 1;
;
; CHECK:    + DO i1 = 0, sext.i32.i64(%arg) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 3000001>  <LEGAL_MAX_TC = 2147483647>
;           |   %i25 = @llvm.fabs.f32((@"cortesa_$VEL")[0][i1]);
;           |   %i28 = %i25  +  (@"cortesa_$SOUND")[0][i1];
;           |   %i29 = (@"cortesa_$DX")[0][i1]  /  %i28;
;           |   (%i13)[i1] = %i29;
;           |   %i40 = (%i13)[i1];
;           |   %i38 = (%i40 < %i36) ? i1 + 1 : %i38;
;           |   %i36 = (%i40 < %i36) ? %i40 : %i36;
; CHECK:    + END LOOP
; CHECK-NOT: DO i1
;
;           (%i3)[0] = 1;
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"chozdt_$ISET" = external hidden unnamed_addr global [1 x i32], align 16, !llfort.type_idx !0
@"chozdt_$format_pack" = external hidden unnamed_addr global [100 x i8], align 4
@"cortesa_$DX" = external hidden unnamed_addr global [3000001 x float], align 16, !llfort.type_idx !1
@"cortesa_$SOUND" = external hidden global [3000001 x float], align 16, !llfort.type_idx !1
@"cortesa_$VEL" = external hidden global [3000001 x float], align 16, !llfort.type_idx !1

; Function Attrs: nofree nounwind uwtable
define hidden fastcc void @foo(i32 %arg, ptr noalias nocapture writeonly dereferenceable(4) %arg1, float %arg2) unnamed_addr #0 !llfort.type_idx !5 {
bb:
  %i = alloca [8 x i64], align 16, !llfort.type_idx !6
  %i3 = alloca i32, align 16
  %i4 = alloca [4 x i8], align 1, !llfort.type_idx !7
  %i5 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %i6 = alloca [4 x i8], align 1, !llfort.type_idx !9
  %i7 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %i8 = alloca [4 x i8], align 1, !llfort.type_idx !10
  %i9 = alloca <{ i64 }>, align 8, !llfort.type_idx !8
  %i10 = sext i32 %arg to i64, !llfort.type_idx !11
  %i11 = icmp sgt i64 %i10, 0
  %i12 = select i1 %i11, i64 %i10, i64 0
  %i13 = alloca float, i64 %i12, align 4, !llfort.type_idx !12
  %i14 = icmp slt i32 %arg, 1
  br i1 %i14, label %bb15, label %bb17

bb15:                                             ; preds = %bb
  %i16 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i3, i64 1), !llfort.type_idx !13
  store i32 1, ptr %i16, align 4, !tbaa !14
  br label %bb49

bb17:                                             ; preds = %bb
  %i18 = add nsw i64 %i10, 1
  br label %bb19

bb19:                                             ; preds = %bb19, %bb17
  %i20 = phi i64 [ %i31, %bb19 ], [ 1, %bb17 ]
  %i21 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) @"cortesa_$DX", i64 %i20), !llfort.type_idx !18
  %i22 = load float, ptr %i21, align 1, !tbaa !19, !llfort.type_idx !18
  %i23 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) @"cortesa_$VEL", i64 %i20), !llfort.type_idx !21
  %i24 = load float, ptr %i23, align 1, !tbaa !22, !llfort.type_idx !21
  %i25 = tail call fast float @llvm.fabs.f32(float %i24), !llfort.type_idx !24
  %i26 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) @"cortesa_$SOUND", i64 %i20), !llfort.type_idx !25
  %i27 = load float, ptr %i26, align 1, !tbaa !26, !llfort.type_idx !25
  %i28 = fadd fast float %i25, %i27
  %i29 = fdiv fast float %i22, %i28
  %i30 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i13, i64 %i20), !llfort.type_idx !28
  store float %i29, ptr %i30, align 4, !tbaa !29
  %i31 = add nuw nsw i64 %i20, 1
  %i32 = icmp eq i64 %i31, %i18
  br i1 %i32, label %bb33, label %bb19

bb33:                                             ; preds = %bb19
  %i34 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %i3, i64 1), !llfort.type_idx !13
  store i32 1, ptr %i34, align 4, !tbaa !14
  br label %bb35

bb35:                                             ; preds = %bb35, %bb33
  %i36 = phi float [ %i44, %bb35 ], [ 0x7FF0000000000000, %bb33 ]
  %i37 = phi i64 [ %i45, %bb35 ], [ 1, %bb33 ]
  %i38 = phi i32 [ %i43, %bb35 ], [ 1, %bb33 ]
  %i39 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %i13, i64 %i37), !llfort.type_idx !31
  %i40 = load float, ptr %i39, align 4, !tbaa !29, !llfort.type_idx !31
  %i41 = fcmp fast olt float %i40, %i36
  %i42 = trunc i64 %i37 to i32
  %i43 = select i1 %i41, i32 %i42, i32 %i38
  %i44 = select i1 %i41, float %i40, float %i36
  %i45 = add nuw nsw i64 %i37, 1
  %i46 = icmp eq i64 %i45, %i18
  br i1 %i46, label %bb47, label %bb35

bb47:                                             ; preds = %bb35
  %i48 = phi i32 [ %i43, %bb35 ]
  br label %bb49

bb49:                                             ; preds = %bb47, %bb15
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare !llfort.type_idx !55 !llfort.intrin_id !56 float @llvm.fabs.f32(float) #2

; Function Attrs: nofree
declare !llfort.type_idx !57 !llfort.intrin_id !58 dso_local i32 @for_write_seq_fmt(ptr, i32, i64, ptr, ptr, ptr, ...) local_unnamed_addr #3

; Function Attrs: nofree
declare !llfort.type_idx !59 !llfort.intrin_id !60 dso_local i32 @for_write_seq_fmt_xmit(ptr nocapture readonly, ptr nocapture readonly, ptr) local_unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!2, !3, !4}

!0 = !{i64 117}
!1 = !{i64 2491}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"LTOPostLink", i32 1}
!5 = !{i64 140}
!6 = !{i64 121}
!7 = !{i64 188}
!8 = !{i64 192}
!9 = !{i64 201}
!10 = !{i64 211}
!11 = !{i64 3}
!12 = !{i64 153}
!13 = !{i64 167}
!14 = !{!15, !15, i64 0}
!15 = !{!"Fortran Data Symbol", !16, i64 0}
!16 = !{!"Generic Fortran Symbol", !17, i64 0}
!17 = !{!"ifx$root$2$chozdt_"}
!18 = !{i64 157}
!19 = !{!20, !20, i64 0}
!20 = !{!"ifx$unique_sym$13", !15, i64 0}
!21 = !{i64 158}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$14", !15, i64 0}
!24 = !{i64 5}
!25 = !{i64 160}
!26 = !{!27, !27, i64 0}
!27 = !{!"ifx$unique_sym$15", !15, i64 0}
!28 = !{i64 156}
!29 = !{!30, !30, i64 0}
!30 = !{!"ifx$unique_sym$16", !15, i64 0}
!31 = !{i64 164}
!32 = !{i64 162}
!33 = !{i64 1}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$17", !15, i64 0}
!36 = !{i64 175}
!37 = !{i64 179}
!38 = !{i64 183}
!39 = !{i64 194}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$21", !15, i64 0}
!42 = !{i64 195}
!43 = !{i64 199}
!44 = !{i64 2}
!45 = !{i64 204}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$22", !15, i64 0}
!48 = !{i64 209}
!49 = !{i64 213}
!50 = !{!51, !51, i64 0}
!51 = !{!"ifx$unique_sym$23", !15, i64 0}
!52 = !{i64 218}
!53 = !{!54, !54, i64 0}
!54 = !{!"ifx$unique_sym$18", !15, i64 0}
!55 = !{i64 4080}
!56 = !{i32 336}
!57 = !{i64 189}
!58 = !{i32 331}
!59 = !{i64 202}
!60 = !{i32 332}
