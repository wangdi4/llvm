; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt -passes="default<O3>" -S -loopopt -vplan-force-vf=4 -disable-hir-opt-predicate-select %s | FileCheck %s

; This code was asserting in HIR because SimpleUnswitcher disturbed the
; SIMD loopnest. SimpleUnswitcher is only run inside the new PM pipeline.

; This code is on the longer side, but it's needed otherwise the O3 pipeline
; will remove the failing pattern.
; The alias metadata is needed for unswitching to work.
; The CHECK lines are minimal to avoid maintenance updates when the O3
; optimizations change.

; Optimizing the Select instruction in HIR OptPredicate was disabled because it
; breaks the pattern that causes the issue in SimpleUnswitcher.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.kmpc_loc.0.0 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.2 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare void @__kmpc_for_static_init_8(%struct.ident_t*, i32, i32, i32*, i64*, i64*, i64*, i64, i64)

declare void @__kmpc_for_static_fini(%struct.ident_t*, i32)

; Function Attrs: nounwind uwtable
define hidden void @MAIN__.DIR.OMP.PARALLEL.LOOP.2.split78(i32* %tid, i32* %bid, float* %"_unnamed_main$$_$G", float* %"_unnamed_main$$_$D", float* %"_unnamed_main$$_$A", float* %do.step17, float* %do.start15, float* %do.step11, float* %do.start9, float* %do.step5, float* %do.start3, float* %do.step, float* %do.start, float* %omp.pdo.step, float* %omp.pdo.start, i64 %do.norm.ub.val86, i64 %omp.pdo.norm.ub.val, i64 %omp.collapsed.lb.val, i64 %omp.pdo.norm.lb.val, i64 %do.norm.lb.val, i64 %do.norm.lb6.val, i64 %do.norm.lb12.val, i64 %do.norm.lb18.val, i64* %omp.collapsed.ub, i64 %"(i64)div.4$", float* %"_unnamed_main$$_$E") #1 {
; Just check that vectorization occurred -- O3 testing is too fragile.
; CHECK-LABEL: @MAIN__.DIR.OMP.PARALLEL.LOOP.2.split78(
; CHECK:    [[TMP8:%.*]] = icmp sgt <4 x i64> {{.*}}, <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[TMP9:%.*]] = select <4 x i1> [[TMP8]], <4 x i1> <i1 true, i1 poison, i1 poison, i1 poison>
; CHECK-NEXT:    [[TMP10:%.*]] = extractelement <4 x i1> [[TMP9]], i64 0
;
newFuncRoot:
  %do.norm.ub.fpriv = alloca i64, align 8
  %omp.pdo.norm.ub.fpriv = alloca i64, align 8
  %omp.collapsed.lb.fpriv = alloca i64, align 8
  %omp.pdo.norm.lb.fpriv = alloca i64, align 8
  %do.norm.lb.fpriv = alloca i64, align 8
  %do.norm.lb6.fpriv = alloca i64, align 8
  %do.norm.lb12.fpriv = alloca i64, align 8
  %do.norm.lb18.fpriv = alloca i64, align 8
  %"_unnamed_main$$_$E.lpriv" = alloca float, align 8
  %"_unnamed_main$$_$B.priv" = alloca float, align 8
  %do.norm.iv.priv = alloca i64, align 8
  %omp.pdo.norm.iv.priv = alloca i64, align 8
  %"_unnamed_main$$_$C.priv" = alloca float, align 8
  %"_unnamed_main$$_$F.priv" = alloca float, align 8
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i64, align 4
  %upper.bnd = alloca i64, align 4
  %stride = alloca i64, align 4
  %upperD = alloca i64, align 4
  %"_unnamed_main$$_$G.fp" = alloca float, align 8, !llfort.type_idx !0
  %"_unnamed_main$$_$A.fp" = alloca float, align 8, !llfort.type_idx !1
  %do.step17.fp = alloca float, align 4, !llfort.type_idx !2
  %do.start15.fp = alloca float, align 4, !llfort.type_idx !2
  %do.step11.fp = alloca float, align 4, !llfort.type_idx !2
  %do.start9.fp = alloca float, align 4, !llfort.type_idx !2
  %do.step5.fp = alloca float, align 4, !llfort.type_idx !2
  %do.start3.fp = alloca float, align 4, !llfort.type_idx !2
  %do.step.fp = alloca float, align 4, !llfort.type_idx !2
  %do.start.fp = alloca float, align 4, !llfort.type_idx !2
  %omp.pdo.step.fp = alloca float, align 4, !llfort.type_idx !2
  %omp.pdo.start.fp = alloca float, align 4, !llfort.type_idx !2
  br label %DIR.OMP.PARALLEL.LOOP.2.split78

