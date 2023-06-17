; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-details-dims 2>&1 -disable-output | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="hir-ssa-deconstruction,print<hir>" -hir-details-dims 2>&1 -disable-output | FileCheck %s --check-prefix=CHECK-OPAQUE

; Check that we can successfully form a bitcasted load with an
; opaque structure element type (%struct.A).

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   %ld = (i32*)(%opaq)[0:0:0(%struct.A:0)];
; CHECK: |   (%b)[0:i1:4(i32:0)] = %ld;
; CHECK: + END LOOP

; Verify that bitcast destination type goes away for opaque ptr self refs like
; (%opaq)[0] and the stride of 4 is set bassed on load type of i32.

; CHECK-OPAQUE: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK-OPAQUE: |   %ld = (%opaq)[0:0:4(i32:0)];
; CHECK-OPAQUE: |   (%b)[0:i1:4(i32:0)] = %ld;
; CHECK-OPAQUE: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.A = type opaque


define void @foo(%struct.A* %opaq, i32* %b, i32 %n) {
entry:
  br label %for.body

for.body:
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %b.phi = phi i32* [ %gep, %for.body ], [ %b, %entry ]
  %gep = getelementptr inbounds i32, i32* %b.phi, i64 1
  %opaq.bc = bitcast %struct.A* %opaq to i32*
  %ld = load i32, i32* %opaq.bc, align 8
  store i32 %ld, i32* %b.phi
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %if.end.loopexit, label %for.body

if.end.loopexit:
  br label %if.end

if.end:
  ret void
}

