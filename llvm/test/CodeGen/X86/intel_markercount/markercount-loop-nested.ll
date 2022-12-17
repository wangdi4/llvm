; REQUIRES: intel_feature_markercount
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- | FileCheck %s

; CHECK: # =>This Loop Header: Depth=1
; CHECK: markercount_loopheader
; CHECK: =>  This Inner Loop Header: Depth=2
; CHECK: markercount_loopheader

define i32 @f(i1 %cmp2) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond1, %entry
  br label %for.cond1

for.cond1:                                        ; preds = %for.cond1, %for.cond
  br i1 %cmp2, label %for.cond1, label %for.cond
}
