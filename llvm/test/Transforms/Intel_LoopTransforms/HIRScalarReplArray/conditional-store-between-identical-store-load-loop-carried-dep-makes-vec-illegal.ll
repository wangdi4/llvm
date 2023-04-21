; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-before=hir-loop-independent-scalar-repl -print-after=hir-loop-independent-scalar-repl -disable-output < %s 2>&1 | FileCheck %s

; Verify that we give up on the (@A)[0][i1 + 1] group with conditional store
; because moving the store after the if will convert this lexically forward
; edge-
; (@A)[0][i1 + 1] --> (@A)[0][i1] FLOW (<)
; into a lexially backward edge thereby making vectorization illegal.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1 + 1] = 5;
; CHECK: |   %ld.phi = 0;
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1 + 1] = 10;
; CHECK: |      %ld.phi = (@A)[0][i1];
; CHECK: |   }
; CHECK: |   %add = %ld.phi  +  (@A)[0][i1 + 1];
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, 98, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1 + 1] = 5;
; CHECK: |   %ld.phi = 0;
; CHECK: |   if (i1 > 10)
; CHECK: |   {
; CHECK: |      (@A)[0][i1 + 1] = 10;
; CHECK: |      %ld.phi = (@A)[0][i1];
; CHECK: |   }
; CHECK: |   %add = %ld.phi  +  (@A)[0][i1 + 1];
; CHECK: + END LOOP


@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %idxprom.1 = add nsw i64 %idxprom, 1
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom
  %arrayidx1 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %idxprom.1
  store i32 5, ptr %arrayidx1, align 4
  %cmp1 = icmp sgt i32 %i.01, 10
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  store i32 10, ptr %arrayidx1, align 4
  %ld = load i32, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %ld.phi = phi i32 [ %ld, %if.then ], [ 0, %for.body ]
  %ld1 = load i32, ptr %arrayidx1, align 4
  %add = add i32 %ld.phi, %ld1
  %inc = add nsw i32 %i.01, 1
  %cmp = icmp slt i32 %inc, 99 
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  %add.lcssa = phi i32 [ %add, %for.inc ]
  ret i32 %add.lcssa
}
