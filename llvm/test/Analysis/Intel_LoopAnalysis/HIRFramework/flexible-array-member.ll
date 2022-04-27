; RUN: opt -hir-ssa-deconstruction -analyze -hir-framework -enable-new-pm=0 < %s  2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we do not estimate trip counts using possible flexible array
; members. These are the last array field of structures with a size of 1.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>

; CHECK-NOT: MAX_TC_EST

; CHECK: |   (%ptr)[0].2[i1] = i1;
; CHECK: + END LOOP


%struct.PixelWeight = type { i32, i32, [1 x i32] }

define void @foo(i32 %n, %struct.PixelWeight* %ptr) {
entry:
  br label %for.body

for.body:                                       ; preds = %for.body, %entry
  %iv = phi i32 [ 0, %entry ], [ %iv.inc, %for.body ]
  %arrayidx = getelementptr inbounds %struct.PixelWeight, %struct.PixelWeight* %ptr, i32 0, i32 2, i32 %iv
  store i32 %iv, i32* %arrayidx
  %iv.inc = add nsw i32 %iv, 1
  %exitcond754 = icmp eq i32 %iv.inc, %n
  br i1 %exitcond754, label %exit, label %for.body

exit:
  ret void
}
