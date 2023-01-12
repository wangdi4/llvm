; REQUIRES: intel_feature_markercount
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- -stop-before=tailduplication | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- -stop-after=block-placement | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- -x86-expand-pseudo-marker-count=false | FileCheck --check-prefix=COMMENT %s
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- | FileCheck %s

; PSEUDO-NOT: PSEUDO_FUNCTION_PROLOG
; PSEUDO-NOT: PSEUDO_FUNCTION_EPILOG
; PSEUDO: bb.{{.*}}.loop
; PSEUDO: PSEUDO_LOOP_HEADER
; PSEUDO-NOT: PSEUDO_LOOP_HEADER

; COMMENT: # =>This Inner Loop Header: Depth=1
; COMMENT-NEXT: #LOOP_HEADER
; COMMENT-NOT: #LOOP_HEADER

; CHECK: # =>This Inner Loop Header: Depth=1
; CHECK-NEXT: marker_loop
; CHECK-NOT: marker_loop

define i32 @f() {
entry:
  br label %loop

loop:                                        ; preds = %loop, %entry
  br label %loop
}
