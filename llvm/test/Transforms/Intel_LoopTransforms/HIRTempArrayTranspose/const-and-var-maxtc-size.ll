; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-array-transpose,print<hir>" -hir-temp-array-transpose-max-const-dimsize=400 -disable-output 2>&1 | FileCheck %s

; Check that transpose succeeds for one const and one var dimension.
; For now test is not testing new functionality, as the variable path has been optimized
; to const for i3 loop.

; HIR Before
;         BEGIN REGION { }
;               + DO i1 = 0, 199, 1   <DO_LOOP>
;               |   + DO i2 = 0, 299, 1   <DO_LOOP>
;               |   |   (@global.3)[0][i2][i1] = 0;
;               |   |
;               |   |      %phi132 = 0;
;               |   |   + DO i3 = 0, sext.i32.i64(%load92) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 400>  <LEGAL_MAX_TC = 2147483647>
;               |   |   |   %load136 = (@global.5)[0][i3][i1];
;               |   |   |   %load138 = (@global.4)[0][i2][i3];
;               |   |   |   %phi132 = (%load138 * %load136)  +  %phi132;
;               |   |   + END LOOP
;               |   |      (@global.3)[0][i2][i1] = %phi132;
;               |   + END LOOP
;               + END LOOP
;         END REGION


; HIR After
; CHECK:  BEGIN REGION { modified }
; CHECK:        %call = @llvm.stacksave.p0();
; CHECK:        %TranspTmpArr = alloca 320000;
;
; CHECK:        + DO i1 = 0, 199, 1   <DO_LOOP>
;               |   + DO i2 = 0, 399, 1   <DO_LOOP>
; CHECK:        |   |   (%TranspTmpArr)[i1][i2] = (@global.5)[0][i2][i1];
;               |   + END LOOP
;               + END LOOP
;
;
; CHECK:        + DO i1 = 0, 199, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 299, 1   <DO_LOOP>
; CHECK:        |   |   (@global.3)[0][i2][i1] = 0;
;               |   |
;               |   |      %phi132 = 0;
; CHECK:        |   |   + DO i3 = 0, sext.i32.i64(%load92) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 400>  <LEGAL_MAX_TC = 2147483647>
; CHECK:        |   |   |   %load136 = (%TranspTmpArr)[i1][i3];
; CHECK:        |   |   |   %load138 = (@global.4)[0][i2][i3];
;               |   |   |   %phi132 = (%load138 * %load136)  +  %phi132;
;               |   |   + END LOOP
;               |   |      (@global.3)[0][i2][i1] = %phi132;
;               |   + END LOOP
;               + END LOOP
;
;               @llvm.stackrestore.p0(&((%call)[0]));
;         END REGION


; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-array-transpose,print<hir>" -hir-temp-array-transpose-max-const-dimsize=200 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-DISABLED

; CHECK-DISABLED: BEGIN REGION
; CHECK-DISABLED-NOT: modified



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global.3 = external hidden unnamed_addr global [300 x [200 x i32]], align 16
@global.4 = external hidden global [300 x [400 x i32]], align 16
@global.5 = external hidden global [400 x [200 x i32]], align 16

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind uwtable
define dso_local void @wobble.bb116(i32 %load92) #1 {
newFuncRoot:
  br label %bb116

bb116:                                            ; preds = %newFuncRoot
  %icmp117 = icmp slt i32 %load92, 1
  br i1 false, label %bb118, label %bb119

bb118:                                            ; preds = %bb116
  br label %bb149.exitStub

bb119:                                            ; preds = %bb116
  %add120 = add nuw nsw i32 %load92, 1
  %sext = sext i32 %add120 to i64
  br label %bb121

bb121:                                            ; preds = %bb145, %bb119
  %phi122 = phi i64 [ 1, %bb119 ], [ %add146, %bb145 ]
  br label %bb123

bb123:                                            ; preds = %bb143, %bb121
  %phi124 = phi i64 [ %add125, %bb143 ], [ 1, %bb121 ]
  %add125 = add nuw nsw i64 %phi124, 1
  %call126 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 800, ptr nonnull elementtype(i32) @global.3, i64 %phi124), !llfort.type_idx !3
  %call127 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %call126, i64 %phi122), !llfort.type_idx !3
  store i32 0, ptr %call127, align 1, !tbaa !4, !noalias !9
  br i1 %icmp117, label %bb143, label %bb128

bb128:                                            ; preds = %bb123
  %call129 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1600, ptr elementtype(i32) @global.4, i64 %phi124), !llfort.type_idx !15
  br label %bb130

bb130:                                            ; preds = %bb130, %bb128
  %phi131 = phi i64 [ 1, %bb128 ], [ %add133, %bb130 ]
  %phi132 = phi i32 [ 0, %bb128 ], [ %add139, %bb130 ]
  %add133 = add nuw nsw i64 %phi131, 1
  %call134 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 800, ptr elementtype(i32) @global.5, i64 %phi131), !llfort.type_idx !15
  %call135 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %call134, i64 %phi122), !llfort.type_idx !15
  %load136 = load i32, ptr %call135, align 1, !tbaa !16, !alias.scope !18, !noalias !19, !llfort.type_idx !15
  %call137 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %call129, i64 %phi131), !llfort.type_idx !15
  %load138 = load i32, ptr %call137, align 1, !tbaa !20, !alias.scope !22, !noalias !23, !llfort.type_idx !15
  %mul = mul nsw i32 %load138, %load136
  %add139 = add nsw i32 %mul, %phi132
  %icmp140 = icmp eq i64 %add133, %sext
  br i1 %icmp140, label %bb141, label %bb130

bb141:                                            ; preds = %bb130
  %phi142 = phi i32 [ %add139, %bb130 ]
  store i32 %phi142, ptr %call127, align 1, !tbaa !4, !noalias !9
  br label %bb143

bb143:                                            ; preds = %bb141, %bb123
  %icmp144 = icmp eq i64 %add125, 301
  br i1 %icmp144, label %bb145, label %bb123

bb145:                                            ; preds = %bb143
  %add146 = add nuw nsw i64 %phi122, 1
  %icmp147 = icmp eq i64 %add146, 201
  br i1 %icmp147, label %bb148, label %bb121

bb148:                                            ; preds = %bb145
  br label %bb149.exitStub

bb149.exitStub:                                   ; preds = %bb148, %bb118
  ret void
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 235}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$26$0", !6, i64 0}
!6 = !{!"Fortran Data Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$2$foo_mat_mul_i4_$0"}
!9 = !{!10, !12, !13, !14}
!10 = distinct !{!10, !11, !"foo_mat_mul_i4_: argument 0"}
!11 = distinct !{!11, !"foo_mat_mul_i4_"}
!12 = distinct !{!12, !11, !"foo_mat_mul_i4_: argument 1"}
!13 = distinct !{!13, !11, !"foo_mat_mul_i4_"}
!14 = distinct !{!14, !11, !"foo_mat_mul_i4_"}
!15 = !{i64 2}
!16 = !{!17, !17, i64 0}
!17 = !{!"ifx$unique_sym$28$0", !6, i64 0}
!18 = !{!13}
!19 = !{!10, !12, !14}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$29$0", !6, i64 0}
!22 = !{!14}
!23 = !{!10, !12, !13}
