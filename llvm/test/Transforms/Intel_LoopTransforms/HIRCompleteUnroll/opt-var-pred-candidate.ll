; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s --check-prefix=CHECK-DEBUG

; Test checks that pre vec complete unroll treats i3-loop mem refs as unconditionally
; executed in case of embracing 'if' statement is a candidate for
; HIR Opt Var Predicate optimization. Both i3 loops should be unrolled.

; HIR before transformation:
;<0>          BEGIN REGION { }
;<91>               + DO i1 = 0, zext.i32.i64(%i114) + -1 * zext.i32.i64(%i113), 1   <DO_LOOP> <ivdep>
;<5>                |   %i23 = (@md_globals_mp_x_)[0].0;
;<7>                |   %i25 = (@md_globals_mp_zii_)[0].0;
;<8>                |   %i26 = (@md_globals_mp_zii_)[0].6[0].2;
;<92>               |
;<92>               |   + DO i2 = 0, zext.i32.i64(%i118) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;<14>               |   |   if (i1 + %i113 != i2)
;<14>               |   |   {
;<20>               |   |      %i35 = 0.000000e+00;
;<93>               |   |
;<93>               |   |      + DO i3 = 0, 2, 1   <DO_LOOP>
;<27>               |   |      |   %i40 = (%i23)[i1 + zext.i32.i64(%i113)][i3]  -  (%i23)[i2][i3];
;<30>               |   |      |   %i43 = %i40  -  %i133;
;<31>               |   |      |   %i44 = (%i40 > %i7) ? %i43 : %i40;
;<33>               |   |      |   %i46 = %i133  +  %i44;
;<34>               |   |      |   %i47 = (%i44 < %i105) ? %i46 : %i44;
;<35>               |   |      |   (%i8)[0][i3] = %i47;
;<36>               |   |      |   %i48 = %i47  *  %i47;
;<37>               |   |      |   %i35 = %i48  +  %i35;
;<93>               |   |      + END LOOP
;<93>               |   |
;<45>               |   |      %i54 = @llvm.sqrt.f64(%i35);
;<48>               |   |      %i57 =  - %i54;
;<49>               |   |      %i58 = %i132  *  %i57;
;<50>               |   |      %i59 = @llvm.exp.f64(%i58);
;<51>               |   |      %i60 = 1.000000e+00  /  %i54;
;<52>               |   |      %i61 = %i132  +  %i60;
;<53>               |   |      %i62 = %i61  *  (%i25)[i2];
;<54>               |   |      %i63 = %i62  *  %i59;
;<55>               |   |      %i64 = 1.000000e+00  /  %i35;
;<94>               |   |
;<94>               |   |      + DO i3 = 0, 2, 1   <DO_LOOP>
;<63>               |   |      |   %i71 = %i63  *  (%i8)[0][i3];
;<64>               |   |      |   %i72 = %i71  *  %i64;
;<65>               |   |      |   %i73 = %i72  +  (%i117)[i1 + zext.i32.i64(%i113)][i3];
;<66>               |   |      |   (%i117)[i1 + zext.i32.i64(%i113)][i3] = %i73;
;<94>               |   |      + END LOOP
;<14>               |   |   }
;<92>               |   + END LOOP
;<91>               + END LOOP
;<0>          END REGION

; CHECK: DO i1
; CHECK: DO i2
; CHECK-NOT: DO i3
; CHECK: END LOOP
; CHECK: END LOOP

; Verify that we compute 5 fp operations for first i3 loop and 3 for second i3 loop.
; CHECK-DEBUG: Number of FP operations which cannot be eliminated: 5

; CHECK-DEBUG: Number of FP operations which cannot be eliminated: 3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$ptr$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$ptr$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%struct.ident_t = type { i32, i32, i32, i32, i8* }

