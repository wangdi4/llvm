; RUN: opt -disable-output -passes=print-alias-sets < %s 2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt -disable-output -passes=print-alias-sets -print-loopcarried-alias-sets < %s 2>&1 | FileCheck --check-prefix=LOOPCARRIED %s

; This file checks that AliasSetTrackers can be asked to disambiguate either
; using AliasAnalysis::alias() or ::loopCarriedAlias(). This IR below has only two
; pointers but exercises the distinction; alias() may say they NoAlias, but
; loopCarriedAlias() cannot.

; CHECK: Alias sets for function 'f':
; CHECK-NEXT: Alias Set Tracker: 2 alias sets for 2 pointer values.

; LOOPCARRIED: Alias sets for function 'f':
; LOOPCARRIED-NEXT: Alias Set Tracker: 1 alias sets for 2 pointer values.

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

