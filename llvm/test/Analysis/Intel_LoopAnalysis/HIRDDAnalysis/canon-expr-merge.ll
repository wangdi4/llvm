; RUN:  opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction" | opt -passes="print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s

; This test has different src type for the second dimension of refs
; (%tk)[0][i1][0] (i64 type) and (%tk)[0][i1 + 1][0] (i2 type).
; It was failing in CanonExprUtils::subtract() during DD analysis.
; This is to verify that it works.

; CHECK: (%tk)[0][i1 + 1][0] --> (%tk)[0][i1][0] ANTI

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@S = external global [0 x i8], align 1

; Function Attrs: nounwind uwtable
define void @rijndaelKeySched() local_unnamed_addr {
entry:
  %tk = alloca [4 x [8 x i8]], align 16
  %0 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %tk, i64 0, i64 0, i64 0
  call void @llvm.memset.p0i8.i64(i8* nonnull %0, i8 0, i64 32, i32 16, i1 false)
  br label %for.body55

for.body55:                                       ; preds = %for.body55, %while.body.lr.ph
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body55 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %rem57 = and i64 %indvars.iv.next, 3
  %arrayidx61 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %tk, i64 0, i64 %rem57, i64 0
  %1 = load i8, i8* %arrayidx61, align 1, !tbaa !6
  %idxprom62 = zext i8 %1 to i64
  %arrayidx63 = getelementptr inbounds [0 x i8], [0 x i8]* @S, i64 0, i64 %idxprom62
  %2 = load i8, i8* %arrayidx63, align 1, !tbaa !8
  %arrayidx66 = getelementptr inbounds [4 x [8 x i8]], [4 x [8 x i8]]* %tk, i64 0, i64 %indvars.iv, i64 0
  %3 = load i8, i8* %arrayidx66, align 8, !tbaa !6
  %xor324 = xor i8 %3, %2
  store i8 %xor324, i8* %arrayidx66, align 8, !tbaa !6
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end71, label %for.body55

for.end71:                                        ; preds = %for.body55
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #0

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA8_h", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !4, i64 0}
!7 = !{!"array@_ZTSA4_A8_h", !3, i64 0}
!8 = !{!4, !4, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"long", !4, i64 0}