DIR.OMP.PARALLEL.LOOP.2.split78:                  ; preds = %newFuncRoot
  store i32 0, i32* %is.last, align 4
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split85

DIR.OMP.PARALLEL.LOOP.2.split78.split85:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78
  store i64 %do.norm.ub.val86, i64* %do.norm.ub.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split84

DIR.OMP.PARALLEL.LOOP.2.split78.split84:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split85
  store i64 %omp.pdo.norm.ub.val, i64* %omp.pdo.norm.ub.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split83

DIR.OMP.PARALLEL.LOOP.2.split78.split83:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split84
  store i64 %omp.collapsed.lb.val, i64* %omp.collapsed.lb.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split82

DIR.OMP.PARALLEL.LOOP.2.split78.split82:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split83
  store i64 %omp.pdo.norm.lb.val, i64* %omp.pdo.norm.lb.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split81

DIR.OMP.PARALLEL.LOOP.2.split78.split81:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split82
  store i64 %do.norm.lb.val, i64* %do.norm.lb.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split80

DIR.OMP.PARALLEL.LOOP.2.split78.split80:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split81
  store i64 %do.norm.lb6.val, i64* %do.norm.lb6.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split79

DIR.OMP.PARALLEL.LOOP.2.split78.split79:          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split80
  store i64 %do.norm.lb12.val, i64* %do.norm.lb12.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split78.split

DIR.OMP.PARALLEL.LOOP.2.split78.split:            ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split79
  store i64 %do.norm.lb18.val, i64* %do.norm.lb18.fpriv, align 8
  br label %DIR.OMP.PARALLEL.LOOP.2.split75

DIR.OMP.PARALLEL.LOOP.2.split75:                  ; preds = %DIR.OMP.PARALLEL.LOOP.2.split78.split
  %0 = load i64, i64* %omp.collapsed.ub, align 8, !alias.scope !3, !noalias !6
  br label %DIR.OMP.PARALLEL.LOOP.2.split

DIR.OMP.PARALLEL.LOOP.2.split:                    ; preds = %DIR.OMP.PARALLEL.LOOP.2.split75
  %"_unnamed_main$$_$G.v" = load float, float* %"_unnamed_main$$_$G", align 4, !alias.scope !32, !noalias !6
  store float %"_unnamed_main$$_$G.v", float* %"_unnamed_main$$_$G.fp", align 4, !alias.scope !34, !noalias !35
  %"_unnamed_main$$_$A.v" = load float, float* %"_unnamed_main$$_$A", align 4, !alias.scope !51, !noalias !6
  store float %"_unnamed_main$$_$A.v", float* %"_unnamed_main$$_$A.fp", align 4, !alias.scope !52, !noalias !53
  %do.step17.v = load float, float* %do.step17, align 4, !alias.scope !54, !noalias !6
  store float %do.step17.v, float* %do.step17.fp, align 4, !alias.scope !55, !noalias !56
  %do.start15.v = load float, float* %do.start15, align 4, !alias.scope !57, !noalias !6
  store float %do.start15.v, float* %do.start15.fp, align 4, !alias.scope !58, !noalias !59
  %do.step11.v = load float, float* %do.step11, align 4, !alias.scope !60, !noalias !6
  store float %do.step11.v, float* %do.step11.fp, align 4, !alias.scope !61, !noalias !62
  %do.start9.v = load float, float* %do.start9, align 4, !alias.scope !63, !noalias !6
  store float %do.start9.v, float* %do.start9.fp, align 4, !alias.scope !64, !noalias !65
  %do.step5.v = load float, float* %do.step5, align 4, !alias.scope !66, !noalias !6
  store float %do.step5.v, float* %do.step5.fp, align 4, !alias.scope !67, !noalias !68
  %do.start3.v = load float, float* %do.start3, align 4, !alias.scope !69, !noalias !6
  store float %do.start3.v, float* %do.start3.fp, align 4, !alias.scope !70, !noalias !71
  %do.step.v = load float, float* %do.step, align 4, !alias.scope !72, !noalias !6
  store float %do.step.v, float* %do.step.fp, align 4, !alias.scope !73, !noalias !74
  %do.start.v = load float, float* %do.start, align 4, !alias.scope !75, !noalias !6
  store float %do.start.v, float* %do.start.fp, align 4, !alias.scope !76, !noalias !77
  %omp.pdo.step.v = load float, float* %omp.pdo.step, align 4, !alias.scope !78, !noalias !6
  store float %omp.pdo.step.v, float* %omp.pdo.step.fp, align 4, !alias.scope !79, !noalias !80
  %omp.pdo.start.v = load float, float* %omp.pdo.start, align 4, !alias.scope !81, !noalias !6
  store float %omp.pdo.start.v, float* %omp.pdo.start.fp, align 4, !alias.scope !82, !noalias !83
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.2.split
  br label %DIR.OMP.PARALLEL.LOOP.1.split