@md_globals_mp_vc_ = external hidden unnamed_addr global double, align 8, !llfort.type_idx !0
@md_globals_mp_frp_ = external hidden global double, align 8, !llfort.type_idx !1
@md_globals_mp_xmuc_ = external hidden unnamed_addr global double, align 8, !llfort.type_idx !2
@md_globals_mp_zii_ = external hidden global %"QNCA_a0$ptr$rank1$", !llfort.type_idx !3
@md_globals_mp_xl_ = external hidden unnamed_addr global double, align 8, !llfort.type_idx !4
@md_globals_mp_x_ = external hidden global %"QNCA_a0$ptr$rank2$", !llfort.type_idx !5
@md_globals_mp_n_ = external hidden global i32, align 8, !llfort.type_idx !6
@md_globals_mp_a_ = external hidden global %"QNCA_a0$ptr$rank2$", !llfort.type_idx !5
@md_globals_mp_xmass_ = external hidden global double, align 8, !llfort.type_idx !7
@md_globals_mp_aii_ = external hidden global %"QNCA_a0$ptr$rank1$", !llfort.type_idx !3
@.kmpc_loc.0.0.18.108 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.20.109 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, i64*, i32) #0

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare !llfort.type_idx !12 !llfort.intrin_id !13 double @llvm.sqrt.f64(double) #1

; Function Attrs: nofree nounwind
declare void @__kmpc_dispatch_init_4(ptr, i32, i32, i32, i32, i32, i32) local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare i32 @__kmpc_dispatch_next_4(ptr, i32, ptr, ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare !llfort.type_idx !14 !llfort.intrin_id !15 double @llvm.exp.f64(double) #1

; Function Attrs: nofree nounwind uwtable
define hidden void @foo(ptr nocapture readonly %arg, ptr nocapture readnone %arg1, i64 %arg2, i64 %arg3, i64 %arg4, i64 %arg5, i64 %arg6) #3 {
bb:
  %i = trunc i64 %arg6 to i32
  %i7 = bitcast i64 %arg2 to double
  %i8 = alloca [3 x double], align 64
  %i9 = alloca i32, align 4
  %i10 = alloca i32, align 4
  %i11 = alloca i32, align 4
  %i12 = alloca i32, align 4
  store i32 0, ptr %i9, align 4
  %i13 = icmp slt i32 %i, 0
  br i1 %i13, label %bb167, label %bb100

bb14:                                             ; preds = %bb127, %bb97
  %i15 = phi i64 [ %i128, %bb127 ], [ %i98, %bb97 ]
  %i16 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 24, ptr elementtype(double) %i117, i64 %i15)
  br label %bb22

bb22:                                             ; preds = %bb17
  %i23 = load ptr, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank2$", ptr @md_globals_mp_x_, i64 0, i32 0), align 16
  %i24 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 24, ptr elementtype(double) %i23, i64 %i15)
  %i25 = load ptr, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @md_globals_mp_zii_, i64 0, i32 0), align 16
  %i26 = load i64, i64* %i102, align 1
  %i27 = and i64 %i15, 4294967295
  br label %bb28

bb28:                                             ; preds = %bb77, %bb22
  %i29 = phi i64 [ 0, %bb22 ], [ %i78, %bb77 ]
  %i30 = icmp eq i64 %i27, %i29
  br i1 %i30, label %bb77, label %bb31

bb31:                                             ; preds = %bb28
  %i32 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 24, ptr elementtype(double) %i23, i64 %i29), !llfort.type_idx !112
  br label %bb33

