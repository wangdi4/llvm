; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans
; RUN: opt < %s -opaque-pointers -enable-dtrans -passes='function(functionrecognizer)' -S 2>&1 | FileCheck %s

; Test that @arc_compare is recognized as qsort compare.
; This is the same test as compare02.ll, but does not require asserts.

; CHECK: define{{.*}}@arc_compare{{.*}} #0
; CHECK: attributes #0 = { "is-qsort-compare" }

%struct.arc = type { i32, i64, ptr, ptr, i16, ptr, ptr, i64, i64 }

define internal i32 @arc_compare(ptr %a1, ptr %a2) {
entry:
  %i = load ptr, ptr %a1, align 8
  %flow = getelementptr inbounds %struct.arc, ptr %i, i32 0, i32 7
  %i1 = load i64, ptr %flow, align 8
  %i2 = load ptr, ptr %a2, align 8
  %flow1 = getelementptr inbounds %struct.arc, ptr %i2, i32 0, i32 7
  %i3 = load i64, ptr %flow1, align 8
  %cmp = icmp sgt i64 %i1, %i3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %i4 = load ptr, ptr %a1, align 8
  %flow2 = getelementptr inbounds %struct.arc, ptr %i4, i32 0, i32 7
  %i5 = load i64, ptr %flow2, align 8
  %i6 = load ptr, ptr %a2, align 8
  %flow3 = getelementptr inbounds %struct.arc, ptr %i6, i32 0, i32 7
  %i7 = load i64, ptr %flow3, align 8
  %cmp4 = icmp slt i64 %i5, %i7
  br i1 %cmp4, label %if.then5, label %if.end6

if.then5:                                         ; preds = %if.end
  br label %return

if.end6:                                          ; preds = %if.end
  %i8 = load ptr, ptr %a1, align 8
  %id = getelementptr inbounds %struct.arc, ptr %i8, i32 0, i32 0
  %i9 = load i32, ptr %id, align 8
  %i10 = load ptr, ptr %a2, align 8
  %id7 = getelementptr inbounds %struct.arc, ptr %i10, i32 0, i32 0
  %i11 = load i32, ptr %id7, align 8
  %cmp8 = icmp slt i32 %i9, %i11
  br i1 %cmp8, label %if.then9, label %if.end10

if.then9:                                         ; preds = %if.end6
  br label %return

if.end10:                                         ; preds = %if.end6
  br label %return

return:                                           ; preds = %if.end10, %if.then9, %if.then5, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ -1, %if.then5 ], [ -1, %if.then9 ], [ 1, %if.end10 ]
  ret i32 %retval.0
}
; end INTEL_FEATURE_SW_ADVANCED
