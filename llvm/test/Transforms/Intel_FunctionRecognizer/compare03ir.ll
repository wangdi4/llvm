; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans
; RUN: opt < %s -opaque-pointers -enable-dtrans -passes='function(functionrecognizer)' -S 2>&1 | FileCheck %s

; Test that @cost_compare is not recognized as a qsort compare, because it
; incorrectly indexes the structure fields being compared.
; This is the same test as compare03.ll, but does not require asserts.

; CHECK: define{{.*}}@cost_compare
; CHECK-NOT: attributes #0 = { "is-qsort-compare" }

%struct.basket = type { ptr, i64, i64, i64 }
%struct.arc = type { i32, i64, ptr, ptr, i16, ptr, ptr, i64, i64 }

define dso_local i32 @cost_compare(ptr %b1, ptr %b2) {
entry:
  %i = load ptr, ptr %b1, align 8
  %abs_cost = getelementptr inbounds %struct.basket, ptr %i, i32 0, i32 2
  %i1 = load i64, ptr %abs_cost, align 8
  %i2 = load ptr, ptr %b2, align 8
  %abs_cost1 = getelementptr inbounds %struct.basket, ptr %i2, i32 0, i32 3
  %i3 = load i64, ptr %abs_cost1, align 8
  %cmp = icmp slt i64 %i1, %i3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %i4 = load ptr, ptr %b1, align 8
  %abs_cost2 = getelementptr inbounds %struct.basket, ptr %i4, i32 0, i32 3
  %i5 = load i64, ptr %abs_cost2, align 8
  %i6 = load ptr, ptr %b2, align 8
  %abs_cost3 = getelementptr inbounds %struct.basket, ptr %i6, i32 0, i32 2
  %i7 = load i64, ptr %abs_cost3, align 8
  %cmp4 = icmp sgt i64 %i5, %i7
  br i1 %cmp4, label %if.then5, label %if.end6

if.then5:                                         ; preds = %if.end
  br label %return

if.end6:                                          ; preds = %if.end
  %i8 = load ptr, ptr %b1, align 8
  %a = getelementptr inbounds %struct.basket, ptr %i8, i32 0, i32 0
  %i9 = load ptr, ptr %a, align 8
  %id = getelementptr inbounds %struct.arc, ptr %i9, i32 0, i32 0
  %i10 = load i32, ptr %id, align 8
  %i11 = load ptr, ptr %b2, align 8
  %a7 = getelementptr inbounds %struct.basket, ptr %i11, i32 0, i32 0
  %i12 = load ptr, ptr %a7, align 8
  %id8 = getelementptr inbounds %struct.arc, ptr %i12, i32 0, i32 0
  %i13 = load i32, ptr %id8, align 8
  %cmp9 = icmp sgt i32 %i10, %i13
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