omp.pdo.cond7:                                    ; preds = %omp.collapsed.loop.body, %do.epilog16
  %omp.pdo.norm.iv_fetch.5 = load i64, i64* %omp.pdo.norm.iv.priv, align 1, !tbaa !84, !alias.scope !87, !noalias !88, !llvm.access.group !89
  %omp.pdo.norm.ub_fetch.6 = load i64, i64* %omp.pdo.norm.ub.fpriv, align 1, !tbaa !84, !alias.scope !90, !noalias !91, !llvm.access.group !89
  %rel.1.not = icmp sgt i64 %omp.pdo.norm.iv_fetch.5, %omp.pdo.norm.ub_fetch.6
  br i1 %rel.1.not, label %omp.collapsed.loop.inc, label %omp.pdo.body8

omp.pdo.body8:                                    ; preds = %omp.pdo.cond7
  %"(float)omp.pdo.norm.iv_fetch.7$" = sitofp i64 %omp.pdo.norm.iv_fetch.5 to float
  %omp.pdo.step_fetch.8 = load float, float* %omp.pdo.step.fp, align 1, !tbaa !84, !alias.scope !79, !noalias !92, !llvm.access.group !89
  %mul.1 = fmul reassoc ninf nsz arcp contract afn float %omp.pdo.step_fetch.8, %"(float)omp.pdo.norm.iv_fetch.7$"
  %omp.pdo.start_fetch.9 = load float, float* %omp.pdo.start.fp, align 1, !tbaa !84, !alias.scope !82, !noalias !93, !llvm.access.group !89
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %mul.1, %omp.pdo.start_fetch.9
  store float %add.1, float* %"_unnamed_main$$_$E.lpriv", align 8, !tbaa !94, !alias.scope !96, !noalias !97, !llvm.access.group !89
  %do.norm.lb_fetch.13 = load i64, i64* %do.norm.lb.fpriv, align 1, !tbaa !84, !alias.scope !98, !noalias !99, !llvm.access.group !89
  br label %do.cond14

do.cond14:                                        ; preds = %do.epilog20, %omp.pdo.body8
  %storemerge = phi i64 [ %do.norm.lb_fetch.13, %omp.pdo.body8 ], [ %add.9, %do.epilog20 ]
  store i64 %storemerge, i64* %do.norm.iv.priv, align 1, !tbaa !84, !alias.scope !100, !noalias !101, !llvm.access.group !89
  %do.norm.ub_fetch.15 = load i64, i64* %do.norm.ub.fpriv, align 1, !tbaa !84, !alias.scope !102, !noalias !103, !llvm.access.group !89
  %rel.2.not = icmp sgt i64 %storemerge, %do.norm.ub_fetch.15
  br i1 %rel.2.not, label %do.epilog16, label %do.body15

do.body15:                                        ; preds = %do.cond14
  %"(float)do.norm.iv_fetch.16$" = sitofp i64 %storemerge to float
  %do.step_fetch.17 = load float, float* %do.step.fp, align 1, !tbaa !84, !alias.scope !73, !noalias !104, !llvm.access.group !89
  %mul.2 = fmul reassoc ninf nsz arcp contract afn float %do.step_fetch.17, %"(float)do.norm.iv_fetch.16$"
  %do.start_fetch.18 = load float, float* %do.start.fp, align 1, !tbaa !84, !alias.scope !76, !noalias !105, !llvm.access.group !89
  %add.2 = fadd reassoc ninf nsz arcp contract afn float %mul.2, %do.start_fetch.18
  store float %add.2, float* %"_unnamed_main$$_$B.priv", align 8, !tbaa !106, !alias.scope !108, !noalias !109, !llvm.access.group !89
  %do.norm.lb6_fetch.22 = load i64, i64* %do.norm.lb6.fpriv, align 1, !tbaa !84, !alias.scope !110, !noalias !6, !llvm.access.group !89
  br label %do.cond18

