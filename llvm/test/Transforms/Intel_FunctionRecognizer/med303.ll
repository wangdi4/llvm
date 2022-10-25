; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; RUN: opt < %s -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Test that @med3 is not recognized as a qsort med3, because it has the wrong
; inputs on the PHINode.

; CHECK-NOT: FUNCTION-RECOGNIZER: FOUND QSORT-MED3 med3
; CHECK: define{{.*}}@med3{{.*}}
; CHECK-NOT: attributes #0 = { "is-qsort-med3" }

define internal i8* @med3(i8* %a, i8* %b, i8* %c, i32 (i8*, i8*)* %cmp) #2 {
entry:
  %call = call i32 %cmp(i8* %a, i8* %b)
  %cmp1 = icmp slt i32 %call, 0
  %call2 = call i32 %cmp(i8* %b, i8* %c)
  br i1 %cmp1, label %cond.true, label %cond.false11

cond.true:                                        ; preds = %entry
  %cmp3 = icmp slt i32 %call2, 0
  br i1 %cmp3, label %cond.true4, label %cond.false

cond.true4:                                       ; preds = %cond.true
  br label %cond.end24

cond.false:                                       ; preds = %cond.true
  %call5 = call i32 %cmp(i8* %a, i8* %c)
  %cmp6 = icmp slt i32 %call5, 0
  %0 = select i1 %cmp6, i8* %c, i8* %a
  br label %cond.end24

cond.false11:                                     ; preds = %entry
  %cmp13 = icmp sgt i32 %call2, 0
  br i1 %cmp13, label %cond.true14, label %cond.false15

cond.true14:                                      ; preds = %cond.false11
  br label %cond.end24

cond.false15:                                     ; preds = %cond.false11
  %call16 = call i32 %cmp(i8* %a, i8* %c)
  %cmp17 = icmp slt i32 %call16, 0
  %1 = select i1 %cmp17, i8* %a, i8* %c
  br label %cond.end24

cond.end24:                                       ; preds = %cond.true14, %cond.false15, %cond.true4, %cond.false
  %cond25 = phi i8* [ %a, %cond.true4 ], [ %0, %cond.false ], [ %c, %cond.true14 ], [ %1, %cond.false15 ]
  ret i8* %cond25
}


; end INTEL_FEATURE_SW_ADVANCED
