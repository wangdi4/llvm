; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>" < %s 2>&1 | FileCheck %s

; Test checks that all i3 innermost loops got unrolled due to alloca stores.

; Original HIR:
;     BEGIN REGION { }
;           + DO i1 = 0, zext.i32.i64(%96) + -1 * zext.i32.i64(%95), 1   <DO_LOOP> <ivdep>
;           |   + DO i2 = 0, 2, 1   <DO_LOOP>
;           |   |   (%99)[i1 + zext.i32.i64(%95)][i2] = 0.000000e+00;
;           |   + END LOOP
;           |
;           |   %23 = (@md_globals_mp_x_)[0].0;
;           |   %25 = (@md_globals_mp_zii_)[0].0;
;           |   %26 = (@md_globals_mp_zii_)[0].6[0].2;
;           |
;           |   + DO i2 = 0, zext.i32.i64(%100) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;           |   |   if (i1 + %95 != i2)
;           |   |   {
;           |   |      %35 = 0.000000e+00;
;           |   |
;           |   |      + DO i3 = 0, 2, 1   <DO_LOOP>
;           |   |      |   %40 = (%23)[i1 + zext.i32.i64(%95)][i3]  -  (%23)[i2][i3];
;           |   |      |   (%7)[0][i3] = %40;
;           |   |      |   %42 = %40  *  %40;
;           |   |      |   %35 = %42  +  %35;
;           |   |      + END LOOP
;           |   |
;           |   |      %50 = @llvm.exp.f64(%35);
;           |   |      %51 = (%25)[i2]  *  %50;
;           |   |
;           |   |      + DO i3 = 0, 2, 1   <DO_LOOP>
;           |   |      |   %58 = %51  *  (%7)[0][i3];
;           |   |      |   %59 = %58  +  (%99)[i1 + zext.i32.i64(%95)][i3];
;           |   |      |   (%99)[i1 + zext.i32.i64(%95)][i3] = %59;
;           |   |      + END LOOP
;           |   |   }
;           |   + END LOOP
;           |
;           |   %68 = (%102)[i1 + zext.i32.i64(%95)];
;           |   %71 = 1.000000e+00  /  (%104)[i1 + zext.i32.i64(%95)];
;           |
;           |   + DO i2 = 0, 2, 1   <DO_LOOP>
;           |   |   %76 = (%99)[i1 + zext.i32.i64(%95)][i2]  *  %68;
;           |   |   %77 = %76  *  %71;
;           |   |   (%99)[i1 + zext.i32.i64(%95)][i2] = %77;
;           |   + END LOOP
;           + END LOOP
;     END REGION

; Check that both i3 innermost loops are unrolled.

; CHECK: DO i1
; CHECK: DO i2
; CHECK: DO i2
; CHECK: if (i1 + 
; CHECK-NOT: DO i3
; CHECK: DO i2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank1$" = type { double*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }
%struct.ident_t = type { i32, i32, i32, i32, i8* }

@md_globals_mp_zii_ = external hidden global %"QNCA_a0$double*$rank1$", !llfort.type_idx !0
@md_globals_mp_x_ = external hidden global %"QNCA_a0$double*$rank2$", !llfort.type_idx !1
@md_globals_mp_n_ = external hidden global i32, align 8, !llfort.type_idx !2
@md_globals_mp_a_ = external hidden global %"QNCA_a0$double*$rank2$", !llfort.type_idx !1
@md_globals_mp_aii_ = external hidden global %"QNCA_a0$double*$rank1$", !llfort.type_idx !0
@.kmpc_loc.0.0.18.108 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.20.109 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #0

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nofree nounwind
declare void @__kmpc_dispatch_init_4(%struct.ident_t*, i32, i32, i32, i32, i32, i32) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare i32 @__kmpc_dispatch_next_4(%struct.ident_t*, i32, i32*, i32*, i32*, i32*) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare !llfort.type_idx !7 !llfort.intrin_id !8 double @llvm.exp.f64(double) #2

; Function Attrs: nofree nounwind uwtable
define hidden void @foo_.DIR.OMP.PARALLEL.LOOP.2114.split120(i32* nocapture readonly %0, i32* nocapture readnone %1, i64 %2, i64 %3, i64 %4, i64 %5) #3 {
  %7 = alloca [3 x double], align 64
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  store i32 0, i32* %8, align 4
  %12 = trunc i64 %5 to i32
  %13 = icmp slt i32 %12, 0
  br i1 %13, label %b141, label %83

14:                                               ; preds = %106, %80
  %15 = phi i64 [ %107, %106 ], [ %81, %80 ]
  %16 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 24, double* elementtype(double) %99, i64 %15)
  br label %17

