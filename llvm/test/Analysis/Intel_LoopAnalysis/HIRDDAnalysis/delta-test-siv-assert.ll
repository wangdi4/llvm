; RUN: opt -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region -debug-only=hir-dd-test < %s 2>&1  | FileCheck %s
; RUN: opt -passes="print<hir-dd-analysis>"  -hir-dd-analysis-verify=Region -debug-only=hir-dd-test -disable-output < %s 2>&1  | FileCheck %s


; The test case checks that testSIV does not raise an assert with
; SrcLoop not equal DstLoop during Loop Fusion analysis.
;
;<0>          BEGIN REGION { }
;<32>               + DO i1 = 0, 499, 1   <DO_LOOP>
;<33>               |   + DO i2 = 0, 499, 1   <DO_LOOP>
;<6>                |   |   (@a)[0][i2][i2][i1] = 5;
;<33>               |   + END LOOP
;<33>               |
;<34>               |
;<34>               |   + DO i2 = 0, 499, 1   <DO_LOOP>
;<19>               |   |   (@a)[0][i2][i2 + 1][i1] = 6;
;<34>               |   + END LOOP
;<32>               + END LOOP
;<0>          END REGION


; CHECK: subscript 0
; CHECK: src = i1
; CHECK: dst = i1
; CHECK: class = 1
; CHECK: subscript 1
; CHECK: src = i2
; CHECK: dst = i2 + 1
; CHECK: class = 2
; CHECK: subscript 2
; CHECK: src = i2
; CHECK: dst = i2
; CHECK: class = 2
; CHECK: subscript 3
; CHECK: src = 0
; CHECK: dst = 0
; CHECK: class = 0
; CHECK: Separable = {0 3}
; CHECK: Coupled = {2}
; CHECK: testing subscript 0, SIV
; CHECK: Test SIV
; CHECK: Strong SIV test




target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1000 x [1000 x [1000 x i32]]] zeroinitializer, align 16

define dso_local i32 @foo(i32 %N) local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc20
  %indvars.iv49 = phi i64 [ 0, %entry ], [ %indvars.iv.next50, %for.inc20 ], !in.de.ssa !2
  %indvars.iv.in = call i64 @llvm.ssa.copy.i64(i64 0), !in.de.ssa !3
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ], !in.de.ssa !3
  %arrayidx7 = getelementptr inbounds [1000 x [1000 x [1000 x i32]]], [1000 x [1000 x [1000 x i32]]]* @a, i64 0, i64 %indvars.iv, i64 %indvars.iv, i64 %indvars.iv49, !intel-tbaa !4
  store i32 5, i32* %arrayidx7, align 4, !tbaa !4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 500
  %indvars.iv.in52 = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next), !in.de.ssa !3
  br i1 %exitcond, label %for.body10.preheader, label %for.body3

for.body10.preheader:                             ; preds = %for.body3
  %indvars.iv46.in53 = call i64 @llvm.ssa.copy.i64(i64 0), !in.de.ssa !11
  br label %for.body10

for.body10:                                       ; preds = %for.body10.preheader, %for.body10
  %indvars.iv46 = phi i64 [ %indvars.iv.next47, %for.body10 ], [ 0, %for.body10.preheader ], !in.de.ssa !11
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %arrayidx16 = getelementptr inbounds [1000 x [1000 x [1000 x i32]]], [1000 x [1000 x [1000 x i32]]]* @a, i64 0, i64 %indvars.iv46, i64 %indvars.iv.next47, i64 %indvars.iv49, !intel-tbaa !4
  store i32 6, i32* %arrayidx16, align 4, !tbaa !4
  %exitcond48 = icmp eq i64 %indvars.iv.next47, 500
  %indvars.iv46.in = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next47), !in.de.ssa !11
  br i1 %exitcond48, label %for.inc20, label %for.body10

for.inc20:                                        ; preds = %for.body10
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond51 = icmp eq i64 %indvars.iv.next50, 500
  %indvars.iv49.in = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next50), !in.de.ssa !2
  br i1 %exitcond51, label %for.end22, label %for.cond1.preheader

for.end22:                                        ; preds = %for.inc20
  %idxprom23 = sext i32 %N to i64
  %arrayidx28 = getelementptr inbounds [1000 x [1000 x [1000 x i32]]], [1000 x [1000 x [1000 x i32]]]* @a, i64 0, i64 %idxprom23, i64 %idxprom23, i64 500, !intel-tbaa !4
  %0 = load i32, i32* %arrayidx28, align 16, !tbaa !4
  ret i32 %0
}

; Function Attrs: nounwind readnone
declare i64 @llvm.ssa.copy.i64(i64 returned)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!"indvars.iv49.de.ssa"}
!3 = !{!"indvars.iv.de.ssa"}
!4 = !{!5, !8, i64 0}
!5 = !{!"array@_ZTSA1000_A1000_A1000_i", !6, i64 0}
!6 = !{!"array@_ZTSA1000_A1000_i", !7, i64 0}
!7 = !{!"array@_ZTSA1000_i", !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!"indvars.iv46.de.ssa"}

