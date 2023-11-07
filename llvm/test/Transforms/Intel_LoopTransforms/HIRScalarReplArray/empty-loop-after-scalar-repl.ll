; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-independent-scalar-repl" -print-before=hir-loop-independent-scalar-repl -print-after=hir-loop-independent-scalar-repl -disable-output 2>&1 | FileCheck %s

; Verify that we are able to handle loops that can be optimized away after loop
; independent scalar-replacement successfully. This loop is recognized as dead
; after scalar-replacement and removed.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   if ((@A)[0][i1] != (@A)[0][i1])
; CHECK: |   {
; CHECK: |      goto for.end;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT: END REGION


@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  %ld1 = load i32, ptr %arrayidx, align 4
  %ld2 = load i32, ptr %arrayidx, align 4
  %cmp1 = icmp ne i32 %ld1, %ld2
  br i1 %cmp1, label %for.end, label %if.end

if.end:                                           ; preds = %land.lhs.true, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %land.lhs.true, %if.end
  ret void
}