bb33:                                             ; preds = %bb33, %bb31
  %i34 = phi i64 [ 1, %bb31 ], [ %i50, %bb33 ]
  %i35 = phi double [ 0.000000e+00, %bb31 ], [ %i49, %bb33 ]
  %i36 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i24, i64 %i34), !llfort.type_idx !113
  %i37 = load double, ptr %i36, align 1, !tbaa !114, !alias.scope !117, !noalias !118, !llvm.access.group !111, !llfort.type_idx !113
  %i38 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i32, i64 %i34), !llfort.type_idx !119
  %i39 = load double, ptr %i38, align 1, !tbaa !114, !alias.scope !120, !noalias !118, !llvm.access.group !111, !llfort.type_idx !119
  %i40 = fsub fast double %i37, %i39
  %i41 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i104, i64 %i34), !llfort.type_idx !121, !ifx.array_extent !122
  %i42 = fcmp fast ogt double %i40, %i7
  %i43 = fsub fast double %i40, %i133
  %i44 = select i1 %i42, double %i43, double %i40
  %i45 = fcmp fast olt double %i44, %i105
  %i46 = fadd fast double %i133, %i44
  %i47 = select i1 %i45, double %i46, double %i44
  store double %i47, ptr %i41, align 8, !tbaa !123, !alias.scope !125, !noalias !126
  %i48 = fmul fast double %i47, %i47
  %i49 = fadd fast double %i48, %i35
  %i50 = add nuw nsw i64 %i34, 1
  %i51 = icmp eq i64 %i50, 4
  br i1 %i51, label %bb52, label %bb33

bb52:                                             ; preds = %bb33
  %i53 = phi double [ %i49, %bb33 ]
  %i54 = call fast double @llvm.sqrt.f64(double %i53), !llfort.type_idx !127
  %i55 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %i26, i64 8, ptr elementtype(double) %i25, i64 %i29), !llfort.type_idx !128
  %i56 = load double, ptr %i55, align 1, !tbaa !129, !alias.scope !131, !noalias !118, !llvm.access.group !111, !llfort.type_idx !128
  %i57 = fneg fast double %i54
  %i58 = fmul fast double %i132, %i57
  %i59 = call fast double @llvm.exp.f64(double %i58), !llfort.type_idx !127
  %i60 = fdiv fast double 1.000000e+00, %i54
  %i61 = fadd fast double %i132, %i60
  %i62 = fmul fast double %i61, %i56
  %i63 = fmul fast double %i62, %i59
  %i64 = fdiv fast double 1.000000e+00, %i53
  br label %bb65

bb65:                                             ; preds = %bb65, %bb52
  %i66 = phi i64 [ 1, %bb52 ], [ %i74, %bb65 ]
  %i67 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %i16, i64 %i66)
  %i68 = load double, ptr %i67, align 1, !tbaa !17, !alias.scope !132, !noalias !133, !llvm.access.group !111, !llfort.type_idx !134
  %i69 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i104, i64 %i66), !llfort.type_idx !135, !ifx.array_extent !122
  %i70 = load double, ptr %i69, align 8, !tbaa !123, !alias.scope !136, !noalias !137, !llvm.access.group !111, !llfort.type_idx !138
  %i71 = fmul fast double %i63, %i70
  %i72 = fmul fast double %i71, %i64
  %i73 = fadd fast double %i72, %i68
  store double %i73, ptr %i67, align 1, !tbaa !17, !alias.scope !139, !noalias !27, !llvm.access.group !111
  %i74 = add nuw nsw i64 %i66, 1
  %i75 = icmp eq i64 %i74, 4
  br i1 %i75, label %bb76, label %bb65

bb76:                                             ; preds = %bb65
  br label %bb77

bb77:                                             ; preds = %bb76, %bb28
  %i78 = add nuw nsw i64 %i29, 1
  %i79 = icmp eq i64 %i78, %i131
  br i1 %i79, label %bb80, label %bb28

bb80:                                             ; preds = %bb77
  br label %bb97

bb97:                                             ; preds = %bb80
  %i98 = add nuw nsw i64 %i15, 1
  %i99 = icmp eq i64 %i98, %i130
  br i1 %i99, label %bb107, label %bb14, !llvm.loop !149

