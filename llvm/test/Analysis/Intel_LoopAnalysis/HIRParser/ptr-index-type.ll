; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -xmain-opt-level=3 -hir-framework-debug=parser -hir-details 2>&1 | FileCheck %s

; Verify that parser uses index type explicitly specified in datalayout
; (32 bits) when it is different from pointer size (40 bits).

; CHECK: + DO i32 i1 = 0, 63, 1   <DO_LOOP>
; CHECK: |   (%p)[2 * (%d /u 4) * i1] = 0;
; CHECK: |   <LVAL-REG> {al:4}(LINEAR ptr %p)[LINEAR i32 2 * (%d /u 4) * i1]
; CHECK: |   (%p)[2 * (%d /u 4) * i1 + (%d /u 4)] = 1;
; CHECK: + END LOOP

target datalayout = "e-m:m-p:40:64:64:32-i32:32-i16:16-i8:8-n32"

define void @test1(i32 %d, ptr nocapture %p) {
entry:
  %div = lshr i32 %d, 2
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.02 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %p.addr.01 = phi ptr [ %p, %entry ], [ %add.ptr1, %for.body ]
  store i32 0, ptr %p.addr.01, align 4
  %add.ptr = getelementptr inbounds i32, ptr %p.addr.01, i32 %div
  store i32 1, ptr %add.ptr, align 4
  %add.ptr1 = getelementptr inbounds i32, ptr %add.ptr, i32 %div
  %inc = add nuw nsw i32 %i.02, 1
  %cmp = icmp eq i32 %inc, 64
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