17:                                               ; preds = %17, %14
  %18 = phi i64 [ 1, %14 ], [ %20, %17 ]
  %19 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %16, i64 %18), !llfort.type_idx !9
  store double 0.000000e+00, double* %19, align 1, !tbaa !10, !alias.scope !15, !noalias !20, !llvm.access.group !88
  %20 = add nuw nsw i64 %18, 1
  %21 = icmp eq i64 %20, 4
  br i1 %21, label %22, label %17

22:                                               ; preds = %17
  %23 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* @md_globals_mp_x_, i64 0, i32 0), align 16
  %24 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 24, double* elementtype(double) %23, i64 %15)
  %25 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @md_globals_mp_zii_, i64 0, i32 0), align 16
  %26 = load i64, i64* %85, align 1
  %27 = and i64 %15, 4294967295
  br label %28

28:                                               ; preds = %63, %22
  %29 = phi i64 [ 0, %22 ], [ %64, %63 ]
  %30 = icmp eq i64 %27, %29
  br i1 %30, label %63, label %31

31:                                               ; preds = %28
  %32 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 24, double* elementtype(double) %23, i64 %29), !llfort.type_idx !89
  br label %33

33:                                               ; preds = %33, %31
  %34 = phi i64 [ 1, %31 ], [ %44, %33 ]
  %35 = phi double [ 0.000000e+00, %31 ], [ %43, %33 ]
  %36 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %24, i64 %34), !llfort.type_idx !90
  %37 = load double, double* %36, align 1, !tbaa !91, !alias.scope !94, !noalias !95, !llvm.access.group !88, !llfort.type_idx !90
  %38 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %32, i64 %34), !llfort.type_idx !96
  %39 = load double, double* %38, align 1, !tbaa !91, !alias.scope !97, !noalias !95, !llvm.access.group !88, !llfort.type_idx !96
  %40 = fsub fast double %37, %39
  %41 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %87, i64 %34), !llfort.type_idx !98
  store double %40, double* %41, align 8, !tbaa !99, !alias.scope !101, !noalias !102, !llvm.access.group !88
  %42 = fmul fast double %40, %40
  %43 = fadd fast double %42, %35
  %44 = add nuw nsw i64 %34, 1
  %45 = icmp eq i64 %44, 4
  br i1 %45, label %46, label %33

46:                                               ; preds = %33
  %47 = phi double [ %43, %33 ]
  %48 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %26, i64 8, double* elementtype(double) %25, i64 %29), !llfort.type_idx !103
  %49 = load double, double* %48, align 1, !tbaa !104, !alias.scope !106, !noalias !95, !llvm.access.group !88, !llfort.type_idx !103
  %50 = call fast double @llvm.exp.f64(double %47), !llfort.type_idx !107
  %51 = fmul fast double %49, %50
  br label %52

52:                                               ; preds = %52, %46
  %53 = phi i64 [ 1, %46 ], [ %60, %52 ]
  %54 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %16, i64 %53)
  %55 = load double, double* %54, align 1, !tbaa !10, !alias.scope !108, !noalias !109, !llvm.access.group !88, !llfort.type_idx !110
  %56 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %87, i64 %53), !llfort.type_idx !111
  %57 = load double, double* %56, align 8, !tbaa !99, !alias.scope !112, !noalias !113, !llvm.access.group !88, !llfort.type_idx !114
  %58 = fmul fast double %51, %57
  %59 = fadd fast double %58, %55
  store double %59, double* %54, align 1, !tbaa !10, !alias.scope !115, !noalias !20, !llvm.access.group !88
  %60 = add nuw nsw i64 %53, 1
  %61 = icmp eq i64 %60, 4
  br i1 %61, label %62, label %52

62:                                               ; preds = %52
  br label %63

63:                                               ; preds = %62, %28
  %64 = add nuw nsw i64 %29, 1
  %65 = icmp eq i64 %64, %110
  br i1 %65, label %66, label %28

66:                                               ; preds = %63
  %67 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %103, i64 8, double* elementtype(double) %102, i64 %15), !llfort.type_idx !116
  %68 = load double, double* %67, align 1, !tbaa !104, !alias.scope !117, !noalias !95, !llvm.access.group !88, !llfort.type_idx !116
  %69 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %105, i64 8, double* elementtype(double) %104, i64 %15), !llfort.type_idx !118
  %70 = load double, double* %69, align 1, !tbaa !119, !alias.scope !121, !noalias !95, !llvm.access.group !88, !llfort.type_idx !118
  %71 = fdiv fast double 1.000000e+00, %70
  br label %72