bb100:                                            ; preds = %bb
  %i101 = load i32, ptr %arg, align 4
  store i32 0, ptr %i10, align 4
  store i32 %i, ptr %i11, align 4
  store i32 1, ptr %i12, align 4
  tail call void @__kmpc_dispatch_init_4(ptr nonnull @.kmpc_loc.0.0.18.108, i32 %i101, i32 37, i32 0, i32 %i, i32 1, i32 0)
  %i102 = tail call i64* @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @md_globals_mp_zii_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i103 = tail call i64* @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @md_globals_mp_aii_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %i104 = getelementptr inbounds [3 x double], [3 x double]* %i8, i64 0, i64 0
  %i105 = fneg fast double %i7
  br label %bb109

bb106:                                            ; preds = %bb163
  br label %bb108

bb107:                                            ; preds = %bb97
  br label %bb108

bb108:                                            ; preds = %bb107, %bb106
  br label %bb109, !llvm.loop !149

bb109:                                            ; preds = %bb108, %bb100
  %i110 = call i32 @__kmpc_dispatch_next_4(ptr nonnull @.kmpc_loc.0.0.20.109, i32 %i101, ptr nonnull %i9, ptr nonnull %i10, ptr nonnull %i11, ptr nonnull %i12)
  %i111 = icmp eq i32 %i110, 0
  br i1 %i111, label %bb167, label %bb112

bb112:                                            ; preds = %bb109
  %i113 = load i32, ptr %i10, align 4, !range !156
  %i114 = load i32, ptr %i11, align 4, !range !156
  %i115 = icmp ugt i32 %i113, %i114
  br i1 %i115, label %bb167, label %bb116

bb116:                                            ; preds = %bb112
  %i117 = load ptr, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank2$", ptr @md_globals_mp_a_, i64 0, i32 0), align 16, !tbaa !157, !alias.scope !161, !noalias !118, !llvm.access.group !111, !llfort.type_idx !127
  %i118 = load i32, ptr @md_globals_mp_n_, align 8, !tbaa !162, !alias.scope !164, !noalias !118, !llvm.access.group !111, !llfort.type_idx !165
  %i119 = icmp slt i32 %i118, 1
  %i120 = load double, ptr @md_globals_mp_frp_, align 8, !tbaa !166, !alias.scope !168, !noalias !118, !llvm.access.group !111, !llfort.type_idx !169
  %i121 = load ptr, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @md_globals_mp_zii_, i64 0, i32 0), align 16, !tbaa !170, !alias.scope !172, !noalias !118, !llvm.access.group !111, !llfort.type_idx !127
  %i122 = load i64, i64* %i102, align 1, !tbaa !173, !alias.scope !174, !noalias !118, !llvm.access.group !111, !llfort.type_idx !175
  %i123 = load double, ptr @md_globals_mp_vc_, align 8, !tbaa !176, !alias.scope !178, !noalias !118, !llvm.access.group !111, !llfort.type_idx !179
  %i124 = load ptr, ptr getelementptr inbounds (%"QNCA_a0$ptr$rank1$", ptr @md_globals_mp_aii_, i64 0, i32 0), align 16, !tbaa !180, !alias.scope !182, !noalias !118, !llvm.access.group !111, !llfort.type_idx !127
  %i125 = load i64, i64* %i103, align 1, !tbaa !183, !alias.scope !184, !noalias !118, !llvm.access.group !111, !llfort.type_idx !185
  %i126 = load double, ptr @md_globals_mp_xmass_, align 8, !tbaa !186, !alias.scope !188, !noalias !118, !llvm.access.group !111, !llfort.type_idx !189
  br i1 %i119, label %bb167, label %bb127

bb127:                                            ; preds = %bb116
  %i128 = zext i32 %i113 to i64
  %i129 = add nuw nsw i32 %i114, 1
  %i130 = zext i32 %i129 to i64
  %i131 = zext i32 %i118 to i64
  %i132 = load double, ptr @md_globals_mp_xmuc_, align 8
  %i133 = load double, ptr @md_globals_mp_xl_, align 8
  br label %bb14

