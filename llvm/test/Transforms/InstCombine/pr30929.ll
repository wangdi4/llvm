; We need this pipeline because to trigger dominator info verification
; we have to compute the dominator before libcalls-shrinkwrap and
; have a pass which requires the dominator tree after.
<<<<<<< HEAD
; RUN: opt -domtree -libcalls-shrinkwrap -instcombine -verify-dom-info %s
=======
; RUN: opt -passes=libcalls-shrinkwrap,instcombine -verify-dom-info %s
>>>>>>> b314bb9eb728d86a5c549dd5c507be1fb81f1d3a

define void @main() {
  %_tmp31 = call float @acosf(float 2.000000e+00)
  ret void
}

declare float @acosf(float)
