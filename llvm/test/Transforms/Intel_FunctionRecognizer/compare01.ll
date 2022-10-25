; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; RUN: opt < %s -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Test that @cost_compare is recognized as a qsort compare.

; CHECK: FUNCTION-RECOGNIZER: FOUND QSORT-COMPARE cost_compare
; CHECK: define{{.*}}@cost_compare{{.*}} #0
; CHECK: attributes #0 = { "is-qsort-compare" }

%struct.node = type { i64, i32, %struct.node*, %struct.node*, %struct.node*, %struct.node*, %struct.arc*, %struct.arc*, %struct.arc*, %struct.arc*, i64, i64, i32, i32 }
%struct.arc = type { i32, i64, %struct.node*, %struct.node*, i16, %struct.arc*, %struct.arc*, i64, i64 }
%struct.basket = type { %struct.arc*, i64, i64, i64 }

; Function Attrs: nounwind uwtable
define dso_local i32 @cost_compare(%struct.basket** %b1, %struct.basket** %b2) #0 {
entry:
  %0 = load %struct.basket*, %struct.basket** %b1, align 8
  %abs_cost = getelementptr inbounds %struct.basket, %struct.basket* %0, i32 0, i32 2
  %1 = load i64, i64* %abs_cost, align 8
  %2 = load %struct.basket*, %struct.basket** %b2, align 8
  %abs_cost1 = getelementptr inbounds %struct.basket, %struct.basket* %2, i32 0, i32 2
  %3 = load i64, i64* %abs_cost1, align 8
  %cmp = icmp slt i64 %1, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %4 = load %struct.basket*, %struct.basket** %b1, align 8
  %abs_cost2 = getelementptr inbounds %struct.basket, %struct.basket* %4, i32 0, i32 2
  %5 = load i64, i64* %abs_cost2, align 8
  %6 = load %struct.basket*, %struct.basket** %b2, align 8
  %abs_cost3 = getelementptr inbounds %struct.basket, %struct.basket* %6, i32 0, i32 2
  %7 = load i64, i64* %abs_cost3, align 8
  %cmp4 = icmp sgt i64 %5, %7
  br i1 %cmp4, label %if.then5, label %if.end6

if.then5:                                         ; preds = %if.end
  br label %return

if.end6:                                          ; preds = %if.end
  %8 = load %struct.basket*, %struct.basket** %b1, align 8
  %a = getelementptr inbounds %struct.basket, %struct.basket* %8, i32 0, i32 0
  %9 = load %struct.arc*, %struct.arc** %a, align 8
  %id = getelementptr inbounds %struct.arc, %struct.arc* %9, i32 0, i32 0
  %10 = load i32, i32* %id, align 8
  %11 = load %struct.basket*, %struct.basket** %b2, align 8
  %a7 = getelementptr inbounds %struct.basket, %struct.basket* %11, i32 0, i32 0
  %12 = load %struct.arc*, %struct.arc** %a7, align 8
  %id8 = getelementptr inbounds %struct.arc, %struct.arc* %12, i32 0, i32 0
  %13 = load i32, i32* %id8, align 8
  %cmp9 = icmp sgt i32 %10, %13
  br i1 %cmp9, label %if.then10, label %if.else

if.then10:                                        ; preds = %if.end6
  br label %return

if.else:                                          ; preds = %if.end6
  br label %return

return:                                           ; preds = %if.else, %if.then10, %if.then5, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ -1, %if.then5 ], [ 1, %if.then10 ], [ -1, %if.else ]
  ret i32 %retval.0
}
; end INTEL_FEATURE_SW_ADVANCED
