; REQUIRES: intel_feature_markercount
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -stop-before=tailduplication | FileCheck --check-prefix=TAIL %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -stop-after=x86-pseudo | FileCheck --check-prefix=PSEUDO %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- -x86-expand-pseudo-marker-count=false | FileCheck --check-prefix=COMMENT %s
; RUN: llc < %s -function-marker-count=be -mtriple=x86_64-- | FileCheck %s

; TAIL-NOT: PSEUDO_LOOP_HEADER
; TAIL: f
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL: PSEUDO_FUNCTION_EPILOG

; TAIL: g
; TAIL: bb.0.entry
; TAIL: PSEUDO_FUNCTION_PROLOG
; TAIL-NOT: PSEUDO_FUNCTION_PROLOG
; TAIL: bb.{{.*}}.for.end
; TAIL: PSEUDO_FUNCTION_EPILOG
; TAIL-NOT: PSEUDO_FUNCTION_EPILOG

; PSEUDO-NOT: PSEUDO_FUNCTION_PROLOG
; PSEUDO-NOT: PSEUDO_FUNCTION_EPILOG
; PSEUDO: f
; PSEUDO: MARKER_COUNT_FUNCTION
; PSEUDO: MARKER_COUNT_FUNCTION

; PSEUDO: g
; PSEUDO: bb.0.entry
; PSEUDO: MARKER_COUNT_FUNCTION
; PSEUDO: bb.{{.*}}.for.end
; PSEUDO: MARKER_COUNT_FUNCTION
; PSEUDO-NOT: MARKER_COUNT_FUNCTION

; COMMENT: f
; COMMENT: #FUNCTION_PROLOG
; COMMENT: #FUNCTION_EPILOG
; COMMENT: retq

; COMMENT: g
; COMMENT: #FUNCTION_PROLOG
; COMMENT: #FUNCTION_EPILOG
; COMMENT: retq

; CHECK: f
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG
; CHECK: retq

; CHECK: g
; CHECK: markercount_function                    # PROLOG
; CHECK: markercount_function                    # EPILOG
; CHECK: retq
define i32 @f() {
entry:
  ret i32 0
}

define i32 @g(i1 %cmp) {
entry:
  br i1 %cmp, label %for.cond1, label %for.end6

for.cond1:                                        ; preds = %for.cond1, %entry
  br label %for.cond1

for.end6:                                         ; preds = %entry
  ret i32 0
}