72:                                               ; preds = %72, %66
  %73 = phi i64 [ %78, %72 ], [ 1, %66 ]
  %74 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %16, i64 %73)
  %75 = load double, double* %74, align 1, !tbaa !10, !alias.scope !122, !noalias !109, !llvm.access.group !88, !llfort.type_idx !123
  %76 = fmul fast double %75, %68
  %77 = fmul fast double %76, %71
  store double %77, double* %74, align 1, !tbaa !10, !alias.scope !124, !noalias !20, !llvm.access.group !88
  %78 = add nuw nsw i64 %73, 1
  %79 = icmp eq i64 %78, 4
  br i1 %79, label %80, label %72

80:                                               ; preds = %72
  %81 = add nuw nsw i64 %15, 1
  %82 = icmp eq i64 %81, %109
  br i1 %82, label %89, label %14, !llvm.loop !125

83:                                               ; preds = %6
  %84 = load i32, i32* %0, align 4
  store i32 0, i32* %9, align 4
  store i32 %12, i32* %10, align 4
  store i32 1, i32* %11, align 4
  tail call void @__kmpc_dispatch_init_4(%struct.ident_t* nonnull @.kmpc_loc.0.0.18.108, i32 %84, i32 37, i32 0, i32 %12, i32 1, i32 0)
  %85 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @md_globals_mp_zii_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %86 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @md_globals_mp_aii_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %87 = getelementptr inbounds [3 x double], [3 x double]* %7, i64 0, i64 0
  br label %91

88:                                               ; preds = %128
  br label %90

89:                                               ; preds = %80
  br label %90

90:                                               ; preds = %89, %88
  br label %91, !llvm.loop !125

91:                                               ; preds = %90, %83
  %92 = call i32 @__kmpc_dispatch_next_4(%struct.ident_t* nonnull @.kmpc_loc.0.0.20.109, i32 %84, i32* nonnull %8, i32* nonnull %9, i32* nonnull %10, i32* nonnull %11)
  %93 = icmp eq i32 %92, 0
  br i1 %93, label %b140, label %94

94:                                               ; preds = %91
  %95 = load i32, i32* %9, align 4, !range !132
  %96 = load i32, i32* %10, align 4, !range !132
  %97 = icmp ugt i32 %95, %96
  br i1 %97, label %b140, label %98

98:                                               ; preds = %94
  %99 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* @md_globals_mp_a_, i64 0, i32 0), align 16, !tbaa !133, !alias.scope !137, !noalias !95, !llvm.access.group !88, !llfort.type_idx !107
  %100 = load i32, i32* @md_globals_mp_n_, align 8, !tbaa !138, !alias.scope !140, !noalias !95, !llvm.access.group !88, !llfort.type_idx !141
  %101 = icmp slt i32 %100, 1
  %102 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @md_globals_mp_zii_, i64 0, i32 0), align 16, !tbaa !142, !alias.scope !144, !noalias !95, !llvm.access.group !88, !llfort.type_idx !107
  %103 = load i64, i64* %85, align 1, !tbaa !145, !alias.scope !146, !noalias !95, !llvm.access.group !88, !llfort.type_idx !147
  %104 = load double*, double** getelementptr inbounds (%"QNCA_a0$double*$rank1$", %"QNCA_a0$double*$rank1$"* @md_globals_mp_aii_, i64 0, i32 0), align 16, !tbaa !148, !alias.scope !150, !noalias !95, !llvm.access.group !88, !llfort.type_idx !107
  %105 = load i64, i64* %86, align 1, !tbaa !151, !alias.scope !152, !noalias !95, !llvm.access.group !88, !llfort.type_idx !153
  br i1 %101, label %111, label %106

106:                                              ; preds = %98
  %107 = zext i32 %95 to i64
  %108 = add nuw nsw i32 %96, 1
  %109 = zext i32 %108 to i64
  %110 = zext i32 %100 to i64
  br label %14

111:                                              ; preds = %98
  %112 = zext i32 %95 to i64
  %113 = add nuw nsw i32 %96, 1
  %114 = zext i32 %113 to i64
  br label %b140

b140:                                              ; preds = %94, %91
  br label %b141

b141:                                              ; preds = %b140, %6
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "processed-by-vpo" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!omp_offload.info = !{}
!nvvm.annotations = !{}
!llvm.module.flags = !{!3, !4, !5, !6}

