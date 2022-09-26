; Test to check that VF forced via "llvm.loop.vectorize.vector.width" metadata is propagated to HIR vectorizer.

; C source
; int arr[1025];
; void foo() {
;   int i;
;   #pragma clang loop vectorize_width(32)
;   for (i = 0; i < 1025; i++)
;     arr[i] = 42;
; }

; Incoming HIR
;   + DO i1 = 0, 1024, 1   <DO_LOOP> <vectorize = 32>
;   |   (@arr)[0][i1] = 42;
;   + END LOOP

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s


; CHECK:          BEGIN REGION { modified }
; CHECK:                + DO i1 = 0, 1023, 32   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:           |   (<32 x i32>*)(@arr)[0][i1] = 42;
; CHECK-NEXT:           + END LOOP

; CHECK:                + DO i1 = {{.*}}, 1024, 1   <DO_LOOP>{{.*}}<novectorize>
; CHECK-NEXT:           |   (@arr)[0][i1] = 42;
; CHECK-NEXT:           + END LOOP
; CHECK:          END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1025 x i32] zeroinitializer, align 16
; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1025 x i32], [1025 x i32]* @arr, i64 0, i64 %indvars.iv, !intel-tbaa !2
  store i32 42, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1025
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !7

for.end:                                          ; preds = %for.body
  ret void
}

!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8, !9}
!8 = !{!"llvm.loop.vectorize.width", i32 32}
!9 = !{!"llvm.loop.vectorize.enable", i1 true}
