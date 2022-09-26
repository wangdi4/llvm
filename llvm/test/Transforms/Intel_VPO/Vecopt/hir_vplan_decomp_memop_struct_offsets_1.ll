; Test HIR decomposition of memory operands with trailing struct offsets.
; Input LLVM-IR generated for below C code with command: icx -O2 -print-module-before-loopopt

; struct S1 {
;     int a1;
;     int a2[100];
; } s1;
;
; void foo() {
;     int i1;
;     for (i1 = 0; i1 < 100; i1++)
;         s1.a2[i1] = i1;
; }

; Input HIR
; <11>    + DO i1 = 0, 99, 1   <DO_LOOP>
; <4>     |   (@s1)[0].1[i1] = i1;
; <11>    + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-subscript-details -disable-output < %s 2>&1 | FileCheck %s

; Check decomposed VPInstructions
; CHECK: i64 [[I1:%vp.*]] = phi
; CHECK: i32* [[ADDR1:%vp.*]] = subscript inbounds %struct.S1* @s1 {i64 0 : i64 0 : i64 404 : %struct.S1*(%struct.S1) (1 )} {i64 0 : i64 [[I1]] : i64 4 : [100 x i32](i32)}
; CHECK-NEXT: store i32 {{%vp.*}} i32* [[ADDR1]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S1 = type { i32, [100 x i32] }

@s1 = common dso_local local_unnamed_addr global %struct.S1 zeroinitializer, align 4

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds %struct.S1, %struct.S1* @s1, i64 0, i32 1, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

!2 = !{!3, !4, i64 4}
!3 = !{!"struct@S1", !4, i64 0, !7, i64 4}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"array@_ZTSA100_i", !4, i64 0}