!0 = !{i64 72}
!1 = !{i64 56}
!2 = !{i64 168}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 1, !"LTOPostLink", i32 1}
!7 = !{i64 722}
!8 = !{i32 544}
!9 = !{i64 294}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$23", !12, i64 0}
!12 = !{!"Fortran Data Symbol", !13, i64 0}
!13 = !{!"Generic Fortran Symbol", !14, i64 0}
!14 = !{!"ifx$root$2$"}
!15 = !{!16, !18, !19}
!16 = distinct !{!16, !17, !"OMPAliasScope"}
!17 = distinct !{!17, !"OMPDomain"}
!18 = distinct !{!18, !17, !"OMPAliasScope"}
!19 = distinct !{!19, !17, !"OMPAliasScope"}
!20 = !{!21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87}
!21 = distinct !{!21, !17, !"OMPAliasScope"}
!22 = distinct !{!22, !17, !"OMPAliasScope"}
!23 = distinct !{!23, !17, !"OMPAliasScope"}
!24 = distinct !{!24, !17, !"OMPAliasScope"}
!25 = distinct !{!25, !17, !"OMPAliasScope"}
!26 = distinct !{!26, !17, !"OMPAliasScope"}
!27 = distinct !{!27, !17, !"OMPAliasScope"}
!28 = distinct !{!28, !17, !"OMPAliasScope"}
!29 = distinct !{!29, !17, !"OMPAliasScope"}
!30 = distinct !{!30, !17, !"OMPAliasScope"}
!31 = distinct !{!31, !17, !"OMPAliasScope"}
!32 = distinct !{!32, !17, !"OMPAliasScope"}
!33 = distinct !{!33, !17, !"OMPAliasScope"}
!34 = distinct !{!34, !17, !"OMPAliasScope"}
!35 = distinct !{!35, !17, !"OMPAliasScope"}
!36 = distinct !{!36, !17, !"OMPAliasScope"}
!37 = distinct !{!37, !17, !"OMPAliasScope"}
!38 = distinct !{!38, !17, !"OMPAliasScope"}
!39 = distinct !{!39, !17, !"OMPAliasScope"}
!40 = distinct !{!40, !17, !"OMPAliasScope"}
!41 = distinct !{!41, !17, !"OMPAliasScope"}
!42 = distinct !{!42, !17, !"OMPAliasScope"}
!43 = distinct !{!43, !17, !"OMPAliasScope"}
!44 = distinct !{!44, !17, !"OMPAliasScope"}
!45 = distinct !{!45, !17, !"OMPAliasScope"}
!46 = distinct !{!46, !17, !"OMPAliasScope"}
!47 = distinct !{!47, !17, !"OMPAliasScope"}
!48 = distinct !{!48, !17, !"OMPAliasScope"}
!49 = distinct !{!49, !17, !"OMPAliasScope"}
!50 = distinct !{!50, !17, !"OMPAliasScope"}
!51 = distinct !{!51, !17, !"OMPAliasScope"}
!52 = distinct !{!52, !17, !"OMPAliasScope"}
!53 = distinct !{!53, !17, !"OMPAliasScope"}
!54 = distinct !{!54, !17, !"OMPAliasScope"}
!55 = distinct !{!55, !17, !"OMPAliasScope"}
!56 = distinct !{!56, !17, !"OMPAliasScope"}
!57 = distinct !{!57, !17, !"OMPAliasScope"}
!58 = distinct !{!58, !17, !"OMPAliasScope"}
!59 = distinct !{!59, !17, !"OMPAliasScope"}
!60 = distinct !{!60, !17, !"OMPAliasScope"}
!61 = distinct !{!61, !17, !"OMPAliasScope"}
!62 = distinct !{!62, !17, !"OMPAliasScope"}
!63 = distinct !{!63, !17, !"OMPAliasScope"}
!64 = distinct !{!64, !17, !"OMPAliasScope"}
!65 = distinct !{!65, !17, !"OMPAliasScope"}
!66 = distinct !{!66, !17, !"OMPAliasScope"}
!67 = distinct !{!67, !17, !"OMPAliasScope"}
!68 = distinct !{!68, !17, !"OMPAliasScope"}
!69 = distinct !{!69, !17, !"OMPAliasScope"}
!70 = distinct !{!70, !17, !"OMPAliasScope"}
!71 = distinct !{!71, !17, !"OMPAliasScope"}
!72 = distinct !{!72, !17, !"OMPAliasScope"}
!73 = distinct !{!73, !17, !"OMPAliasScope"}
!74 = distinct !{!74, !17, !"OMPAliasScope"}
!75 = distinct !{!75, !17, !"OMPAliasScope"}
!76 = distinct !{!76, !17, !"OMPAliasScope"}
!77 = distinct !{!77, !17, !"OMPAliasScope"}
!78 = distinct !{!78, !17, !"OMPAliasScope"}
!79 = distinct !{!79, !17, !"OMPAliasScope"}
!80 = distinct !{!80, !17, !"OMPAliasScope"}
!81 = distinct !{!81, !17, !"OMPAliasScope"}
!82 = distinct !{!82, !17, !"OMPAliasScope"}
!83 = distinct !{!83, !17, !"OMPAliasScope"}
!84 = distinct !{!84, !17, !"OMPAliasScope"}
!85 = distinct !{!85, !17, !"OMPAliasScope"}
!86 = distinct !{!86, !17, !"OMPAliasScope"}
!87 = distinct !{!87, !17, !"OMPAliasScope"}
!88 = distinct !{}
!89 = !{i64 309}
!90 = !{i64 302}
!91 = !{!92, !92, i64 0}
!92 = !{!"ifx$unique_sym$27", !93, i64 0}
!93 = !{!"Fortran Target Data Symbol", !12, i64 0}
!94 = !{!71}
!95 = !{!23, !25, !27, !28, !29, !30, !36, !37, !38, !40, !41, !42, !43, !44, !45, !16, !18, !19, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !73, !74, !78, !83, !84, !85, !86, !87}
!96 = !{i64 310}
!97 = !{!72}
!98 = !{i64 313}
!99 = !{!100, !100, i64 0}
!100 = !{!"ifx$unique_sym$28", !12, i64 0}
!101 = !{!73, !74}
!102 = !{!21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !16, !18, !19, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87}
!103 = !{i64 325}
!104 = !{!105, !105, i64 0}
!105 = !{!"ifx$unique_sym$29", !12, i64 0}
!106 = !{!77}
!107 = !{i64 6}
!108 = !{!16}
!109 = !{!23, !25, !27, !28, !29, !30, !36, !37, !38, !40, !41, !42, !43, !44, !45, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !73, !74, !78, !83, !84, !85, !86, !87}
!110 = !{i64 350}
!111 = !{i64 353}
!112 = !{!73}
!113 = !{!23, !25, !27, !28, !29, !30, !36, !37, !38, !40, !41, !42, !43, !44, !45, !16, !18, !19, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !78, !83, !84, !85, !86, !87}
!114 = !{i64 354}
!115 = !{!16, !18}
!116 = !{i64 369}
!117 = !{!53}
!118 = !{i64 384}
!119 = !{!120, !120, i64 0}
!120 = !{!"ifx$unique_sym$31", !12, i64 0}
!121 = !{!56}
!122 = !{!18, !19}
!123 = !{i64 381}
!124 = !{!16, !19}
!125 = distinct !{!125, !126, !130, !131}
!126 = distinct !{!"intel.optreport.rootnode", !127}
!127 = distinct !{!"intel.optreport", !128}
!128 = !{!"intel.optreport.remarks", !129}
!129 = !{!"intel.optreport.remark", i32 0, !"OpenMP: Outlined parallel loop"}
!130 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!131 = !{!"llvm.loop.parallel_accesses", !88}
!132 = !{i32 0, i32 2147483647}
!133 = !{!134, !135, i64 0}
!134 = !{!"ifx$descr$3", !135, i64 0, !135, i64 8, !135, i64 16, !135, i64 24, !135, i64 32, !135, i64 40, !135, i64 48, !135, i64 56, !135, i64 64, !135, i64 72, !135, i64 80, !135, i64 88}
!135 = !{!"ifx$descr$field", !136, i64 0}
!136 = !{!"Fortran Dope Vector Symbol", !13, i64 0}
!137 = !{!31}
!138 = !{!139, !139, i64 0}
!139 = !{!"ifx$unique_sym$20", !12, i64 0}
!140 = !{!39}
!141 = !{i64 257}
!142 = !{!143, !135, i64 0}
!143 = !{!"ifx$descr$5", !135, i64 0, !135, i64 8, !135, i64 16, !135, i64 24, !135, i64 32, !135, i64 40, !135, i64 48, !135, i64 56, !135, i64 64}
!144 = !{!51}
!145 = !{!143, !135, i64 64}
!146 = !{!52}
!147 = !{i64 367}
!148 = !{!149, !135, i64 0}
!149 = !{!"ifx$descr$6", !135, i64 0, !135, i64 8, !135, i64 16, !135, i64 24, !135, i64 32, !135, i64 40, !135, i64 48, !135, i64 56, !135, i64 64}
!150 = !{!54}
!151 = !{!149, !135, i64 64}
!152 = !{!55}
!153 = !{i64 382}
!154 = distinct !{!154, !130, !131}
