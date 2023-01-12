; REQUIRES: intel_feature_markercount
; RUN: llc < %s -function-marker-count=be -loop-marker-count=be -mtriple=x86_64-- | FileCheck %s

; CHECK: marker_function                    # PROLOG
; CHECK: # =>This Inner Loop Header: Depth=1
; CHECK-NEXT: marker_loop
; CHECK: marker_function                    # EPILOG

define i32 @g(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}