do.cond18:                                        ; preds = %do.epilog29, %do.body15
  %do.norm.iv8.0 = phi i64 [ %do.norm.lb6_fetch.22, %do.body15 ], [ %add.8, %do.epilog29 ]
  %rel.3 = icmp slt i64 %do.norm.iv8.0, 3
  br i1 %rel.3, label %do.body19, label %do.epilog20

do.body19:                                        ; preds = %do.cond18
  %"(float)do.norm.iv8_fetch.25$" = sitofp i64 %do.norm.iv8.0 to float
  %do.step5_fetch.26 = load float, float* %do.step5.fp, align 1, !tbaa !84, !alias.scope !67, !noalias !111, !llvm.access.group !89
  %mul.3 = fmul reassoc ninf nsz arcp contract afn float %do.step5_fetch.26, %"(float)do.norm.iv8_fetch.25$"
  %do.start3_fetch.27 = load float, float* %do.start3.fp, align 1, !tbaa !84, !alias.scope !70, !noalias !112, !llvm.access.group !89
  %add.3 = fadd reassoc ninf nsz arcp contract afn float %mul.3, %do.start3_fetch.27
  store float %add.3, float* %"_unnamed_main$$_$C.priv", align 8, !tbaa !113, !alias.scope !115, !noalias !116, !llvm.access.group !89
  %do.norm.lb12_fetch.32 = load i64, i64* %do.norm.lb12.fpriv, align 1, !tbaa !84, !alias.scope !117, !noalias !6, !llvm.access.group !89
  br label %do.cond22

do.cond22:                                        ; preds = %do.body23, %do.body19
  %do.norm.iv14.0 = phi i64 [ %do.norm.lb12_fetch.32, %do.body19 ], [ %add.5, %do.body23 ]
  %rel.4.not = icmp sgt i64 %do.norm.iv14.0, %"(i64)div.4$"
  br i1 %rel.4.not, label %do.epilog24, label %do.body23

do.body23:                                        ; preds = %do.cond22
  %"(float)do.norm.iv14_fetch.35$" = sitofp i64 %do.norm.iv14.0 to float
  %do.step11_fetch.36 = load float, float* %do.step11.fp, align 1, !tbaa !84, !alias.scope !61, !noalias !118, !llvm.access.group !89
  %mul.4 = fmul reassoc ninf nsz arcp contract afn float %do.step11_fetch.36, %"(float)do.norm.iv14_fetch.35$"
  %do.start9_fetch.37 = load float, float* %do.start9.fp, align 1, !tbaa !84, !alias.scope !64, !noalias !119, !llvm.access.group !89
  %add.4 = fadd reassoc ninf nsz arcp contract afn float %mul.4, %do.start9_fetch.37
  store float %add.4, float* %"_unnamed_main$$_$F.priv", align 8, !tbaa !120, !alias.scope !122, !noalias !123, !llvm.access.group !89
  %"_unnamed_main$$_$G_fetch.38" = load float, float* %"_unnamed_main$$_$G.fp", align 8, !tbaa !124, !alias.scope !34, !noalias !126, !llvm.access.group !89
  store float %"_unnamed_main$$_$G_fetch.38", float* %"_unnamed_main$$_$D", align 8, !tbaa !127, !alias.scope !129, !noalias !130, !llvm.access.group !89
  %add.5 = add nsw i64 %do.norm.iv14.0, 1
  br label %do.cond22

do.epilog24:                                      ; preds = %do.cond22
  %do.norm.lb18_fetch.43 = load i64, i64* %do.norm.lb18.fpriv, align 1, !tbaa !84, !alias.scope !131, !noalias !6, !llvm.access.group !89
  br label %do.cond27

