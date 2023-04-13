; RUN: opt -passes="loop(indvars)" < %s -S | FileCheck %s

; Verify that indvars does not replace %gep1 with %struct.gep in the presence of
; "pre_loopopt" attribute even though they have identical SCEVs.

; CHECK-LABEL: foo
; CHECK: %gep1 = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 0
; CHECK: %ld1 = load double, ptr %gep1, align 8

%struct = type { i16, i16 }

define double @foo(ptr %p, i64 %n) "pre_loopopt" {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %struct.gep = getelementptr inbounds %struct, ptr %p, i64 %iv
  %gep = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 1 
  %ld = load double, ptr %gep, align 8
  %gep1 = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 0
  %ld1 = load double, ptr %gep1, align 8
  %iv.inc = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, %n
  br i1 %cmp, label %exit, label %loop

exit:
  %phi = phi double [ %ld, %loop ]
  %phi1 = phi double [ %ld1, %loop ]
  %add = fadd double %phi, %phi1

  ret double %add
}

; CHECK-LABEL: bar
; CHECK-NOT: %gep1
; CHECK: %ld1 = load double, ptr %struct.gep, align 8

define double @bar(ptr %p, i64 %n) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %struct.gep = getelementptr inbounds %struct, ptr %p, i64 %iv
  %gep = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 1 
  %ld = load double, ptr %gep, align 8
  %gep1 = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 0
  %ld1 = load double, ptr %gep1, align 8
  %iv.inc = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, %n
  br i1 %cmp, label %exit, label %loop

exit:
  %phi = phi double [ %ld, %loop ]
  %phi1 = phi double [ %ld1, %loop ]
  %add = fadd double %phi, %phi1

  ret double %add
}
