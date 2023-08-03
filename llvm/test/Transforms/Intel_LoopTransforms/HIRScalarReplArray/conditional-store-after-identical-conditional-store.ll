; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-scalarrepl-array" -print-before=hir-scalarrepl-array -print-after=hir-scalarrepl-array -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not process groups with only conditional stores.

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 10;
; CHECK: |   }
; CHECK: |   if (i1 > 30)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 20;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 10;
; CHECK: |   }
; CHECK: |   if (i1 > 30)
; CHECK: |   {
; CHECK: |      (@A)[0][i1] = 20;
; CHECK: |   }
; CHECK: + END LOOP


@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc1
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc1 ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  %cmp1 = icmp sgt i32 %i.01, 10
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  store i32 10, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %cmp2 = icmp sgt i32 %i.01, 30
  br i1 %cmp2, label %if.then1, label %for.inc1

if.then1:                                          ; preds = %for.body
  store i32 20, ptr %arrayidx, align 4
  br label %for.inc1

for.inc1:
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}