bb167:                                            ; preds = %bb112, %bb109, %bb
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree nounwind }
attributes #3 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "processed-by-vpo" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!omp_offload.info = !{}
!nvvm.annotations = !{}
!llvm.module.flags = !{!8, !9, !10, !11}

!0 = !{i64 144}
!1 = !{i64 143}
!2 = !{i64 145}
!3 = !{i64 72}
!4 = !{i64 170}
!5 = !{i64 56}
!6 = !{i64 172}
!7 = !{i64 168}
!8 = !{i32 1, !"ThinLTO", i32 0}
!9 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!10 = !{i32 7, !"openmp", i32 50}
!11 = !{i32 1, !"LTOPostLink", i32 1}
!12 = !{i64 2839}
!13 = !{i32 582}
!14 = !{i64 727}
!15 = !{i32 544}
!16 = !{i64 349}
!17 = !{!18, !18, i64 0}
!18 = !{!"ifx$unique_sym$24", !19, i64 0}
!19 = !{!"Fortran Data Symbol", !20, i64 0}
!20 = !{!"Generic Fortran Symbol", !21, i64 0}
!21 = !{!"ifx$root$2$"}
!22 = !{!23, !25, !26}
!23 = distinct !{!23, !24, !"OMPAliasScope"}
!24 = distinct !{!24, !"OMPDomain"}
!25 = distinct !{!25, !24, !"OMPAliasScope"}
!26 = distinct !{!26, !24, !"OMPAliasScope"}
!27 = !{!28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110}
!28 = distinct !{!28, !24, !"OMPAliasScope"}
!29 = distinct !{!29, !24, !"OMPAliasScope"}
!30 = distinct !{!30, !24, !"OMPAliasScope"}
!31 = distinct !{!31, !24, !"OMPAliasScope"}
!32 = distinct !{!32, !24, !"OMPAliasScope"}
!33 = distinct !{!33, !24, !"OMPAliasScope"}
!34 = distinct !{!34, !24, !"OMPAliasScope"}
!35 = distinct !{!35, !24, !"OMPAliasScope"}
!36 = distinct !{!36, !24, !"OMPAliasScope"}
!37 = distinct !{!37, !24, !"OMPAliasScope"}
!38 = distinct !{!38, !24, !"OMPAliasScope"}
!39 = distinct !{!39, !24, !"OMPAliasScope"}
!40 = distinct !{!40, !24, !"OMPAliasScope"}
!41 = distinct !{!41, !24, !"OMPAliasScope"}
!42 = distinct !{!42, !24, !"OMPAliasScope"}
!43 = distinct !{!43, !24, !"OMPAliasScope"}
!44 = distinct !{!44, !24, !"OMPAliasScope"}
!45 = distinct !{!45, !24, !"OMPAliasScope"}
!46 = distinct !{!46, !24, !"OMPAliasScope"}
!47 = distinct !{!47, !24, !"OMPAliasScope"}
!48 = distinct !{!48, !24, !"OMPAliasScope"}
!49 = distinct !{!49, !24, !"OMPAliasScope"}
!50 = distinct !{!50, !24, !"OMPAliasScope"}
!51 = distinct !{!51, !24, !"OMPAliasScope"}
!52 = distinct !{!52, !24, !"OMPAliasScope"}
!53 = distinct !{!53, !24, !"OMPAliasScope"}
!54 = distinct !{!54, !24, !"OMPAliasScope"}
!55 = distinct !{!55, !24, !"OMPAliasScope"}
!56 = distinct !{!56, !24, !"OMPAliasScope"}
!57 = distinct !{!57, !24, !"OMPAliasScope"}
!58 = distinct !{!58, !24, !"OMPAliasScope"}
!59 = distinct !{!59, !24, !"OMPAliasScope"}
!60 = distinct !{!60, !24, !"OMPAliasScope"}
!61 = distinct !{!61, !24, !"OMPAliasScope"}
!62 = distinct !{!62, !24, !"OMPAliasScope"}
!63 = distinct !{!63, !24, !"OMPAliasScope"}
!64 = distinct !{!64, !24, !"OMPAliasScope"}
!65 = distinct !{!65, !24, !"OMPAliasScope"}
!66 = distinct !{!66, !24, !"OMPAliasScope"}
!67 = distinct !{!67, !24, !"OMPAliasScope"}
!68 = distinct !{!68, !24, !"OMPAliasScope"}
!69 = distinct !{!69, !24, !"OMPAliasScope"}
!70 = distinct !{!70, !24, !"OMPAliasScope"}
!71 = distinct !{!71, !24, !"OMPAliasScope"}
!72 = distinct !{!72, !24, !"OMPAliasScope"}
!73 = distinct !{!73, !24, !"OMPAliasScope"}
!74 = distinct !{!74, !24, !"OMPAliasScope"}
!75 = distinct !{!75, !24, !"OMPAliasScope"}
!76 = distinct !{!76, !24, !"OMPAliasScope"}
!77 = distinct !{!77, !24, !"OMPAliasScope"}
!78 = distinct !{!78, !24, !"OMPAliasScope"}
!79 = distinct !{!79, !24, !"OMPAliasScope"}
!80 = distinct !{!80, !24, !"OMPAliasScope"}
!81 = distinct !{!81, !24, !"OMPAliasScope"}
!82 = distinct !{!82, !24, !"OMPAliasScope"}
!83 = distinct !{!83, !24, !"OMPAliasScope"}
!84 = distinct !{!84, !24, !"OMPAliasScope"}
!85 = distinct !{!85, !24, !"OMPAliasScope"}
!86 = distinct !{!86, !24, !"OMPAliasScope"}
!87 = distinct !{!87, !24, !"OMPAliasScope"}
!88 = distinct !{!88, !24, !"OMPAliasScope"}
!89 = distinct !{!89, !24, !"OMPAliasScope"}
!90 = distinct !{!90, !24, !"OMPAliasScope"}
!91 = distinct !{!91, !24, !"OMPAliasScope"}
!92 = distinct !{!92, !24, !"OMPAliasScope"}
!93 = distinct !{!93, !24, !"OMPAliasScope"}
!94 = distinct !{!94, !24, !"OMPAliasScope"}
!95 = distinct !{!95, !24, !"OMPAliasScope"}
!96 = distinct !{!96, !24, !"OMPAliasScope"}
!97 = distinct !{!97, !24, !"OMPAliasScope"}
!98 = distinct !{!98, !24, !"OMPAliasScope"}
!99 = distinct !{!99, !24, !"OMPAliasScope"}
!100 = distinct !{!100, !24, !"OMPAliasScope"}
!101 = distinct !{!101, !24, !"OMPAliasScope"}
!102 = distinct !{!102, !24, !"OMPAliasScope"}
!103 = distinct !{!103, !24, !"OMPAliasScope"}
!104 = distinct !{!104, !24, !"OMPAliasScope"}
!105 = distinct !{!105, !24, !"OMPAliasScope"}
!106 = distinct !{!106, !24, !"OMPAliasScope"}
!107 = distinct !{!107, !24, !"OMPAliasScope"}
!108 = distinct !{!108, !24, !"OMPAliasScope"}
!109 = distinct !{!109, !24, !"OMPAliasScope"}
!110 = distinct !{!110, !24, !"OMPAliasScope"}
!111 = distinct !{}
!112 = !{i64 364}
!113 = !{i64 357}
!114 = !{!115, !115, i64 0}
!115 = !{!"ifx$unique_sym$28", !116, i64 0}
!116 = !{!"Fortran Target Data Symbol", !19, i64 0}
!117 = !{!87}
!118 = !{!30, !31, !33, !35, !37, !38, !39, !40, !46, !47, !48, !50, !51, !52, !53, !54, !55, !23, !25, !26, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !89, !90, !91, !92, !93, !96, !101, !106, !107, !108, !109, !110}
!119 = !{i64 365}
!120 = !{!88}
!121 = !{i64 367}
!122 = !{i64 3}
!123 = !{!124, !124, i64 0}
!124 = !{!"ifx$unique_sym$29", !19, i64 0}
!125 = !{!89, !90, !91, !92, !93}
!126 = !{!28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !23, !25, !26, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110}
!127 = !{i64 6}
!128 = !{i64 396}
!129 = !{!130, !130, i64 0}
!130 = !{!"ifx$unique_sym$31", !19, i64 0}
!131 = !{!99}
!132 = !{!23}
!133 = !{!30, !31, !33, !35, !37, !38, !39, !40, !46, !47, !48, !50, !51, !52, !53, !54, !55, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !89, !90, !91, !92, !93, !96, !101, !106, !107, !108, !109, !110}
!134 = !{i64 421}
!135 = !{i64 423}
!136 = !{!89}
!137 = !{!30, !31, !33, !35, !37, !38, !39, !40, !46, !47, !48, !50, !51, !52, !53, !54, !55, !23, !25, !26, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !96, !101, !106, !107, !108, !109, !110}
!138 = !{i64 424}
!139 = !{!23, !25}
!140 = !{i64 454}
!141 = !{!142, !142, i64 0}
!142 = !{!"ifx$unique_sym$36", !19, i64 0}
!143 = !{!68}
!144 = !{i64 439}
!145 = !{!64}
!146 = !{!25, !26}
!147 = !{i64 451}
!148 = !{!23, !26}
!149 = distinct !{!149, !150, !154, !155}
!150 = distinct !{!"intel.optreport", !152}
!152 = !{!"intel.optreport.remarks", !153}
!153 = !{!"intel.optreport.remark", i32 0, !"OpenMP: Outlined parallel loop"}
!154 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!155 = !{!"llvm.loop.parallel_accesses", !111}
!156 = !{i32 0, i32 2147483647}
!157 = !{!158, !159, i64 0}
!158 = !{!"ifx$descr$3", !159, i64 0, !159, i64 8, !159, i64 16, !159, i64 24, !159, i64 32, !159, i64 40, !159, i64 48, !159, i64 56, !159, i64 64, !159, i64 72, !159, i64 80, !159, i64 88}
!159 = !{!"ifx$descr$field", !160, i64 0}
!160 = !{!"Fortran Dope Vector Symbol", !20, i64 0}
!161 = !{!41}
!162 = !{!163, !163, i64 0}
!163 = !{!"ifx$unique_sym$21", !19, i64 0}
!164 = !{!49}
!165 = !{i64 308}
!166 = !{!167, !167, i64 0}
!167 = !{!"ifx$unique_sym$34", !19, i64 0}
!168 = !{!61}
!169 = !{i64 305}
!170 = !{!171, !159, i64 0}
!171 = !{!"ifx$descr$5", !159, i64 0, !159, i64 8, !159, i64 16, !159, i64 24, !159, i64 32, !159, i64 40, !159, i64 48, !159, i64 56, !159, i64 64}
!172 = !{!62}
!173 = !{!171, !159, i64 64}
!174 = !{!63}
!175 = !{i64 437}
!176 = !{!177, !177, i64 0}
!177 = !{!"ifx$unique_sym$35", !19, i64 0}
!178 = !{!65}
!179 = !{i64 304}
!180 = !{!181, !159, i64 0}
!181 = !{!"ifx$descr$6", !159, i64 0, !159, i64 8, !159, i64 16, !159, i64 24, !159, i64 32, !159, i64 40, !159, i64 48, !159, i64 56, !159, i64 64}
!182 = !{!66}
!183 = !{!181, !159, i64 64}
!184 = !{!67}
!185 = !{i64 452}
!186 = !{!187, !187, i64 0}
!187 = !{!"ifx$unique_sym$37", !19, i64 0}
!188 = !{!69}
!189 = !{i64 303}
