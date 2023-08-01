
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; Test checks that DD analysis sets (*) dependence on the reduced array levels (Fortran only).
; sum1 is sum reduction for 0 dimension. It reduce the array from n dim to n-1 dim.

; subroutine foo(A)
;   integer  A(3,4,2) , d
;   integer::sum1(4, 2)
;   d= 1
;   sum1 = sum(A, dim=d)
; end subroutine foo


;Function: foo_
;<10>         BEGIN REGION { }
;<73>               + DO i1 = 0, 1, 1   <DO_LOOP>
;<74>               |   + DO i2 = 0, 3, 1   <DO_LOOP>
;<75>               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;<24>               |   |   |   %"foo_$A_entry[][][]_fetch.7" = (%"foo_$A")[0:i1:48(i32*:2)][0:i2:12(i32*:4)][0:i3:4(i32*:3)];
;<25>               |   |   |   %"var$1618[][][]_fetch.38" = (%"var$161853.sub")[0:i1:16(i32*:0)][0:i2:4(i32*:4)][0:i3:0(i32*:0)];
;<27>               |   |   |   (%"var$161853.sub")[0:i1:16(i32*:0)][0:i2:4(i32*:4)][0:i3:0(i32*:0)] = %"foo_$A_entry[][][]_fetch.7" + %"var$1618[][][]_fetch.38";
;<75>               |   |   + END LOOP
;<74>               |   + END LOOP
;<73>               + END LOOP
;<10>         END REGION


; CHECK-LABEL: DD graph for function foo_:
; CHECK-DAG: (%"var$161853.sub")[i1][i2][i3] --> (%"var$161853.sub")[i1][i2][i3] ANTI (= = *) (0 0 ?)
; CHECK-DAG: (%"var$161853.sub")[i1][i2][i3] --> (%"var$161853.sub")[i1][i2][i3] FLOW (= = *) (0 0 ?)




target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"foo_$SUM1" = internal unnamed_addr global [2 x [4 x i32]] zeroinitializer, align 16, !llfort.type_idx !0

; Function Attrs: nofree nosync nounwind uwtable
define void @foo_(ptr noalias nocapture readonly dereferenceable(4) %"foo_$A") local_unnamed_addr #0 !llfort.type_idx !1 {
bb15:
  %"var$7" = alloca [2 x i64], align 16, !llfort.type_idx !2
  %"(ptr)var$7$6" = getelementptr inbounds [2 x i64], ptr %"var$7", i64 0, i64 0
  %"var$7[]7" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"(ptr)var$7$6", i64 0), !llfort.type_idx !3
  store i64 4, ptr %"var$7[]7", align 8, !tbaa !4
  %"var$7[]10" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"(ptr)var$7$6", i64 1), !llfort.type_idx !8
  store i64 2, ptr %"var$7[]10", align 8, !tbaa !4
  %"var$161853" = alloca [8 x i32], align 4
  %"var$161853.sub" = getelementptr inbounds [8 x i32], ptr %"var$161853", i64 0, i64 0
  br label %single_loop_body14

single_loop_body14:                               ; preds = %bb15, %single_loop_body14
  %"var$17.054" = phi i64 [ 1, %bb15 ], [ %add.7, %single_loop_body14 ]
  %"var$1618[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"var$161853.sub", i64 %"var$17.054"), !llfort.type_idx !9
  store i32 0, ptr %"var$1618[]", align 4, !tbaa !4
  %add.7 = add nuw nsw i64 %"var$17.054", 1
  %exitcond = icmp eq i64 %add.7, 9
  br i1 %exitcond, label %loop_test21.preheader.preheader, label %single_loop_body14

loop_test21.preheader.preheader:                  ; preds = %single_loop_body14
  br label %loop_test21.preheader

loop_body18:                                      ; preds = %loop_test17.preheader, %loop_body18
  %"$loop_ctr2.055" = phi i64 [ 1, %loop_test17.preheader ], [ %add.9, %loop_body18 ]
  %"var$1618[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr nonnull elementtype(i32) %"var$1618[][]", i64 %"$loop_ctr2.055"), !llfort.type_idx !10
  %"foo_$A_entry[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"foo_$A_entry[][]", i64 %"$loop_ctr2.055"), !llfort.type_idx !11
  %"foo_$A_entry[][][]_fetch.7" = load i32, ptr %"foo_$A_entry[][][]", align 1, !tbaa !12, !llfort.type_idx !14
  %"var$1618[][][]_fetch.38" = load i32, ptr %"var$1618[][][]", align 4, !tbaa !4, !llfort.type_idx !10
  %add.8 = add nsw i32 %"var$1618[][][]_fetch.38", %"foo_$A_entry[][][]_fetch.7"
  store i32 %add.8, ptr %"var$1618[][][]", align 4, !tbaa !4
  %add.9 = add nuw nsw i64 %"$loop_ctr2.055", 1
  %exitcond60.not = icmp eq i64 %add.9, 4
  br i1 %exitcond60.not, label %loop_exit19, label %loop_body18

