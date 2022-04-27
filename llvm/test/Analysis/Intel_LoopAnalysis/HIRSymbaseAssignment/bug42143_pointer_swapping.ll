; REQUIRES: asserts
; Check that we have fixed Bug 42143 from the community:
; https://bugs.llvm.org/show_bug.cgi?id=42143
;
; This test case was provided directly as IR. There should be a dependence between the two memory references.
; In our dependence analysis symbase assignment should assign them to one symbase.

; RUN: opt -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=symbase-assignment -debug-only=hir-framework < %s 2>&1 | FileCheck %s

; CHECK-DAG: {{.*%p.*\[.*}} {sb:[[Base1:[0-9]+]]}
; CHECK-DAG: {{.*%q.*\[.*}} {sb:[[Base1]]}

define float @f() {
entry:
  %g = alloca float, align 4
  %h = alloca float, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %p = phi float* [ %g, %entry ], [ %q, %for.body ]
  %q = phi float* [ %h, %entry ], [ %p, %for.body ]
  %0 = load float, float* %p, align 4
  store float undef, float* %q, align 4
  %branch_cond = fcmp ugt float %0, 0.0
  br i1 %branch_cond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret float undef
}

