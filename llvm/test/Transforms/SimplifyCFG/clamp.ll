<<<<<<< HEAD
; INTEL added phi-node-folding-threshold=2 because xmain defaults that option to 1.
; RUN: opt < %s -simplifycfg -simplifycfg-require-and-preserve-domtree=1 -phi-node-folding-threshold=2 -S | FileCheck %s ;INTEL
=======
; RUN: opt < %s -passes=simplifycfg -simplifycfg-require-and-preserve-domtree=1 -S | FileCheck %s
>>>>>>> 5fdc6846c55f31d443a0f98fad8a7570e5416dec

define float @clamp(float %a, float %b, float %c) {
; CHECK-LABEL: @clamp
; CHECK:  %cmp = fcmp ogt float %a, %c
; CHECK:  %cmp1 = fcmp olt float %a, %b
; CHECK:  %cond = select i1 %cmp1, float %b, float %a
; CHECK:  [[COND5:%.*]] = select i1 %cmp, float %c, float %cond ;INTEL
; CHECK:  ret float [[COND5]] ;INTEL
entry:
  %cmp = fcmp ogt float %a, %c
  br i1 %cmp, label %cond.end4, label %cond.false

cond.false:                                       ; preds = %entry
  %cmp1 = fcmp olt float %a, %b
  %cond = select i1 %cmp1, float %b, float %a
  br label %cond.end4

cond.end4:                                        ; preds = %entry, %cond.false
  %cond5 = phi float [ %cond, %cond.false ], [ %c, %entry ]
  ret float %cond5
}
