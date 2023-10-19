; REQUIRES: asserts
; RUN: opt < %s -passes="print<hir-region-identification>" -xmain-opt-level=3 -hir-loop-or-bb-inst-threshold=1 -debug-only=hir-region-identification  2>&1 | FileCheck %s --check-prefix=CHECK-O3

; RUN: opt < %s -passes="print<hir-region-identification>" -hir-loop-or-bb-inst-threshold=1 -debug-only=hir-region-identification  2>&1 | FileCheck %s --check-prefix=CHECK-O2

; Verify that we form the region with the loop by skipping the instruction
; threshold at O3 because it is a single bblock countable loop with a single phi.

; Also check that we honor the threshold at O2.

; CHECK-O3: Region 1

; CHECK-O2: Loop %for.body: Throttled due to presence of too many statements.


define void @scalar_soa(ptr %a, ptr %b, ptr %c) {
entry:
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds i32, ptr %a, i64 %indvars.iv
  store i32 1, ptr %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  store i32 2, ptr %arrayidx2, align 4
  %arrayidx3 = getelementptr inbounds i32, ptr %c, i64 %indvars.iv
  store i32 3, ptr %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.body, label %exit

exit:
  ret void
}

