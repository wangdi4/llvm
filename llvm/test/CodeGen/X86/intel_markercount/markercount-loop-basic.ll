; REQUIRES: intel_feature_markercount
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- -stop-before=tailduplication | FileCheck --check-prefix=TAIL %s
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- -stop-after=x86-pseudo | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- -x86-expand-pseudo-marker-count=false | FileCheck --check-prefix=COMMENT %s
; RUN: llc < %s -loop-marker-count=be -mtriple=x86_64-- | FileCheck %s

; TAIL-NOT: PSEUDO_FUNCTION_PROLOG
; TAIL-NOT: PSEUDO_FUNCTION_EPILOG
; TAIL: bb.{{.*}}.loop
; TAIL: PSEUDO_LOOP_HEADER
; TAIL-NOT: PSEUDO_LOOP_HEADER

; PSEUDO-NOT: PSEUDO_LOOP_HEADER
; PSEUDO: bb.{{.*}}.loop
; PSEUDO: MARKER_COUNT_LOOP_HEADER
; PSEUDO-NOT: MARKER_COUNT_LOOP_HEADER

; COMMENT: # =>This Inner Loop Header: Depth=1
; COMMENT-NEXT: #LOOP_HEADER
; COMMENT-NOT: #LOOP_HEADER

; CHECK: # =>This Inner Loop Header: Depth=1
; CHECK-NEXT: markercount_loopheader
; CHECK-NOT: markercount_loopheader

define i32 @f() {
entry:
  br label %loop

loop:                                        ; preds = %loop, %entry
  br label %loop
}
