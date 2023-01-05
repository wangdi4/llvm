; REQUIRES: intel_feature_markercount
; RUN: llc < %s -function-marker-count=be -loop-marker-count=be -mtriple=x86_64-- -stop-before=tailduplication -override-marker-count-file=%S/../../Inputs/intel-override-marker-count-file.json | FileCheck %s

; CHECK: f1
; CHECK-NOT: PSEUDO_FUNCTION_PROLOG
; CHECK-NOT: PSEUDO_LOOP_HEADER
; CHECK-NOT: PSEUDO_FUNCTION_EPILOG

; CHECK: f2
; CHECK: PSEUDO_FUNCTION_PROLOG
; CHECK-NOT: PSEUDO_LOOP_HEADER
; CHECK: PSEUDO_FUNCTION_EPILOG

; CHECK: f3
; CHECK: PSEUDO_FUNCTION_PROLOG
; CHECK-NOT: PSEUDO_LOOP_HEADER
; CHECK: PSEUDO_FUNCTION_EPILOG

; CHECK: f4
; CHECK-NOT: PSEUDO_FUNCTION_PROLOG
; CHECK: PSEUDO_LOOP_HEADER
; CHECK-NOT: PSEUDO_FUNCTION_EPILOG

; CHECK: f5
; CHECK-NOT: PSEUDO_FUNCTION_PROLOG
; CHECK: PSEUDO_LOOP_HEADER
; CHECK-NOT: PSEUDO_FUNCTION_EPILOG
define i32 @f1(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}

define i32 @f2(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}

define i32 @f3(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}

define i32 @f4(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}

define i32 @f5(i1 %cmp) {
entry:
  br i1 %cmp, label %loop, label %exit

loop:                                        ; preds = %loop, %entry
  br label %loop

exit:                                         ; preds = %entry
  ret i32 0
}