do.cond27:                                        ; preds = %do.body28, %do.epilog24
  %do.norm.iv20.0 = phi i64 [ %do.norm.lb18_fetch.43, %do.epilog24 ], [ %add.7, %do.body28 ]
  %rel.5 = icmp slt i64 %do.norm.iv20.0, 3
  br i1 %rel.5, label %do.body28, label %do.epilog29

do.body28:                                        ; preds = %do.cond27
  %"(float)do.norm.iv20_fetch.46$" = sitofp i64 %do.norm.iv20.0 to float
  %do.step17_fetch.47 = load float, float* %do.step17.fp, align 1, !tbaa !84, !alias.scope !55, !noalias !132, !llvm.access.group !89
  %mul.5 = fmul reassoc ninf nsz arcp contract afn float %do.step17_fetch.47, %"(float)do.norm.iv20_fetch.46$"
  %do.start15_fetch.48 = load float, float* %do.start15.fp, align 1, !tbaa !84, !alias.scope !58, !noalias !133, !llvm.access.group !89
  %add.6 = fadd reassoc ninf nsz arcp contract afn float %mul.5, %do.start15_fetch.48
  store float %add.6, float* %"_unnamed_main$$_$F.priv", align 8, !tbaa !120, !alias.scope !122, !noalias !123, !llvm.access.group !89
  %add.7 = add nsw i64 %do.norm.iv20.0, 1
  br label %do.cond27

do.epilog29:                                      ; preds = %do.cond27
  %add.8 = add nsw i64 %do.norm.iv8.0, 1
  br label %do.cond18

do.epilog20:                                      ; preds = %do.cond18
  %do.norm.iv_fetch.51 = load i64, i64* %do.norm.iv.priv, align 1, !tbaa !84, !alias.scope !100, !noalias !134, !llvm.access.group !89
  %add.9 = add nsw i64 %do.norm.iv_fetch.51, 1
  br label %do.cond14

do.epilog16:                                      ; preds = %do.cond14
  %omp.pdo.norm.iv_fetch.52 = load i64, i64* %omp.pdo.norm.iv.priv, align 1, !tbaa !84, !alias.scope !135, !noalias !88, !llvm.access.group !89
  %add.10 = add nsw i64 %omp.pdo.norm.iv_fetch.52, 1
  store i64 %add.10, i64* %omp.pdo.norm.iv.priv, align 1, !tbaa !84, !alias.scope !136, !noalias !137, !llvm.access.group !89
  br label %omp.pdo.cond7

DIR.OMP.PARALLEL.LOOP.1.split:                    ; preds = %DIR.OMP.PARALLEL.LOOP.3
  %do.norm.ub.val21 = load i64, i64* %do.norm.ub.fpriv, align 8, !alias.scope !138, !noalias !103
  %1 = add nuw nsw i64 %do.norm.ub.val21, 1
  br label %DIR.OMP.SIMD.4

DIR.OMP.SIMD.4:                                   ; preds = %DIR.OMP.PARALLEL.LOOP.1.split
  br label %DIR.OMP.SIMD.5

DIR.OMP.SIMD.5:                                   ; preds = %DIR.OMP.SIMD.4
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.5
  %omp.pdo.norm.lb_fetch.4 = load i64, i64* %omp.pdo.norm.lb.fpriv, align 1, !tbaa !84, !alias.scope !139, !noalias !140
  store i64 %omp.pdo.norm.lb_fetch.4, i64* %omp.pdo.norm.iv.priv, align 1, !tbaa !84, !alias.scope !136, !noalias !137
  %2 = load i64, i64* %omp.collapsed.lb.fpriv, align 8, !alias.scope !141, !noalias !6
  %.not76 = icmp sgt i64 %2, %0
  br i1 %.not76, label %omp.collapsed.loop.postexit.split.loopexit, label %omp.collapsed.loop.body.lr.ph, !prof !142

