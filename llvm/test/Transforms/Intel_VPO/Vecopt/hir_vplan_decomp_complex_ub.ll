; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-print-plain-cfg -disable-output < %s 2>&1 | FileCheck %s

; Verify that decomposer generates the right code for loop upper bound in a do-while loop.

; Source code
; #define N 1600
;
; int a[N];
; int b[N];
; int c[N];
;
; void foo(int n) {
;     int i = 0;
;     do {
;         c[i] = a[i] + b[i];
;         i++;
;     } while (i < n);
; }

; CHECK: i64 [[Sext:%.*]] = sext i32 {{.*}}
; CHECK-NEXT: i64 [[Max:%.*]] = smax i64 [[Sext]] i64 1
; CHECK-NEXT: i64 [[Add:%.*]] = add i64 [[Max]] i64 -1
; CHECK-NEXT: SUCCESSORS(1):[[H:BB.*]]
; CHECK: [[H]]:
; CHECK-NEXT: i64 [[IVPhi:%.*]] = phi

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [1600 x i32] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [1600 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr {
entry:
  %0 = sext i32 %n to i64
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds [1600 x i32], [1600 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [1600 x i32], [1600 x i32]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx2, align 4, !tbaa !2
  %add = add nsw i32 %2, %1
  %arrayidx4 = getelementptr inbounds [1600 x i32], [1600 x i32]* @c, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 %add, i32* %arrayidx4, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %do.body, label %do.end

do.end:                                           ; preds = %do.body
  ret void
}


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1600_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
