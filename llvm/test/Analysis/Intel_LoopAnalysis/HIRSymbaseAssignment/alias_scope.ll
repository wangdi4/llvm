; Check that Alias Scope metadata are handled by symbase assignment analysis

; RUN: opt -analyze -hir-symbase-assignment < %s | FileCheck %s
; RUN: opt -analyze -hir-symbase-assignment -scoped-noalias < %s | FileCheck -check-prefix=NOALIAS %s

; CHECK-DAG: {{.*%q.*\[.*}} {sb:[[Base1:[0-9]+]]}
; CHECK-DAG: {{.*%p.*\[.*}} {sb:[[Base1]]}

; NOALIAS-DAG: {{.*%q.*\[.*}} {sb:[[Base1:[0-9]+]]}
; NOALIAS-NOT: {{.*%p.*\[.*}} {sb:[[Base1]]}

; ModuleID = 'alias_scope.ll'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind
define void @foo(i32* %p, i32* %q, i32 %n) #0 {
entry:
  %cmp1 = icmp slt i32 0, %n
  br i1 %cmp1, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc
  %p.addr.04 = phi i32* [ %p, %for.body.lr.ph ], [ %incdec.ptr1, %for.inc ]
  %q.addr.03 = phi i32* [ %q, %for.body.lr.ph ], [ %incdec.ptr, %for.inc ]
  %i.02 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  %incdec.ptr = getelementptr inbounds i32, i32* %q.addr.03, i32 1
  %0 = load i32, i32* %q.addr.03, align 4, !alias.scope !1, !noalias !2
  %incdec.ptr1 = getelementptr inbounds i32, i32* %p.addr.04, i32 1
  store i32 %0, i32* %p.addr.04, align 4, !alias.scope !2, !noalias !1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end_crit_edge

for.cond.for.end_crit_edge:                       ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.for.end_crit_edge, %entry
  ret void
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!0 = !{!0}
!1 = !{!1, !0}
!2 = !{!2, !0}