omp.collapsed.loop.body.lr.ph:                    ; preds = %DIR.OMP.SIMD.2
  %my.tid = load i32, i32* %tid, align 4
  store i64 %2, i64* %lower.bnd, align 4
  %.norm.ub.for.scheduling = load i64, i64* %omp.collapsed.ub, align 8
  store i64 %.norm.ub.for.scheduling, i64* %upper.bnd, align 4
  store i64 1, i64* %stride, align 4
  store i64 %.norm.ub.for.scheduling, i64* %upperD, align 4
  call void @__kmpc_for_static_init_8(%struct.ident_t* @.kmpc_loc.0.0, i32 %my.tid, i32 34, i32* %is.last, i64* %lower.bnd, i64* %upper.bnd, i64* %stride, i64 1, i64 1)
  %lb.new = load i64, i64* %lower.bnd, align 4, !range !143
  %ub.new = load i64, i64* %upper.bnd, align 4, !range !143
  %omp.ztt = icmp sle i64 %lb.new, %ub.new
  br i1 %omp.ztt, label %omp.collapsed.loop.body.preheader, label %loop.region.exit

omp.collapsed.loop.body.preheader:                ; preds = %omp.collapsed.loop.body.lr.ph
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.PRIVATE"(float* %"_unnamed_main$$_$F.priv"), "QUAL.OMP.PRIVATE"(float* %"_unnamed_main$$_$C.priv"), "QUAL.OMP.LASTPRIVATE"(float* null), "QUAL.OMP.LASTPRIVATE"(float* null), "QUAL.OMP.LIVEIN"(float* %"_unnamed_main$$_$D"), "QUAL.OMP.LIVEIN"(float* %"_unnamed_main$$_$G.fp"), "QUAL.OMP.LIVEIN"(float* %"_unnamed_main$$_$A.fp"), "QUAL.OMP.PRIVATE"(float* %"_unnamed_main$$_$B.priv"), "QUAL.OMP.PRIVATE"(float* %"_unnamed_main$$_$E.lpriv") ]
  br label %omp.collapsed.loop.body

omp.collapsed.loop.body:                          ; preds = %omp.collapsed.loop.inc, %omp.collapsed.loop.body.preheader
  %omp.collapsed.iv.local.077 = phi i64 [ %6, %omp.collapsed.loop.inc ], [ %lb.new, %omp.collapsed.loop.body.preheader ]
  %4 = sdiv i64 %omp.collapsed.iv.local.077, %1
  store i64 %4, i64* %omp.pdo.norm.lb.fpriv, align 8, !alias.scope !139, !noalias !144, !llvm.access.group !89
  store i64 %4, i64* %omp.pdo.norm.ub.fpriv, align 8, !alias.scope !90, !noalias !145, !llvm.access.group !89
  %5 = srem i64 %omp.collapsed.iv.local.077, %1
  store i64 %4, i64* %omp.pdo.norm.iv.priv, align 8, !alias.scope !136, !noalias !137, !llvm.access.group !89
  store i64 %5, i64* %do.norm.lb.fpriv, align 8, !alias.scope !98, !noalias !146, !llvm.access.group !89
  store i64 %5, i64* %do.norm.ub.fpriv, align 8, !alias.scope !147, !noalias !148, !llvm.access.group !89
  br label %omp.pdo.cond7

omp.collapsed.loop.inc:                           ; preds = %omp.pdo.cond7
  %6 = add nuw nsw i64 %omp.collapsed.iv.local.077, 1
  %.not = icmp sle i64 %6, %ub.new
  br i1 %.not, label %omp.collapsed.loop.body, label %omp.collapsed.loop.cond.omp.collapsed.loop.postexit.split.loopexit_crit_edge, !prof !149, !llvm.loop !150

omp.collapsed.loop.cond.omp.collapsed.loop.postexit.split.loopexit_crit_edge: ; preds = %omp.collapsed.loop.inc
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %omp.collapsed.loop.cond.omp.collapsed.loop.postexit.split.loopexit_crit_edge, %omp.collapsed.loop.body.lr.ph
  call void @__kmpc_for_static_fini(%struct.ident_t* @.kmpc_loc.0.0.2, i32 %my.tid)
  br label %loop.region.exit.split

loop.region.exit.split:                           ; preds = %loop.region.exit
  %7 = load i32, i32* %is.last, align 4
  %8 = icmp ne i32 %7, 0
  br i1 %8, label %last.then, label %last.done

last.then:                                        ; preds = %loop.region.exit.split
  %9 = load float, float* %"_unnamed_main$$_$E.lpriv", align 4
  store float %9, float* %"_unnamed_main$$_$E", align 4
  br label %last.done

last.done:                                        ; preds = %last.then, %loop.region.exit.split
  br label %omp.collapsed.loop.postexit.split.loopexit