loop_exit19:                                      ; preds = %loop_body18
  %add.10 = add nuw nsw i64 %"$loop_ctr3.056", 1
  %exitcond61.not = icmp eq i64 %add.10, 5
  br i1 %exitcond61.not, label %loop_exit23, label %loop_test17.preheader

loop_test17.preheader:                            ; preds = %loop_test21.preheader, %loop_exit19
  %"$loop_ctr3.056" = phi i64 [ 1, %loop_test21.preheader ], [ %add.10, %loop_exit19 ]
  %"var$1618[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4, ptr nonnull elementtype(i32) %"var$1618[]19", i64 %"$loop_ctr3.056"), !llfort.type_idx !15
  %"foo_$A_entry[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 12, ptr nonnull elementtype(i32) %"foo_$A_entry[]", i64 %"$loop_ctr3.056"), !llfort.type_idx !16
  br label %loop_body18

loop_exit23:                                      ; preds = %loop_exit19
  %add.11 = add nuw nsw i64 %"$loop_ctr4.057", 1
  %exitcond62.not = icmp eq i64 %add.11, 3
  br i1 %exitcond62.not, label %loop_test29.preheader.preheader, label %loop_test21.preheader

loop_test29.preheader.preheader:                  ; preds = %loop_exit23
  br label %loop_test29.preheader

loop_test21.preheader:                            ; preds = %loop_test21.preheader.preheader, %loop_exit23
  %"$loop_ctr4.057" = phi i64 [ 2, %loop_exit23 ], [ 1, %loop_test21.preheader.preheader ]
  %"var$1618[]19" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 16, ptr nonnull elementtype(i32) %"var$161853.sub", i64 %"$loop_ctr4.057"), !llfort.type_idx !17
  %"foo_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 48, ptr nonnull elementtype(i32) %"foo_$A", i64 %"$loop_ctr4.057"), !llfort.type_idx !18, !ifx.array_extent !19
  br label %loop_test17.preheader

loop_body30:                                      ; preds = %loop_test29.preheader, %loop_body30
  %"$loop_ctr.058" = phi i64 [ 1, %loop_test29.preheader ], [ %add.13, %loop_body30 ]
  %"var$1618[][]21" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"var$1618[]20", i64 %"$loop_ctr.058"), !llfort.type_idx !20
  %"var$1618[][]_fetch.47" = load i32, ptr %"var$1618[][]21", align 4, !tbaa !4, !llfort.type_idx !20
  %"foo_$SUM1[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$SUM1[]", i64 %"$loop_ctr.058"), !llfort.type_idx !21
  store i32 %"var$1618[][]_fetch.47", ptr %"foo_$SUM1[][]", align 4, !tbaa !22
  %add.13 = add nuw nsw i64 %"$loop_ctr.058", 1
  %exitcond63.not = icmp eq i64 %add.13, 5
  br i1 %exitcond63.not, label %loop_exit31, label %loop_body30

loop_exit31:                                      ; preds = %loop_body30
  %add.14 = add nuw nsw i64 %"$loop_ctr1.059", 1
  %exitcond64.not = icmp eq i64 %add.14, 3
  br i1 %exitcond64.not, label %loop_exit35, label %loop_test29.preheader

loop_test29.preheader:                            ; preds = %loop_test29.preheader.preheader, %loop_exit31
  %"$loop_ctr1.059" = phi i64 [ 2, %loop_exit31 ], [ 1, %loop_test29.preheader.preheader ]
  %"var$1618[]20" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(i32) %"var$161853.sub", i64 %"$loop_ctr1.059"), !llfort.type_idx !24
  %"foo_$SUM1[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr elementtype(i32) @"foo_$SUM1", i64 %"$loop_ctr1.059"), !llfort.type_idx !25, !ifx.array_extent !19
  br label %loop_body30

loop_exit35:                                      ; preds = %loop_exit31
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nofree nosync nounwind readnone speculatable

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{i64 19}
!1 = !{i64 28}
!2 = !{i64 39}
!3 = !{i64 44}
!4 = !{!5, !5, i64 0}
!5 = !{!"Fortran Data Symbol", !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$foo_"}
!8 = !{i64 46}
!9 = !{i64 51}
!10 = !{i64 54}
!11 = !{i64 37}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$2", !5, i64 0}
!14 = !{i64 38}
!15 = !{i64 53}
!16 = !{i64 36}
!17 = !{i64 52}
!18 = !{i64 35}
!19 = !{i64 2}
!20 = !{i64 56}
!21 = !{i64 32}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$3", !5, i64 0}
!24 = !{i64 55}
!25 = !{i64 31}
