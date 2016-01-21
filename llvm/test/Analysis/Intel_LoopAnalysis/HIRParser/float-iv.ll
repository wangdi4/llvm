; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the pseudo floating point IV is handled correctly.

; CHECK: DO i1 = 0, 22
; CHECK-NEXT: %a.055 = %a.055  +  %a.055
; CHECK-NEXT: %add1 = %a.055  +  1.000000e+00
; CHECK-NEXT: %add.i = %add1  +  0.000000e+00
; CHECK-NEXT: %sub = %add.i  -  %a.055
; CHECK-NEXT: %add.i.48 = %sub  +  0.000000e+00
; CHECK-NEXT: %sub4 = %add.i.48  +  -1.000000e+00
; CHECK-NEXT: %add.i.46 = %sub4  +  0.000000e+00
; CHECK-NEXT: END LOOP


; Function Attrs: nounwind uwtable
define i32 @main() #2 {
entry:
  br label %while.body

while.body:                                       ; preds = %entry, %while.body
  %a.055 = phi float [ 2.000000e+00, %entry ], [ %add8, %while.body ]
  %add8 = fadd float %a.055, %a.055
  %add1 = fadd float %add8, 1.000000e+00
  %add.i = fadd float %add1, 0.000000e+00
  %sub = fsub float %add.i, %add8
  %add.i.48 = fadd float %sub, 0.000000e+00
  %sub4 = fadd float %add.i.48, -1.000000e+00
  %add.i.46 = fadd float %sub4, 0.000000e+00
  %cmp = fcmp oeq float %add.i.46, 0.000000e+00
  br i1 %cmp, label %while.body, label %while.end

while.end:                          ; preds = %while.body
  ret i32 0
}