omp.collapsed.loop.postexit.split.loopexit:       ; preds = %last.done, %DIR.OMP.SIMD.2
  br label %omp.collapsed.loop.postexit.split

omp.collapsed.loop.postexit.split:                ; preds = %omp.collapsed.loop.postexit.split.loopexit
  br label %DIR.OMP.END.SIMD.6

DIR.OMP.END.SIMD.6:                               ; preds = %omp.collapsed.loop.postexit.split
  br label %DIR.OMP.END.SIMD.3.split

DIR.OMP.END.SIMD.3.split:                         ; preds = %DIR.OMP.END.SIMD.6
  br label %DIR.OMP.END.PARALLEL.LOOP.7

DIR.OMP.END.PARALLEL.LOOP.7:                      ; preds = %DIR.OMP.END.SIMD.3.split
  br label %DIR.OMP.END.PARALLEL.LOOP.8.exitStub

DIR.OMP.END.PARALLEL.LOOP.8.exitStub:             ; preds = %DIR.OMP.END.PARALLEL.LOOP.7
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "pre_loopopt" "processed-by-vpo" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!0 = !{i64 21}
!1 = !{i64 23}
!2 = !{i64 5}
!3 = !{!4}
!4 = distinct !{!4, !5, !"OMPAliasScope"}
!5 = distinct !{!5, !"OMPDomain"}
!6 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!7 = distinct !{!7, !5, !"OMPAliasScope"}
!8 = distinct !{!8, !5, !"OMPAliasScope"}
!9 = distinct !{!9, !5, !"OMPAliasScope"}
!10 = distinct !{!10, !5, !"OMPAliasScope"}
!11 = distinct !{!11, !5, !"OMPAliasScope"}
!12 = distinct !{!12, !5, !"OMPAliasScope"}
!13 = distinct !{!13, !5, !"OMPAliasScope"}
!14 = distinct !{!14, !5, !"OMPAliasScope"}
!15 = distinct !{!15, !5, !"OMPAliasScope"}
!16 = distinct !{!16, !5, !"OMPAliasScope"}
!17 = distinct !{!17, !5, !"OMPAliasScope"}
!18 = distinct !{!18, !5, !"OMPAliasScope"}
!19 = distinct !{!19, !5, !"OMPAliasScope"}
!20 = distinct !{!20, !5, !"OMPAliasScope"}
!21 = distinct !{!21, !5, !"OMPAliasScope"}
!22 = distinct !{!22, !5, !"OMPAliasScope"}
!23 = distinct !{!23, !5, !"OMPAliasScope"}
!24 = distinct !{!24, !5, !"OMPAliasScope"}
!25 = distinct !{!25, !5, !"OMPAliasScope"}
!26 = distinct !{!26, !5, !"OMPAliasScope"}
!27 = distinct !{!27, !5, !"OMPAliasScope"}
!28 = distinct !{!28, !5, !"OMPAliasScope"}
!29 = distinct !{!29, !5, !"OMPAliasScope"}
!30 = distinct !{!30, !5, !"OMPAliasScope"}
!31 = distinct !{!31, !5, !"OMPAliasScope"}
!32 = !{!33}
!33 = distinct !{!33, !5, !"OMPAliasScope"}
!34 = !{!7}
!35 = !{!4, !33, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!36 = distinct !{!36, !5, !"OMPAliasScope"}
!37 = distinct !{!37, !5, !"OMPAliasScope"}
!38 = distinct !{!38, !5, !"OMPAliasScope"}
!39 = distinct !{!39, !5, !"OMPAliasScope"}
!40 = distinct !{!40, !5, !"OMPAliasScope"}
!41 = distinct !{!41, !5, !"OMPAliasScope"}
!42 = distinct !{!42, !5, !"OMPAliasScope"}
!43 = distinct !{!43, !5, !"OMPAliasScope"}
!44 = distinct !{!44, !5, !"OMPAliasScope"}
!45 = distinct !{!45, !5, !"OMPAliasScope"}
!46 = distinct !{!46, !5, !"OMPAliasScope"}
!47 = distinct !{!47, !5, !"OMPAliasScope"}
!48 = distinct !{!48, !5, !"OMPAliasScope"}
!49 = distinct !{!49, !5, !"OMPAliasScope"}
!50 = distinct !{!50, !5, !"OMPAliasScope"}
!51 = !{!36}
!52 = !{!8}
!53 = !{!4, !33, !7, !36, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!54 = !{!37}
!55 = !{!9}
!56 = !{!4, !33, !7, !36, !8, !37, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!57 = !{!38}
!58 = !{!10}
!59 = !{!4, !33, !7, !36, !8, !37, !9, !38, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!60 = !{!39}
!61 = !{!11}
!62 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!63 = !{!40}
!64 = !{!12}
!65 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!66 = !{!41}
!67 = !{!13}
!68 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!69 = !{!42}
!70 = !{!14}
!71 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!72 = !{!43}
!73 = !{!15}
!74 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!75 = !{!44}
!76 = !{!16}
!77 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!78 = !{!45}
!79 = !{!17}
!80 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!81 = !{!46}
!82 = !{!18}
!83 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!84 = !{!85, !85, i64 0}
!85 = !{!"Generic Fortran Symbol", !86, i64 0}
!86 = !{!"ifx$root$1$MAIN__"}
!87 = !{!20}
!88 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!89 = distinct !{}
!90 = !{!22}
!91 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!92 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!93 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!94 = !{!95, !95, i64 0}
!95 = !{!"ifx$unique_sym$1", !85, i64 0}
!96 = !{!26}
!97 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !27, !28, !48, !29, !49, !50, !30, !31}
!98 = !{!23}
!99 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !24, !25, !26, !27, !28, !29, !30, !31}
!100 = !{!27}
!101 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !28, !48, !29, !49, !50, !30, !31}
!102 = !{!24}
!103 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !26, !27, !28, !29, !30, !31}
!104 = !{!7, !8, !9, !10, !11, !12, !13, !14, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!105 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!106 = !{!107, !107, i64 0}
!107 = !{!"ifx$unique_sym$2", !85, i64 0}
!108 = !{!28}
!109 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !48, !29, !49, !50, !30, !31}
!110 = !{!48}
!111 = !{!7, !8, !9, !10, !11, !12, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!112 = !{!7, !8, !9, !10, !11, !12, !13, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!113 = !{!114, !114, i64 0}
!114 = !{!"ifx$unique_sym$3", !85, i64 0}
!115 = !{!29}
!116 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !49, !50, !30, !31}
!117 = !{!49}
!118 = !{!7, !8, !9, !10, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!119 = !{!7, !8, !9, !10, !11, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!120 = !{!121, !121, i64 0}
!121 = !{!"ifx$unique_sym$5", !85, i64 0}
!122 = !{!30}
!123 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !31}
!124 = !{!125, !125, i64 0}
!125 = !{!"ifx$unique_sym$6", !85, i64 0}
!126 = !{!8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!127 = !{!128, !128, i64 0}
!128 = !{!"ifx$unique_sym$7", !85, i64 0}
!129 = !{!31}
!130 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30}
!131 = !{!50}
!132 = !{!7, !8, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!133 = !{!7, !8, !9, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!134 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !28, !29, !30, !31}
!135 = !{!19}
!136 = !{!19, !20}
!137 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!138 = !{!25}
!139 = !{!21}
!140 = !{!7, !8, !9, !10, !11, !12, !13, !14, !15, !16, !17, !18, !19, !20, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31}
!141 = !{!47}
!142 = !{!"branch_weights", i32 1, i32 99}
!143 = !{i64 0, i64 9223372036854775807}
!144 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !19, !20, !47, !22, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!145 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !23, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!146 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !25, !21, !19, !20, !47, !22, !24, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!147 = !{!24, !25}
!148 = !{!4, !33, !7, !36, !8, !37, !9, !38, !10, !39, !11, !40, !12, !41, !13, !42, !14, !43, !15, !44, !16, !45, !17, !46, !18, !21, !19, !20, !47, !22, !23, !26, !27, !28, !48, !29, !49, !50, !30, !31}
!149 = !{!"branch_weights", i32 99, i32 1}
!150 = distinct !{!150, !151, !155, !156}
!151 = distinct !{!"intel.optreport.rootnode", !152}
!152 = distinct !{!"intel.optreport", !153}
!153 = !{!"intel.optreport.remarks", !154}
!154 = !{!"intel.optreport.remark", i32 0, !"OpenMP: Outlined parallel loop"}
!155 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!156 = !{!"llvm.loop.parallel_accesses", !89}

; end INTEL_FEATURE_SW_ADVANCED
