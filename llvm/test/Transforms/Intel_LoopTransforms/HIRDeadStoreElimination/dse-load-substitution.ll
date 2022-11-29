; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that we eliminate the first two stores to (%A)[0] by forward
; substituting the intermediate load with the second store's RHS.

; Load becomes a copy and the copy's rval is propagated to the use in the add
; instruction by the copy propagation utility.

; Print Before-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = 0;
; CHECK: |   (%A)[0] = i1;
; CHECK: |   %ld = (%A)[0];
; CHECK: |   %t.02 = %t.02  +  %ld;
; CHECK: |   (%A)[0] = 5;
; CHECK: + END LOOP

; Print After-

; CHECK:      BEGIN REGION { modified }
; CHECK:      + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT: |   %t.02 = %t.02  +  i1;
; CHECK:      |   (%A)[0] = 5;
; CHECK:      + END LOOP

define dso_local i32 @foo(i32* nocapture %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %t.02 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  store i32 0, i32* %A, align 4
  store i32 %i.01, i32* %A, align 4
  %ld = load i32, i32* %A, align 4
  %add = add nuw nsw i32 %t.02, %ld
  store i32 5, i32* %A, align 4
  %inc = add nuw nsw i32 %i.01, 1
  %cmp = icmp ult i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %t.0.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %t.0.lcssa
}

