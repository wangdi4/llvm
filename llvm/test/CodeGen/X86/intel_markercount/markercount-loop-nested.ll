; REQUIRES: intel_feature_markercount
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- | FileCheck %s

; CHECK: # =>This Loop Header: Depth=1
; CHECK: marker_loop
; CHECK: =>  This Inner Loop Header: Depth=2
; CHECK: marker_loop

define i32 @f(i1 %cmp2) {
entry:
  br label %loop.outer

loop.outer:                                         ; preds = %loop.inner, %entry
  br label %loop.inner

loop.inner:                                        ; preds = %loop.inner, %loop.outer
  br i1 %cmp2, label %loop.inner, label %loop.outer
}
